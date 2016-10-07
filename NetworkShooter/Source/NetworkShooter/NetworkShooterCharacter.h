// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "NetworkShooterCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class ANetworkShooterCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* FP_Mesh;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Gun mesh: 3rd person view (seen only by others) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* TP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

public:
	ANetworkShooterCharacter();

	virtual void BeginPlay();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	//FVector GunOffset;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** Sound to play each time we fire*/
	UPROPERTY(EditAnywhere, Category = Gameplay)
	class USoundBase* PainSound;

	/**3rd person anim montage asset for gun shot */
	UPROPERTY(EditAnywhere, Category = Gameplay)
	class UAnimMontage* TP_FireAnimation;

	/** 1st person anim montage asset for gun shot*/
	UPROPERTY(EditAnywhere, Category = Gameplay)
	class UAnimMontage* FP_FireAnimaiton;

	/** Particle system for 1st person gunshot effect*/
	UPROPERTY(EditAnywhere, Category = Gameplay)
	class UParticleSystemComponent* FP_GunShotParticle;

	/** Particle system for 3rd person gun shot effect */
	UPROPERTY(EditAnywhere, Category = Gameplay)
	class UParticleSystemComponent* TP_GunShotParticle;

	/** particule system that will represent a bullet*/
	UPROPERTY(EditAnywhere, Category = Gameplay)
	class UParticleSystemComponent* BulletParticle;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	class UForceFeedbackEffect* HitSuccessFeedback;


	/** AnimMontage to play each time we fire */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	//class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns FP_Mesh subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetFP_Mesh() const { return FP_Mesh; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

