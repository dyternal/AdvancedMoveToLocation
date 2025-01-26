// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"
#include "AdvancedMoveToHelper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMoveCompletedDelegate, EPathFollowingResult::Type, Request_Result);

UCLASS(BlueprintType, meta = (DisplayName = "Advanced Move To Helper"))
class ADVANCEDMOVETOLOCATION_API UAdvancedMoveToHelper : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", DisplayName = "Advanced Move To Location"), Category = "Advanced Move To Location")
	static UAdvancedMoveToHelper* AdvancedMoveToLocation(AController* Controller, FVector GoalLocation, float AcceptanceRadius);

	UPROPERTY(BlueprintAssignable, Category = "Advanced Move To Location", meta = (DisplayName = "On Move Completed"))
	FMoveCompletedDelegate OnMoveCompletedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Advanced Move To Location", meta = (DisplayName = "On Move Failed"))
	FMoveCompletedDelegate OnMoveFailedEvent;


protected:
	void StartMovement(AController* Controller, FVector GoalLocation, const FAIMoveRequest& MoveRequest);

private:
	void HandleMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) const;
};