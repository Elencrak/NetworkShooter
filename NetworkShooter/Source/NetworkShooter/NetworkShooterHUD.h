// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "NetworkShooterGameMode.h"
#include "NetworkShooterCharacter.h"
#include "BCNetworkAPI/BCGameState.h"
#include "BCNetworkAPI/BCPlayerState.h"
#include "NetworkShooterHUD.generated.h"

UCLASS()
class ANetworkShooterHUD : public AHUD
{
	GENERATED_BODY()

public:
	ANetworkShooterHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

