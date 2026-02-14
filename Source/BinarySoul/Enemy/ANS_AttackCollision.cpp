#include "ANS_AttackCollision.h"
#include "BinaryTarget.h"

void UANS_AttackCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	if (MeshComp && MeshComp->GetOwner())
	{
		ABinaryTarget* Enemy = Cast<ABinaryTarget>(MeshComp->GetOwner());
		if (Enemy)
		{
			// [수정] 태그 이름을 함께 전달
			Enemy->SetHitboxActive(HitboxTagName, true);
		}
	}
}

void UANS_AttackCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (MeshComp && MeshComp->GetOwner())
	{
		ABinaryTarget* Enemy = Cast<ABinaryTarget>(MeshComp->GetOwner());
		if (Enemy)
		{
			// [수정] 끄는 것도 이름 기반으로
			Enemy->SetHitboxActive(HitboxTagName, false);
		}
	}
}