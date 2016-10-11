// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkShooter.h"
#include "Net/UnrealNetwork.h"
#include "BCGameState.h"

ABCGameState::ABCGameState()
{
	bInMenu = false;
}


void ABCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABCGameState, bInMenu);
}
