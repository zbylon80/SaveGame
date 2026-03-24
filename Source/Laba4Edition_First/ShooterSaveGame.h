// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShooterSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class LABA4EDITION_FIRST_API UShooterSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save")
	FName SavedLevelName = NAME_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save")
	int32 SavedUnlockedLevel = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save")
	FVector SavedPlayerLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save")
	FRotator SavedPlayerRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save")
	bool bHasSavedPlayerLocation = false;
};
