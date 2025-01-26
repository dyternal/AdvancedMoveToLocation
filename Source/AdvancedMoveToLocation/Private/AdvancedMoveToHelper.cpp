// Fill out your copyright notice in the Description page of Project Settings.

#include "AdvancedMoveToHelper.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"

DEFINE_LOG_CATEGORY(LogAdvancedMoveTo)

namespace
{
	UPathFollowingComponent* InitNavigationControl(AController& Controller, UAdvancedMoveToHelper* MoveTo)
	{
		AAIController* AsAIController = Cast<AAIController>(&Controller);
		UPathFollowingComponent* PathFollowingComp = nullptr;

		if (AsAIController)
		{
			PathFollowingComp = AsAIController->GetPathFollowingComponent();
		}
		else
		{
			PathFollowingComp = Controller.FindComponentByClass<UPathFollowingComponent>();
			if (PathFollowingComp == nullptr)
			{
				PathFollowingComp = NewObject<UPathFollowingComponent>(&Controller);
				PathFollowingComp->RegisterComponentWithWorld(Controller.GetWorld());
				PathFollowingComp->OnRequestFinished.AddUObject(MoveTo, &UAdvancedMoveToHelper::HandleMoveCompleted);
				PathFollowingComp->Initialize();
			}
		}

		return PathFollowingComp;
	}
}

UAdvancedMoveToHelper* UAdvancedMoveToHelper::AdvancedMoveToLocation(AController* Controller, FVector GoalLocation, float AcceptanceRadius)
{
	UAdvancedMoveToHelper*  AsyncTask = NewObject<UAdvancedMoveToHelper>();
	FAIMoveRequest MoveRequest(GoalLocation);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	AsyncTask->StartMovement(Controller, GoalLocation, MoveRequest);
	return AsyncTask;
}

void UAdvancedMoveToHelper::StartMovement(AController* Controller, FVector GoalLocation, const FAIMoveRequest& MoveRequest)
{
	UNavigationSystemV1* NavSys = Controller ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(Controller->GetWorld()) : nullptr;
	if (NavSys == nullptr || Controller == nullptr || Controller->GetPawn() == nullptr)
	{
		UE_LOG(LogAdvancedMoveTo, Warning, TEXT("[AdvancedMoveTo] AdvancedMoveToLocation called for NavSys:%s Controller:%s controlling Pawn:%s(if any of these is None then there's your problem"),
			*GetNameSafe(NavSys), *GetNameSafe(Controller), Controller ? *GetNameSafe(Controller->GetPawn()) : TEXT("NULL"));
		return;
	}

	PathFollowingComponent = InitNavigationControl(*Controller, this);
	

	if (PathFollowingComponent == nullptr)
	{
		UE_LOG(LogAdvancedMoveTo, Warning, TEXT("[AdvancedMoveTo] PathFollowingComponent is nullptr. (Controller:%s controlling Pawn:%s)"), *GetNameSafe(Controller), Controller ? *GetNameSafe(Controller->GetPawn()) : TEXT("NULL"));
		return;
	}

	if (!PathFollowingComponent->IsPathFollowingAllowed())
	{
		UE_LOG(LogAdvancedMoveTo, Warning, TEXT("[AdvancedMoveTo] PathFollowing is not allowed. (Controller:%s controlling Pawn:%s)"), *GetNameSafe(Controller), Controller ? *GetNameSafe(Controller->GetPawn()) : TEXT("NULL"));
		return;
	}

	const bool bAlreadyAtGoal = PathFollowingComponent->HasReached(GoalLocation, EPathFollowingReachMode::OverlapAgent);

	// script source, keep only one move request at time
	if (PathFollowingComponent->GetStatus() != EPathFollowingStatus::Idle)
	{
		PathFollowingComponent->AbortMove(*NavSys, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest
			, FAIRequestID::AnyRequest, bAlreadyAtGoal ? EPathFollowingVelocityMode::Reset : EPathFollowingVelocityMode::Keep);
	}

	// script source, keep only one move request at time
	if (PathFollowingComponent->GetStatus() != EPathFollowingStatus::Idle)
	{
		PathFollowingComponent->AbortMove(*NavSys, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest);
	}

	if (bAlreadyAtGoal) UE_LOG(LogAdvancedMoveTo, Display, TEXT("[AdvancedMoveTo] Already at goal. (Controller:%s controlling Pawn:%s)"), *GetNameSafe(Controller), Controller ? *GetNameSafe(Controller->GetPawn()) : TEXT("NULL"));
	const FVector AgentNavLocation = Controller->GetNavAgentLocation();
	const ANavigationData* NavData = NavSys->GetNavDataForProps(Controller->GetNavAgentPropertiesRef(), AgentNavLocation);
	if (NavData)
	{
		FPathFindingQuery Query(Controller, *NavData, AgentNavLocation, GoalLocation);
		FPathFindingResult Result = NavSys->FindPathSync(Query);
		if (Result.IsSuccessful())
		{
			
			FAIRequestID _RequestID = PathFollowingComponent->RequestMove(MoveRequest, Result.Path);
			UE_LOG(LogAdvancedMoveTo, Display, TEXT("[AdvancedMoveTo] Move Request %i has started. (Controller:%s controlling Pawn:%s)"), _RequestID.GetID(), *GetNameSafe(Controller), Controller ? *GetNameSafe(Controller->GetPawn()) : TEXT("NULL"));
		}
		else if (PathFollowingComponent->GetStatus() != EPathFollowingStatus::Idle)
		{
			PathFollowingComponent->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
		}
	}
}

void UAdvancedMoveToHelper::HandleMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (Result.IsSuccess())
	{
		OnMoveFinishedEvent.Broadcast(Result.Code);
		UE_LOG(LogAdvancedMoveTo, Display, TEXT("[AdvancedMoveTo] Move Request %i: Move completed."), RequestID.GetID());
	}
	else
	{
		OnMoveFailedEvent.Broadcast(Result.Code);
		UE_LOG(LogAdvancedMoveTo, Warning, TEXT("[AdvancedMoveTo] Move Request %i: Move failed. (Result: %hs)"), RequestID.GetID(), GetResult(Result));
	}
}

const char* UAdvancedMoveToHelper::GetResult(const FPathFollowingResult& Result)
{
	switch (Result.Code)
	{
	case EPathFollowingResult::Success: return "Success";
	case EPathFollowingResult::Invalid: return "Invalid";
	case EPathFollowingResult::Aborted: return "Aborted";
	case EPathFollowingResult::Blocked: return "Blocked";
	case EPathFollowingResult::OffPath: return "OffPath";
	default: return "Unknown";
	}
}