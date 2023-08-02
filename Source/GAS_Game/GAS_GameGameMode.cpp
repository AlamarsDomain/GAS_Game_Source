// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS_GameGameMode.h"
#include "GAS_GameCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGAS_GameGameMode::AGAS_GameGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
