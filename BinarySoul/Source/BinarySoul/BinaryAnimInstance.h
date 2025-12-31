#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BinaryAnimInstance.generated.h"

class AABinaryCharacter;
class UCharacterMovementComponent;

UCLASS()
class BINARYSOUL_API UBinaryAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 초기화 함수 (Start)
	virtual void NativeInitializeAnimation() override;
    
	// 매 프레임 업데이트 함수 (Tick)
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 블루프린트에서 가져다 쓸 변수들 (BlueprintReadOnly 필수!)
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float ForwardVelocity;

	// [신규] 좌우 속도 (+:오른쪽, -:왼쪽)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float SideVelocity;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling; // 공중에 떠있는가? (점프 중)

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove; // 움직이고 있는가?

	// 참조 캐싱 (매번 Cast 하면 느리니까 저장해둠)
	UPROPERTY(BlueprintReadOnly, Category = "References")
	AABinaryCharacter* Character;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	UCharacterMovementComponent* MovementComponent;
};