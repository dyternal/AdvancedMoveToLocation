// Fill out your copyright notice in the Description page of Project Settings.

#include "AdvancedMoveToLocation.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Modules/ModuleManager.h"
#include "GameFramework/Actor.h"

IMPLEMENT_MODULE(FAdvancedMoveToLocationModule, AdvancedMoveToLocation);

void FAdvancedMoveToLocationModule::StartupModule()
{
    // Add initialization logic here if necessary
    UE_LOG(LogTemp, Log, TEXT("AdvancedMoveToLocation Module Started"));
}

void FAdvancedMoveToLocationModule::ShutdownModule()
{
    // Add cleanup logic here if necessary
    UE_LOG(LogTemp, Log, TEXT("AdvancedMoveToLocation Module Shutdown"));
}