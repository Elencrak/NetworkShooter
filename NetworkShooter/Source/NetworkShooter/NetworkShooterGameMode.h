// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "NetworkShooterGameMode.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	BLUE_TEAM,
	RED_TEAM
};


UCLASS(minimalapi)
class ANetworkShooterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ANetworkShooterGameMode();
};



