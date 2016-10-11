// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NetworkShooter.h"
#include "NetworkShooterHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"

ANetworkShooterHUD::ANetworkShooterHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshiarTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshiarTexObj.Object;
}


//Don't create ue by cpp is just like hell
void ANetworkShooterHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X),
										   (Center.Y));

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );

	//Add information about player 
	ABCGameState* thisGameState = Cast<ABCGameState>(GetWorld()->GetGameState());

	if (thisGameState != nullptr && thisGameState->bInMenu)
	{
		int blueScreenPos = 50;
		int redScreenPos = Center.Y + 50;
		int nameSpacing = 25;
		int numBlueTeam = 1;
		int numRedTeam = 1;
		
		FString thisString = "BlueTeam:";
		DrawText(thisString, FColor::Blue, 50, blueScreenPos);

		thisString = "RedTeam";
		DrawText(thisString, FColor::Red, 50, redScreenPos);
		for (auto player : thisGameState->PlayerArray)
		{
			ABCPlayerState* thisPlayerState = Cast<ABCPlayerState>(player);

			if (thisPlayerState)
			{
				if (thisPlayerState->team == ETeam::BLUE_TEAM)
				{
					thisString = FString::Printf(TEXT("%s"), &thisPlayerState->PlayerName);

					DrawText(thisString, FColor::Blue, 50, blueScreenPos + nameSpacing * numBlueTeam);

					numBlueTeam++;
				}
				else
				{
					thisString = FString::Printf(TEXT("%s"), &thisPlayerState->PlayerName);

					DrawText(thisString, FColor::Red, 50, redScreenPos + nameSpacing * numRedTeam);

					numRedTeam++;
				}
			}
		}

		//Todo test with authority
		if (GetWorld()->GetAuthGameMode())
		{
			thisString = "Press R to launch map";
			DrawText(thisString, FColor::Yellow, Center.X, Center.Y);

		}
		else
		{
			thisString = "Wainting Server Validation";
			DrawText(thisString, FColor::Yellow, Center.X, Center.Y);
		}
	}
	else
	{
		ANetworkShooterCharacter* chara = Cast<ANetworkShooterCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

		if (chara != nullptr)
		{
			if(chara->GetABCPlayerState())
			{
				FString HUDString = FString::Printf(TEXT("Healt : %f, Score: %d, Death: %d"),
					chara->GetABCPlayerState()->health,
					chara->GetABCPlayerState()->Score,
					chara->GetABCPlayerState()->deaths);

				DrawText(HUDString, FColor::Magenta, Center.X,50);

			}
		}
	}
}

