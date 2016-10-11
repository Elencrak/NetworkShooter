// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NetworkShooter.h"
#include "NetworkShooterCharacter.h"
#include "NetworkShooterProjectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Net/UnrealNetwork.h"
#include "BCNetworkAPI/BCPlayerState.h"
#include "MotionControllerComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ANetworkShooterCharacter

ANetworkShooterCharacter::ANetworkShooterCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterFP_Mesh"));
	FP_Mesh->SetOnlyOwnerSee(true);
	FP_Mesh->SetupAttachment(FirstPersonCameraComponent);
	FP_Mesh->bCastDynamicShadow = false;
	FP_Mesh->CastShadow = false;
	FP_Mesh->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	FP_Mesh->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(FP_Mesh, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	//Create a gun mesh component
	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));
	TP_Gun->SetOwnerNoSee(true);
	// original code TP_Gun->AttachTo(GetMesh(), TEXT("hand_rSocket"), EAttachLocation::SnapToTargetIncludingScale, true);
	TP_Gun->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));

	GetMesh()->SetOwnerNoSee(true);

	//Create particles for 3rd personne shooter
	TP_GunShotParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TP_ParticleSystem"));
	TP_GunShotParticle->bAutoActivate = false;
	TP_GunShotParticle->SetupAttachment(TP_Gun);
	TP_GunShotParticle->SetOwnerNoSee(true);

	//Create particle for 1rt person shooter
	FP_GunShotParticle = CreateDefaultSubobject<UParticleSystemComponent>("FP_ParticleSystem");
	FP_GunShotParticle->bAutoActivate = false;
	FP_GunShotParticle->SetupAttachment(TP_Gun);
	FP_GunShotParticle->SetOnlyOwnerSee(true);	

	// Create Bullet Particle shared by the 3rd en 1rt player
	BulletParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BulletSysTp"));
	BulletParticle->bAutoActivate = false;
	BulletParticle->SetupAttachment(FirstPersonCameraComponent);

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void ANetworkShooterCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (Role != ROLE_Authority)
	{
		SetTeam(currentTeam);
	}

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(FP_Mesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		FP_Mesh->SetHiddenInGame(true, true);
	}
	else
	{
		FP_Mesh->SetHiddenInGame(false, true);
	}
}

void ANetworkShooterCharacter::PossessedBy(AController* newController)
{
	Super::PossessedBy(newController);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PossessedBy"));
	NSPlayerState = Cast<ABCPlayerState>(PlayerState);

	// Put in the method all initialize data when the pawn was possessed
	// In network most of data set in player state but not all
	if (Role == ROLE_Authority && NSPlayerState != nullptr) 
	{
		NSPlayerState->health = 100.f;
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANetworkShooterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ANetworkShooterCharacter::OnFire);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANetworkShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANetworkShooterCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANetworkShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANetworkShooterCharacter::LookUpAtRate);
}

void ANetworkShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetworkShooterCharacter, currentTeam);
}

float ANetworkShooterCharacter::TakeDamage(float damage, struct FDamageEvent const&
	damageEvent, AController* eventInstigator, AActor* damageCauser)
{

	Super::TakeDamage(damage, damageEvent, eventInstigator, damageCauser);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%f"), damage));
	if (Role == ROLE_Authority && damageCauser != this && NSPlayerState->health > 0)
	{
		NSPlayerState->health -= damage;
		PlayPain();

		if (NSPlayerState->health <= 0)
		{
			// Increment the number of death
			NSPlayerState->deaths++;

			// Send to all entities to execute the ragdool animation
			MultiCastRagdool();

			ANetworkShooterCharacter* otherActor = Cast<ANetworkShooterCharacter>(damageCauser);

			if (otherActor)
			{
				otherActor->NSPlayerState->Score += 1;
			}

			// For make a timer use this part of code
			// I need timerhandle
			//After 3 seconds respawn
			FTimerHandle timerHandle;

			// With declare timer like this i can call back a method after the timer and
			// I need for that a pointer of function, float, object create action
			// This timer is added in delegate TimerManager execution.
			GetWorldTimerManager().SetTimer(
				timerHandle,
				this,
				& ANetworkShooterCharacter::Respawn,
				3.F,
				false);

		}
		return damage;
	}
	return 0.f;
}

void ANetworkShooterCharacter::OnFire()
{
	// try and play a firing animation if specified
	if (FP_FireAnimaiton != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = FP_Mesh->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FP_FireAnimaiton, 1.f);
		}
	}

	//Play th FP particle effect specified
	if (FP_GunShotParticle != nullptr)
	{
		FP_GunShotParticle->Activate(true);
	}

	//Implementation the shooting paramter must be send to server
	FVector mousePosition;
	FVector mouseDirection;

	APlayerController* controller = Cast<APlayerController>(GetController());

	FVector2D screenSize = GEngine->GameViewport->Viewport->GetSizeXY();

	controller->DeprojectScreenPositionToWorld(
		screenSize.X / 2.f, 
		screenSize.Y / 2.f,
		mousePosition,
		mouseDirection);

	mouseDirection *= 10000000.f;

	// Send to server for execute action
	ServerFire(mousePosition, mouseDirection);
}

void ANetworkShooterCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ANetworkShooterCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ANetworkShooterCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANetworkShooterCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

