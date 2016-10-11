// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NetworkShooter.h"
#include "NetworkShooterGameMode.h"
#include "NetworkShooterHUD.h"
#include "BCPlayerState.h"
#include "WHSpawnPoint.h"
#include "NetworkShooterCharacter.h"

// Declare the static member may be sure the value was set if the game mode was not instanciate
// And keep a value during the travel map
bool ANetworkShooterGameMode::bInGameMenu = true;

ANetworkShooterGameMode::ANetworkShooterGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	PlayerStateClass = ABCPlayerState::StaticClass();

	// use our custom HUD class
	HUDClass = ANetworkShooterHUD::StaticClass();

	bReplicates = true;
}

void ANetworkShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		for (TActorIterator<AWHSpawnPoint> iter(GetWorld()); iter; ++iter) 
		{
			if ((*iter)->team == ETeam::RED_TEAM)
			{
				redSpawnPoint.Add(*iter);
			}
			else
			{
				blueSpawnPoint.Add(*iter);
			}
		}

		//Spawn server
		APlayerController* thisController = GetWorld()->GetFirstPlayerController();

		if (thisController) 
		{
			ANetworkShooterCharacter* thisChar = Cast<ANetworkShooterCharacter>(thisController->GetPawn());

			thisChar->SetTeam(ETeam::BLUE_TEAM);
			blueTeam.Add(thisChar);
			Spawn(thisChar);
		}
	}
}

void ANetworkShooterGameMode::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	if (endPlayReason == EEndPlayReason::EndPlayInEditor ||
		endPlayReason == EEndPlayReason::Quit)
	{
		bInGameMenu = true;
	}
}

void ANetworkShooterGameMode::Tick(float deltaTime)
{
	if (Role == ROLE_Authority)
	{
		APlayerController* thisPlayerController = GetWorld()->GetFirstPlayerController();

		if (toBeSpawned.Num() != 0)
		{
			for (auto charToSpawn : toBeSpawned)
			{
				Spawn(charToSpawn);
			}
		}

		if(thisPlayerController != nullptr && thisPlayerController->IsInputKeyDown(EKeys::R))
		{ 
			bInGameMenu = false;
			GetWorld()->ServerTravel(L"/Game/FirstPersonCPP/Maps/FirstPersonExempleMap?Listen");
		}
	}
}

void ANetworkShooterGameMode::PostLogin(APlayerController* newPlayer)
{
	Super::PostLogin(newPlayer);

	ANetworkShooterCharacter* teamLess = Cast<ANetworkShooterCharacter>(newPlayer->GetPawn());
	ABCPlayerState* abcplayerState = Cast<ABCPlayerState>(newPlayer->PlayerState);

	// Set player player state 
	if (teamLess != nullptr && abcplayerState != nullptr)
	{
		teamLess->SetABCPlayerState(abcplayerState);
	}

	//Assign player to team
	if (Role == ROLE_Authority && teamLess != nullptr)
	{
		if (redTeam.Num() < blueTeam.Num())
		{
			redTeam.Add(teamLess);
			abcplayerState->team = ETeam::RED_TEAM;
		}
		else if (redTeam.Num() > blueTeam.Num())
		{
			blueTeam.Add(teamLess);
			abcplayerState->team = ETeam::BLUE_TEAM;
		}
		else if (redTeam.Num() == blueTeam.Num())
		{
			redTeam.Add(teamLess);
			abcplayerState->team = ETeam::RED_TEAM;
		}

		teamLess->currentTeam = abcplayerState->team;
		teamLess->SetTeam(abcplayerState->team);
		Spawn(teamLess);
	}
}

void ANetworkShooterGameMode::Respawn(class ANetworkShooterCharacter* charaterToRespawn)
{
	if (Role == ROLE_Authority)
	{
		AController* thisPC = charaterToRespawn->GetController();
		charaterToRespawn->DetachFromControllerPendingDestroy();

		ANetworkShooterCharacter* newChara = Cast<ANetworkShooterCharacter>(
			GetWorld()->SpawnActor(DefaultPawnClass));

		if (newChara)
		{
			thisPC->Possess(newChara);
			ABCPlayerState* thisPS = Cast<ABCPlayerState>(newChara->GetController()->PlayerState);

			newChara->currentTeam = thisPS->team;
			newChara->SetABCPlayerState(thisPS);

			Spawn(newChara);

			newChara->SetTeam(newChara->GetABCPlayerState()->team);
		}
	}
}

void ANetworkShooterGameMode::Spawn(class ANetworkShooterCharacter* character)
{
	//Find spawn point that is not blocked
	AWHSpawnPoint* thisPawn = nullptr;
	TArray<class AWHSpawnPoint*>* targetTeam = nullptr;

	if (character->currentTeam == ETeam::BLUE_TEAM)
	{
		targetTeam = &blueSpawnPoint;
	}
	else
	{
		targetTeam = &redSpawnPoint;
	}

	for (auto spawn : (*targetTeam))
	{
		if (!spawn->GetBlocked())
		{
			// Remove from queue location
			if (toBeSpawned.Find(character) != INDEX_NONE)
			{
				toBeSpawned.Remove(character);
			}

			//Otherwise set actor location
			character->SetActorLocation(spawn->GetActorLocation());

			spawn->UpdateOverlaps();

			return;
		}
	}

	if (toBeSpawned.Find(character) == INDEX_NONE)
	{
		toBeSpawned.Add(character);
	}
}