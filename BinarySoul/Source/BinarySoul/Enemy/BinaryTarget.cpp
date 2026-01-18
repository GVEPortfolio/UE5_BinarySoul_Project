// BinaryTarget.cpp
#include "BinaryTarget.h"
#include "BinarySoul/BinaryGameInstance.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BinarySoul/BinarySoulTypes.h"

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
    
    TArray<UBoxComponent*> AllBoxes;
    GetComponents<UBoxComponent>(AllBoxes);

    for (UBoxComponent* Box : AllBoxes)
    {
        if (Box && Box->ComponentHasTag(FName("Hitbox")))
        {
            Box->OnComponentBeginOverlap.AddDynamic(this, &ABinaryTarget::OnWeaponOverlap);
        }
    }
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
void ABinaryTarget::Attack()
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && AttackMontage && !AnimInstance->Montage_IsPlaying(AttackMontage))
    {
        AnimInstance->Montage_Play(AttackMontage);

        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &ABinaryTarget::OnAttackMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
    }
}

void ABinaryTarget::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // 나중에 AI에게 "나 공격 끝났어"라고 알려주는 코드가 여기 들어갈 예정입니다.
    // 지금은 비워둡니다.
}
void ABinaryTarget::SetHitboxActive(FName TagName, bool bEnable)
{
    // 내 몸에 붙은 모든 BoxComponent를 가져옵니다.
    TArray<UBoxComponent*> AllBoxes;
    GetComponents<UBoxComponent>(AllBoxes);

    for (UBoxComponent* Box : AllBoxes)
    {
        // 1. 컴포넌트 태그가 일치하는지 확인
        if (Box && Box->ComponentHasTag(TagName))
        {
            if (bEnable)
            {
                Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                Box->SetHiddenInGame(false); // 디버그용 (빨간색 보임)
            }
            else
            {
                Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Box->SetHiddenInGame(true); // 숨김
            }
        }
    }
}

void ABinaryTarget::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                    bool bFromSweep, const FHitResult& SweepResult)
{
    // 1. 나 자신이나, 내가 아닌 다른 엉뚱한 것과 충돌했는지 체크
    if (OtherActor == nullptr || OtherActor == this) return;

    // 2. 이미 데미지를 입힐 대상인지 확인 (같은 공격에 10번 맞으면 안 되니까)
    // (지금은 단순하게 갑니다. 나중에 '중복 피격 방지' 로직을 추가할 수 있습니다.)

    // 3. 로그 출력
    UE_LOG(LogTemp, Warning, TEXT("⚔️ HIT! Giving Damage to: %s"), *OtherActor->GetName());

    // 4. 데미지 전달! (데미지 양: 10)
    // ApplyDamage(맞은 놈, 데미지 양, 공격한 놈의 컨트롤러, 공격한 놈(나), 데미지 타입)
    UGameplayStatics::ApplyDamage(OtherActor, 10.0f, GetController(), this, UDamageType::StaticClass());
}