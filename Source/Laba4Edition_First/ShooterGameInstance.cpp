#include "ShooterGameInstance.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterSaveGame.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

void UShooterGameInstance::Init()
{
	Super::Init();

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UShooterGameInstance::HandlePostLoadMap);
	GetOrCreateSaveGame();
}

void UShooterGameInstance::Shutdown()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	Super::Shutdown();
}

UShooterSaveGame* UShooterGameInstance::GetOrCreateSaveGame()
{
	if (CurrentSaveGame)
	{
		return CurrentSaveGame;
	}

	if (!SaveGameClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGameClass is not set on %s."), *GetNameSafe(this));
		return nullptr;
	}

	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		CurrentSaveGame = Cast<UShooterSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));

		if (CurrentSaveGame)
		{
			return CurrentSaveGame;
		}

		UE_LOG(LogTemp, Warning, TEXT("Failed to load save from slot '%s'."), *SaveSlotName);
	}

	CurrentSaveGame = Cast<UShooterSaveGame>(UGameplayStatics::CreateSaveGameObject(SaveGameClass));
	return CurrentSaveGame;
}

bool UShooterGameInstance::SaveCurrentGame()
{
	UShooterSaveGame* SaveGame = GetOrCreateSaveGame();
	if (!SaveGame)
	{
		return false;
	}

	const bool bSavedSuccessfully = UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
	if (!bSavedSuccessfully)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save game to slot '%s'."), *SaveSlotName);
	}

	return bSavedSuccessfully;
}

bool UShooterGameInstance::SaveCurrentLevelName()
{
	return UpdateSavedLevelNameOnSaveObject(GetOrCreateSaveGame());
}

bool UShooterGameInstance::SaveCurrentLevelToSlot()
{
	UShooterSaveGame* SaveGame = GetOrCreateSaveGame();
	if (!UpdateSavedLevelNameOnSaveObject(SaveGame))
	{
		return false;
	}

	if (!UpdateSavedPlayerLocationOnSaveObject(SaveGame))
	{
		return false;
	}

	return SaveCurrentGame();
}

bool UShooterGameInstance::SaveCurrentPlayerLocation()
{
	return UpdateSavedPlayerLocationOnSaveObject(GetOrCreateSaveGame());
}

FString UShooterGameInstance::GetSavedLevelName()
{
	UShooterSaveGame* SaveGame = GetOrCreateSaveGame();
	if (!SaveGame)
	{
		return FString();
	}

	return SaveGame->SavedLevelName.ToString();
}

int32 UShooterGameInstance::GetSavedUnlockedLevel()
{
	UShooterSaveGame* SaveGame = GetOrCreateSaveGame();
	if (!SaveGame)
	{
		return 1;
	}

	return SaveGame->SavedUnlockedLevel;
}

bool UShooterGameInstance::OpenSavedLevel()
{
	UShooterSaveGame* SaveGame = GetOrCreateSaveGame();
	if (!SaveGame || SaveGame->SavedLevelName.IsNone())
	{
		return false;
	}

	bRestoreGameplayInputAfterLevelLoad = true;
	bRestoreSavedPlayerLocationAfterLevelLoad = SaveGame->bHasSavedPlayerLocation;
	UGameplayStatics::OpenLevel(this, SaveGame->SavedLevelName);
	return true;
}

bool UShooterGameInstance::UpdateSavedLevelNameOnSaveObject(UShooterSaveGame* SaveGameObject)
{
	if (!SaveGameObject)
	{
		return false;
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	SaveGameObject->SavedLevelName = FName(*CurrentLevelName);
	CurrentSaveGame = SaveGameObject;
	return true;
}

bool UShooterGameInstance::UpdateSavedUnlockedLevelOnSaveObject(UShooterSaveGame* SaveGameObject, int32 UnlockedLevel)
{
	if (!SaveGameObject)
	{
		return false;
	}

	SaveGameObject->SavedUnlockedLevel = UnlockedLevel;
	CurrentSaveGame = SaveGameObject;
	return true;
}

bool UShooterGameInstance::UpdateSavedPlayerLocationOnSaveObject(UShooterSaveGame* SaveGameObject)
{
	if (!SaveGameObject)
	{
		return false;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not save player location because PlayerPawn was not found."));
		return false;
	}

	SaveGameObject->SavedPlayerLocation = PlayerPawn->GetActorLocation();
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		SaveGameObject->SavedPlayerRotation = PlayerController->GetControlRotation();
	}
	else
	{
		SaveGameObject->SavedPlayerRotation = PlayerPawn->GetActorRotation();
	}

	SaveGameObject->bHasSavedPlayerLocation = true;
	CurrentSaveGame = SaveGameObject;
	return true;
}

void UShooterGameInstance::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (bRestoreGameplayInputAfterLevelLoad)
	{
		bRestoreGameplayInputAfterLevelLoad = false;
		RestoreGameplayInput(LoadedWorld);
	}

	if (bRestoreSavedPlayerLocationAfterLevelLoad)
	{
		TryRestoreSavedPlayerLocation(LoadedWorld, 10);
	}
}

void UShooterGameInstance::RestoreGameplayInput(UWorld* LoadedWorld)
{
	if (!LoadedWorld)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(LoadedWorld, 0);
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not restore gameplay input after loading level because PlayerController was not found."));
		return;
	}

	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = false;
	PlayerController->SetIgnoreMoveInput(false);
	PlayerController->SetIgnoreLookInput(false);
}

void UShooterGameInstance::TryRestoreSavedPlayerLocation(UWorld* LoadedWorld, int32 RemainingAttempts)
{
	if (!bRestoreSavedPlayerLocationAfterLevelLoad || !LoadedWorld)
	{
		return;
	}

	UShooterSaveGame* SaveGame = GetOrCreateSaveGame();
	if (!SaveGame || !SaveGame->bHasSavedPlayerLocation)
	{
		bRestoreSavedPlayerLocationAfterLevelLoad = false;
		return;
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(LoadedWorld, true);
	if (SaveGame->SavedLevelName != FName(*CurrentLevelName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Skipping player location restore because the loaded level does not match the saved level."));
		bRestoreSavedPlayerLocationAfterLevelLoad = false;
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(LoadedWorld, 0);
	if (!PlayerPawn)
	{
		if (RemainingAttempts > 0)
		{
			FTimerDelegate RetryDelegate = FTimerDelegate::CreateUObject(this, &UShooterGameInstance::TryRestoreSavedPlayerLocation, LoadedWorld, RemainingAttempts - 1);
			LoadedWorld->GetTimerManager().SetTimerForNextTick(RetryDelegate);
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("Could not restore player location after loading level because PlayerPawn was not found."));
		bRestoreSavedPlayerLocationAfterLevelLoad = false;
		return;
	}

	PlayerPawn->SetActorLocation(SaveGame->SavedPlayerLocation, false, nullptr, ETeleportType::TeleportPhysics);
	PlayerPawn->SetActorRotation(FRotator(0.f, SaveGame->SavedPlayerRotation.Yaw, 0.f), ETeleportType::TeleportPhysics);

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(LoadedWorld, 0))
	{
		PlayerController->SetControlRotation(SaveGame->SavedPlayerRotation);
	}

	bRestoreSavedPlayerLocationAfterLevelLoad = false;
}

