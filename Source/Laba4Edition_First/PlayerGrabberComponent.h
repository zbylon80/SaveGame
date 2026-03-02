// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

#include "PlayerGrabberComponent.generated.h" // ten include musi byc na koncu


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGrabbedItemUpdated);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LABA4EDITION_FIRST_API UPlayerGrabberComponent : public UActorComponent
{
	GENERATED_BODY()// to macro musi byc na poczatku definicji klasy

public:	
	// Sets default values for this component's properties
	UPlayerGrabberComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	AActor* GetGrabbedActor() const;

	UPROPERTY(BlueprintAssignable)
	FGrabbedItemUpdated ItemUpdated;

protected:
	void Use();
	void Grab();
	void Release();

private:

	UPROPERTY(EditAnywhere, Category = "Grab")
	float GrabRange = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Grab")
	UInputAction* GrabAction = nullptr;

	UPhysicsHandleComponent* PhysicsHandle = nullptr;
};
