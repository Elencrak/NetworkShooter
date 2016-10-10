// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "NetworkShooterGameMode.h"
#include "WHSpawnPoint.generated.h"

UCLASS()
class NETWORKSHOOTER_API AWHSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWHSpawnPoint();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// OnConstruction method called when the BP was open or
	// set in editor scene
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	void ActorBeginOverlaps(AActor* overlappedActor, AActor* paramActor);

	UFUNCTION()
	void ActorEndOverlaps(AActor* overlappedActor, AActor* paramActor);

	bool GetBlocked()
	{
		TArray<AActor*> overlapp;
		GetOverlappingActors(overlapp);
		return overlapp.Num() != 0;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETeam team;

private:
	UCapsuleComponent* spawnCapsule;

	TArray<class AActor*> overlappingActor;
};
