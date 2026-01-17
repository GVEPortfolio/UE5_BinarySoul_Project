#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BinaryAIController.generated.h"

UCLASS()
class BINARYSOUL_API ABinaryAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABinaryAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UBlackboardComponent* BlackboardComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
};