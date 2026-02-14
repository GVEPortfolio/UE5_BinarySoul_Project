#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_AttackCollision.generated.h"

UCLASS()
class BINARYSOUL_API UANS_AttackCollision : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FName HitboxTagName;
	// 애니메이션 구간 시작될 때 (판정 ON)
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	// 애니메이션 구간 끝날 때 (판정 OFF)
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
};