/* Only executer on server */
void ANetworkShooterCharacter::Fire(const FVector pos, FVector dir)
{
	//Set object param
	FCollisionObjectQueryParams collisionObjectQuery;
	collisionObjectQuery.AddObjectTypesToQuery(ECC_GameTraceChannel1);

	// Set the collision param
	FCollisionQueryParams collisionQuery;
	collisionQuery.AddIgnoredActor(this);

	// stock the res in FHitResult
	FHitResult hitRes;
	//Draw ray cast
	GetWorld()->LineTraceSingleByObjectType(hitRes,
		pos, 
		dir, 
		collisionObjectQuery, 
		collisionQuery);

	//Debug ray cast
	DrawDebugLine(GetWorld(), pos, dir, FColor::Red, true, 1.f, 0, 3.f);

	if (hitRes.bBlockingHit)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("FireHit"));
		ANetworkShooterCharacter* otherChar = Cast<ANetworkShooterCharacter>(hitRes.GetActor());

		/*if(otherChar->GetABCPlayerState() != nullptr)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("OtherCharacter is not null"));*/
		if (otherChar != nullptr && otherChar->GetABCPlayerState()->team != this->GetABCPlayerState()->team) 
		{
			FDamageEvent damageEvent = FDamageEvent(UDamageType::StaticClass());
			otherChar->TakeDamage(10.f, damageEvent, this->GetController(), this);

			APlayerController* thisPlayerController = Cast<APlayerController>(GetController());

			// Be careful if you want to launch feed back you must pass by player controller
			// and select your feedback to launch
			thisPlayerController->ClientPlayForceFeedback(HitSuccessFeedback, false, NAME_None);
		}
	}
}

/***                                   Private Validate methode                                **/

// Check if the vector send in parameter is not null.
bool ANetworkShooterCharacter::ServerFire_Validate(const FVector pos, const FVector dir) 
{

	if (pos != FVector(ForceInit) && dir != FVector(ForceInit))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ValidateTrue"));
		return true;
	}
	else
	{
		return false;
	}
}

/***                                  Private Implementation methode                                **/

// Implement the method lauched by server
void ANetworkShooterCharacter::ServerFire_Implementation(const FVector pos, const FVector dir)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ServerFire"));
	// Launch fire
	Fire(pos, dir);

	//Inform all other player to launch our shootEffect
	MultiCastShootEffects();
}

void ANetworkShooterCharacter::MultiCastShootEffects_Implementation()
{
	//Ty and play a firing animation specified
	if (TP_FireAnimation != NULL) 
	{
		// Get the animation object for the arms mesh
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if(animInstance != NULL)
		{
			animInstance->Montage_Play(TP_FireAnimation);
		}
	}

	//Try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	//Try to launch 3rd person particle
	if (TP_GunShotParticle != NULL)
	{
		TP_GunShotParticle->bAutoDestroy = true;
	}

	/*if (BulletParticle != NULL)
	{
		UParticleSystemComponent* tempParticle = UGameplayStatics::SpawnEmitterAtLocation(this, 
			BulletParticle->Template, 
			BulletParticle->GetComponentLocation(), 
			BulletParticle->GetComponentRotation());
	}*/
}

void ANetworkShooterCharacter::Desactivate(UParticleSystemComponent* test)
{
	TP_GunShotParticle->DestroyComponent();
}

void ANetworkShooterCharacter::MultiCastRagdool_Implementation()
{
	// Todo go back here for how does ragdool work.
	GetMesh()->SetPhysicsBlendWeight(1.f);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName("Ragdoll");
}

void ANetworkShooterCharacter::PlayPain_Implementation() 
{
	if (Role == ROLE_AutonomousProxy)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PainSound, GetActorLocation());
	}
}

/***                                  Public Implementation methode                                **/

void ANetworkShooterCharacter::SetTeam_Implementation(ETeam newTeam)
{
	FLinearColor outColour;
	if (newTeam == ETeam::BLUE_TEAM)
	{
		outColour = FLinearColor(0.0f, 0.0f, 0.5f);
	}
	else
	{
		outColour = FLinearColor(0.5f, 0.0f, 0.0f);
	}

	if (DynamicMat == nullptr)
	{
		DynamicMat = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0), this);

		DynamicMat->SetVectorParameterValue(TEXT("BodyColor"), outColour);

		GetMesh()->SetMaterial(0, DynamicMat);
		FP_Mesh->SetMaterial(0, DynamicMat);
	}
}


class ABCPlayerState* ANetworkShooterCharacter::GetABCPlayerState()
{
	if (NSPlayerState)
	{
		return NSPlayerState;
	}
	else
	{
		NSPlayerState = Cast<ABCPlayerState>(PlayerState);
		if (PlayerState == nullptr)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PlayerState is null"));
		return NSPlayerState;
	}
}

void ANetworkShooterCharacter::SetABCPlayerState(class ABCPlayerState* newPlayerState)
{
	// Ensure ps is vali and only set on server
	if (newPlayerState && Role == ROLE_Authority)
	{
		NSPlayerState = newPlayerState;
		PlayerState = newPlayerState;
	}
}

void ANetworkShooterCharacter::Respawn()
{
	if (Role == ROLE_Authority)
	{
		// Get Location from game mode
		NSPlayerState->health = 100.f;
		Cast<ANetworkShooterGameMode>(GetWorld()->GetAuthGameMode())->Respawn(this);
		Destroy(true, true);
	}
}