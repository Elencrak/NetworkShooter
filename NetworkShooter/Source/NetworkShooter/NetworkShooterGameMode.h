// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "NetworkShooterGameMode.generated.h"


// Prepare a non team
UENUM(BlueprintType)
enum class ETeam : uint8
{
	BLUE_TEAM,
	RED_TEAM
};


UCLASS()
class ANetworkShooterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ANetworkShooterGameMode();

	// launch and close session logic
	virtual void BeginPlay() override;
	virtual void Tick(float deltaTime) override;
	virtual void PostLogin(APlayerController* newPlayer) override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason);

	// game logic
	void Respawn(class ANetworkShooterCharacter* charaterToRespawn);
	void Spawn(class ANetworkShooterCharacter* chartacterToSpawn);

private:
	TArray<class ANetworkShooterCharacter*> redTeam;
	TArray<class ANetworkShooterCharacter*> blueTeam;

	TArray<class AWHSpawnPoint*> redSpawnPoint;
	TArray<class AWHSpawnPoint*> blueSpawnPoint;
	TArray<class ANetworkShooterCharacter*> toBeSpawned;

	bool bGameStarted;
	static bool bInGameMenu;
};



