#include "BinaryAnimInstance.h"
#include "ABinaryCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBinaryAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AABinaryCharacter>(TryGetPawnOwner());
	if (Character)
	{
		MovementComponent = Character->GetCharacterMovement();
	}
}

void UBinaryAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Character || !MovementComponent) return;

	// 1. 캐릭터의 현재 월드 속도
	FVector Velocity = Character->GetVelocity();

	// 2. 캐릭터가 바라보는 방향 (앞, 오른쪽)
	FVector ForwardVector = Character->GetActorForwardVector();
	FVector RightVector = Character->GetActorRightVector();

	// 3. [핵심] 내적(Dot Product)을 이용해 속도 분해!
	// 정면 방향 속도 구하기 (앞으로 가면 양수, 뒤로 가면 음수)
	ForwardVelocity = FVector::DotProduct(Velocity, ForwardVector);

	// 오른쪽 방향 속도 구하기 (오른쪽이면 양수, 왼쪽이면 음수)
	SideVelocity = FVector::DotProduct(Velocity, RightVector);

	// [디버그] 로그로 확인
	// GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Fwd: %.1f / Side: %.1f"), ForwardVelocity, SideVelocity));
}