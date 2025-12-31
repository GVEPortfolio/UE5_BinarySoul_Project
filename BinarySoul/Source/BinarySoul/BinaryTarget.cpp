// BinaryTarget.cpp
#include "BinaryTarget.h"
#include "Components/CapsuleComponent.h" // 캡슐 컴포넌트 헤더

ABinaryTarget::ABinaryTarget()
{
	PrimaryActorTick.bCanEverTick = false;

    // [변경] ACharacter는 이미 RootComponent가 CapsuleComponent입니다.
    // 따라서 MeshComp 생성 코드는 삭제하고, 기존 설정을 가져와서 씁니다.

    // 1. 캡슐 콜리전 설정 (몬스터 크기)
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    
    // 2. 락온이 되려면 충돌 채널이 열려있어야 함 (Pawn 채널 등)
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    CharacterName = FText::FromString(TEXT("Unknown Enemy"));
}

void ABinaryTarget::BeginPlay()
{
	Super::BeginPlay();
    CurrentHealth = MaxHealth;
}

float ABinaryTarget::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage > 0.f)
    {
        CurrentHealth -= ActualDamage;
        CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
        
        // [신규] "내 체력 변했어!" 라고 방송 (듣고 있는 플레이어/HUD가 반응함)
        OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

        if (CurrentHealth <= 0.0f)
        {
            // 사망 처리 (필요시 델리게이트 해제 등을 위해 죽기 전에 방송 한 번 더 하는 게 좋음)
            GetMesh()->SetSimulatePhysics(true);
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            // Destroy(); // 바로 삭제하면 HUD가 참조하다 튕길 수 있으니 딜레이 후 삭제 추천
        }
    }
    return ActualDamage;
}