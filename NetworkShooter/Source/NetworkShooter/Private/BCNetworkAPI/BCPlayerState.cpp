// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkShooter.h"
#include "BCPlayerState.h"


ABCPlayerState::ABCPlayerState()
	:Super()
{
	health = 100.0f;
	deaths = 0;
	team = ETeam::BLUE_TEAM;
}

void ABCPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABCPlayerState,health);
	DOREPLIFETIME(ABCPlayerState, deaths);
	DOREPLIFETIME(ABCPlayerState, team);
}