// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"
#include "AdvancedMoveToHelper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMoveCompletedDelegate, EPathFollowingResult::Type, Request_Result);
DECLARE_LOG_CATEGORY_EXTERN(LogAdvancedMoveTo, Display, All);

UCLASS(BlueprintType, meta = (DisplayName = "Advanced Move To Helper"))
class ADVANCEDMOVETOLOCATION_API UAdvancedMoveToHelper : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", DisplayName = "Advanced Move To Location"), Category = "AI|Navigation")
	static UAdvancedMoveToHelper* AdvancedMoveToLocation(AController* Controller, FVector GoalLocation, float AcceptanceRadius);

	UPROPERTY(BlueprintAssignable, Category = "AI|Navigation", meta = (DisplayName = "On Move Finished"))
	FMoveCompletedDelegate OnMoveFinishedEvent;

	UPROPERTY(BlueprintAssignable, Category = "AI|Navigation", meta = (DisplayName = "On Move Failed"))
	FMoveCompletedDelegate OnMoveFailedEvent;
	UPathFollowingComponent* PathFollowingComponent;

	void HandleMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

protected:
	void StartMovement(AController* Controller, FVector GoalLocation, const FAIMoveRequest& MoveRequest);

private:
	static const char* GetResult(const FPathFollowingResult& Result);
};