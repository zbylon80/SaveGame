// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ShooterGameInstance.generated.h"

class UShooterSaveGame;
class UWorld;

/**
 * 
 */
UCLASS()
class LABA4EDITION_FIRST_API UShooterGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save")
	TSubclassOf<UShooterSaveGame> SaveGameClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save")
	FString SaveSlotName = TEXT("ShooterSaveSlot");

	UFUNCTION(BlueprintCallable, Category = "Save")
	UShooterSaveGame* GetOrCreateSaveGame();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveCurrentGame();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveCurrentLevelName();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveCurrentLevelToSlot();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveCurrentPlayerLocation();

	UFUNCTION(BlueprintCallable, Category = "Save")
	FString GetSavedLevelName();

	UFUNCTION(BlueprintCallable, Category = "Save")
	int32 GetSavedUnlockedLevel();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool OpenSavedLevel();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool UpdateSavedLevelNameOnSaveObject(UShooterSaveGame* SaveGameObject);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool UpdateSavedUnlockedLevelOnSaveObject(UShooterSaveGame* SaveGameObject, int32 UnlockedLevel);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool UpdateSavedPlayerLocationOnSaveObject(UShooterSaveGame* SaveGameObject);

private:
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void RestoreGameplayInput(UWorld* LoadedWorld);
	void TryRestoreSavedPlayerLocation(UWorld* LoadedWorld, int32 RemainingAttempts);

	UPROPERTY(Transient)
	TObjectPtr<UShooterSaveGame> CurrentSaveGame = nullptr;

	bool bRestoreGameplayInputAfterLevelLoad = false;
	bool bRestoreSavedPlayerLocationAfterLevelLoad = false;
	FDelegateHandle PostLoadMapHandle;
};
