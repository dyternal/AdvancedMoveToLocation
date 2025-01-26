// Fill out your copyright notice in the Description page of Project Settings.

#include "AdvancedMoveToHelper.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"

namespace
{
	UPathFollowingComponent* InitNavigationControl(AController& Controller)
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
				PathFollowingComp->Initialize();
			}
		}

		return PathFollowingComp;
	}
}

UAdvancedMoveToHelper* UAdvancedMoveToHelper::AdvancedMoveToLocation(AController* Controller, FVector GoalLocation, float AcceptanceRadius)
{
	FAIMoveRequest MoveRequest(GoalLocation);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	UAdvancedMoveToHelper* AsyncTask = NewObject<UAdvancedMoveToHelper>();
	AsyncTask->StartMovement(Controller, GoalLocation, MoveRequest);
	return AsyncTask;
}

void UAdvancedMoveToHelper::StartMovement(AController* Controller, FVector GoalLocation, const FAIMoveRequest& MoveRequest)
{
	
	UNavigationSystemV1* NavSys = Controller ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(Controller->GetWorld()) : nullptr;
	if (NavSys == nullptr || Controller == nullptr || Controller->GetPawn() == nullptr)
	{
		UE_LOG(LogNavigation, Warning, TEXT("AdvancedMoveToLocation called for NavSys:%s Controller:%s controlling Pawn:%s (if any of these is None then there's your problem"),
			*GetNameSafe(NavSys), *GetNameSafe(Controller), Controller ? *GetNameSafe(Controller->GetPawn()) : TEXT("NULL"));
		return;
	}

	UPathFollowingComponent* PFollowComp = InitNavigationControl(*Controller);

	if (PFollowComp == nullptr)
	{
		UE_LOG(LogNavigation, Warning, TEXT("PFollowComp is nullptr"));
		return;
	}

	if (!PFollowComp->IsPathFollowingAllowed())
	{
		UE_LOG(LogNavigation, Warning, TEXT("PathFollowing is not allowed."));
		return;
	}

	const bool bAlreadyAtGoal = PFollowComp->HasReached(GoalLocation, EPathFollowingReachMode::OverlapAgent);

	// script source, keep only one move request at time
	if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
	{
		PFollowComp->AbortMove(*NavSys, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest
			, FAIRequestID::AnyRequest, bAlreadyAtGoal ? EPathFollowingVelocityMode::Reset : EPathFollowingVelocityMode::Keep);
	}

	// script source, keep only one move request at time
	if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
	{
		PFollowComp->AbortMove(*NavSys, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest);
	}

	if (bAlreadyAtGoal)
	{
		PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
		UE_LOG(LogNavigation, Warning, TEXT("Already at goal."));
	}
	else
	{
		const FVector AgentNavLocation = Controller->GetNavAgentLocation();
		const ANavigationData* NavData = NavSys->GetNavDataForProps(Controller->GetNavAgentPropertiesRef(), AgentNavLocation);
		if (NavData)
		{
			FPathFindingQuery Query(Controller, *NavData, AgentNavLocation, GoalLocation);
			FPathFindingResult Result = NavSys->FindPathSync(Query);
			if (Result.IsSuccessful())
			{
				
				PFollowComp->RequestMove(MoveRequest, Result.Path);
				PFollowComp->OnRequestFinished.AddUObject(this, &UAdvancedMoveToHelper::HandleMoveCompleted);
			}
			else if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
			{
				PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
			}
		}
	}
}


void UAdvancedMoveToHelper::HandleMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) const
{
	if (Result.IsSuccess())
	{
		OnMoveCompletedEvent.Broadcast(Result.Code);
	}
	else
	{
		OnMoveFailedEvent.Broadcast(Result.Code);
	}
}

