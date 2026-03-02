// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerGrabberComponent.h"

// Sets default values for this component's properties
UPlayerGrabberComponent::UPlayerGrabberComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UPlayerGrabberComponent::BeginPlay()
{
	Super::BeginPlay();

	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (!PhysicsHandle)
	{
		UE_LOG(LogTemp, Warning, TEXT("PhysicsHandleComponent required"));
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
	{
		if (GrabAction)
		{
			EIC->BindAction(GrabAction, ETriggerEvent::Started, this, &UPlayerGrabberComponent::Use);
		}
	}
}

// Called every frame
void UPlayerGrabberComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		FVector Start;
		FVector End;
		FRotator CameraRotation;
		GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(Start, CameraRotation);
		End = Start + CameraRotation.Vector() * GrabRange;

		PhysicsHandle->SetTargetLocationAndRotation(End, CameraRotation);
	}
}


AActor* UPlayerGrabberComponent::GetGrabbedActor() const
{
	if (PhysicsHandle)
	{
		if (UPrimitiveComponent* Component = PhysicsHandle->GetGrabbedComponent())
		{
			return Component->GetOwner();
		}
	}

	return nullptr;
}

void UPlayerGrabberComponent::Use()
{
	if (PhysicsHandle)
	{
		if (PhysicsHandle->GetGrabbedComponent())
		{
			Release();
		}
		else
		{
			Grab();
		}

		ItemUpdated.Broadcast();
	}
}

void UPlayerGrabberComponent::Grab()
{
	UE_LOG(LogTemp, Warning, TEXT("UPlayerGrabberComponent::Grab"));

	FVector Start;
	FVector End;
	FRotator CameraRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(Start, CameraRotation);
	End = Start + CameraRotation.Vector() * GrabRange;

	FHitResult Result;
	GetWorld()->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_Visibility);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, 5.0f, 0, 5.0f);

	if (UPrimitiveComponent* PC = Result.GetComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("Grabbed component: %s"), *PC->GetName());

		if (PhysicsHandle)
		{
			PhysicsHandle->GrabComponentAtLocationWithRotation(PC, NAME_None, Result.Location, CameraRotation);
		}
	}
}

void UPlayerGrabberComponent::Release()
{
	PhysicsHandle->ReleaseComponent();
}
