// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "NetworkShooterGameMode.h"
#include "BCPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKSHOOTER_API ABCPlayerState : public APlayerState
{
	GENERATED_BODY()


public:
	//Constructor
	ABCPlayerState();

	// Life of current player replicated on all network entities
	UPROPERTY(Replicated)
	float health;

	// Number of player was death
	UPROPERTY(Replicated)
	uint8 deaths;

	// Team of the state set by default to red
	UPROPERTY(Replicated)
	ETeam team;

};
