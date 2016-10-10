// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkShooter.h"
#include "WHSpawnPoint.h"


// Sets default values
AWHSpawnPoint::AWHSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	spawnCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	spawnCapsule->SetCollisionProfileName("OverlapAllDynamic");
	spawnCapsule->bGenerateOverlapEvents = true;
	spawnCapsule->SetCollisionResponseToChannel(ECC_EngineTraceChannel1, ECollisionResponse::ECR_Overlap);

	OnActorBeginOverlap.AddDynamic(this, &AWHSpawnPoint::ActorBeginOverlaps);
	OnActorEndOverlap.AddDynamic(this, &AWHSpawnPoint::ActorEndOverlaps);
}

void AWHSpawnPoint::OnConstruction(const FTransform& Transform)
{
	if (team == ETeam::RED_TEAM) 
	{
		spawnCapsule->ShapeColor = FColor(255,0,0);
	}
	else // If there is blue team
	{
		spawnCapsule->ShapeColor = FColor(0, 0, 255);
	}
}

// Called when the game starts or when spawned
void AWHSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWHSpawnPoint::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	spawnCapsule->UpdateOverlaps();

}

void AWHSpawnPoint::ActorBeginOverlaps(AActor* overlappedActor, AActor* paramActor)
{
	if (Role == ROLE_Authority)
	{
		if (overlappingActor.Find(paramActor) == INDEX_NONE)
		{
			overlappingActor.Add(paramActor);
		}
	}
}

void AWHSpawnPoint::ActorEndOverlaps(AActor* overlappedActor, AActor* paramActor)
{
	if (Role = ROLE_Authority)
	{
		if (overlappingActor.Find(paramActor) != INDEX_NONE)
		{
			overlappingActor.Remove(paramActor);
		}
	}
}