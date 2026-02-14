#include "BinarySoul/Enemy/BTTask_Attack.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BinaryTarget.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ABinaryTarget* ControllingPawn = Cast<ABinaryTarget>(AIController->GetPawn());

	if (ControllingPawn)
	{
		// 적 캐릭터의 공격 함수 호출!
		ControllingPawn->Attack();
		bIsAttacking = true;
        
		// "아직 안 끝났어(InProgress)"라고 보고
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	ABinaryTarget* ControllingPawn = Cast<ABinaryTarget>(AIController->GetPawn());

	// 몽타주가 끝났는지 매 프레임 감시
	if (ControllingPawn)
	{
		UAnimInstance* AnimInstance = ControllingPawn->GetMesh()->GetAnimInstance();
		// 몽타주가 더 이상 재생 중이 아니면 -> 공격 끝
		if (AnimInstance && !AnimInstance->Montage_IsPlaying(ControllingPawn->AttackMontage))
		{
			// 태스크 성공 종료 알림
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}