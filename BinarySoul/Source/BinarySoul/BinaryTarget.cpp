// BinaryTarget.cpp
#include "BinaryTarget.h"

#include "BinaryGameInstance.h"
#include "Components/CapsuleComponent.h"
#include "BinarySoulTypes.h"

ABinaryTarget::ABinaryTarget()
{
    PrimaryActorTick.bCanEverTick = false;
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
}

void ABinaryTarget::BeginPlay()
{
	Super::BeginPlay();
    CurrentHealth = MaxHealth;
}
void ABinaryTarget::InitializeEnemy(const FEnemyData& Data)
{
    // 1. 스탯 적용
    MaxHealth = Data.MaxHealth;
    CurrentHealth = MaxHealth;
    CharacterName = Data.Name;
    
    // 몽타주 저장 (TakeDamage에서 쓸 수 있게 멤버 변수로 저장해야 함)
    HitReactMontage = Data.HitReactMontage;
    DeathMontage = Data.DeathMontage;

    // 2. 외형(Mesh) 갈아입기
    if (Data.SkeletalMesh)
    {
        GetMesh()->SetSkeletalMesh(Data.SkeletalMesh);
        
        // 위치 조정 (모델마다 중심점이 다를 수 있어서 보통 아래로 좀 내립니다)
        GetMesh()->SetRelativeLocation(FVector(0, 0, -90.0f)); 
        GetMesh()->SetRelativeRotation(FRotator(0, -90.0f, 0));
    }

    // 3. 애니메이션 적용
    if (Data.AnimClass)
    {
        GetMesh()->SetAnimInstanceClass(Data.AnimClass);
    }
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
            bIsDead = true;
            GetMesh()->SetSimulatePhysics(true);
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            UBinaryGameInstance* GI = Cast<UBinaryGameInstance>(GetWorld()->GetGameInstance());
            if (GI)
            {
                FTimerHandle MyTimerHandle;
                FTimerDelegate TimerDel;
                TimerDel.BindUObject(GI, &UBinaryGameInstance::OnBattleWon);
                GetWorld()->GetTimerManager().SetTimer(MyTimerHandle,TimerDel,3.f,false);
            }
            // Destroy(); // 바로 삭제하면 HUD가 참조하다 튕길 수 있으니 딜레이 후 삭제 추천
        }
    }
    return ActualDamage;
}