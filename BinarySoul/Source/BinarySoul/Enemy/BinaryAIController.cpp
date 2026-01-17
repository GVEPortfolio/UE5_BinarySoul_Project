#include "BinaryAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h" 
#include "GameFramework/Character.h"

ABinaryAIController::ABinaryAIController()
{
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    
	if (SightConfig)
	{
		SightConfig->SightRadius = 1000.0f;
		SightConfig->LoseSightRadius = 1500.0f;
		SightConfig->PeripheralVisionAngleDegrees = 60.0f;

		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

		PerceptionComponent->ConfigureSense(*SightConfig);
        
		PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	}
}

void ABinaryAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (PerceptionComponent)
	{
		PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABinaryAIController::OnTargetDetected);
	}
	if (BehaviorTreeAsset && BehaviorTreeAsset->BlackboardAsset)
	{
		BlackboardComp->InitializeBlackboard(*BehaviorTreeAsset->BlackboardAsset);
		RunBehaviorTree(BehaviorTreeAsset);
		
		UE_LOG(LogTemp, Warning, TEXT("AI Started: %s"), *InPawn->GetName());
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

		if (PlayerPawn)
		{
			BlackboardComp->SetValueAsObject(FName("TargetActor"), PlayerPawn);
            
			UE_LOG(LogTemp, Warning, TEXT("Target Locked Immediately: %s"), *PlayerPawn->GetName());
		}
	}
}
void ABinaryAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{

	if (Stimulus.WasSuccessfullySensed())
	{
		BlackboardComp->SetValueAsObject(FName("TargetActor"), Actor);
		UE_LOG(LogTemp, Warning, TEXT("Target Spotted: %s"), *Actor->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Target Lost: %s"), *Actor->GetName());
	}
}