// Copyright Epic Games, Inc. All Rights Reserved.

#include "MooMooMadnessGameMode.h"
#include "MooMooMadnessCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMooMooMadnessGameMode::AMooMooMadnessGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_Cow"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
