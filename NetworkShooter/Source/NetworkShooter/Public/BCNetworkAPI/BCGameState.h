// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "Net/UnrealNetwork.h"
#include "BCGameState.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKSHOOTER_API ABCGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	ABCGameState();
	
	UPROPERTY(Replicated)
	bool bInMenu;
};
