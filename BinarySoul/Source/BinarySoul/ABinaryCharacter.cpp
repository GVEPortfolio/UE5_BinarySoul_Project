// Fill out your copyright notice in the Description page of Project Settings.

#include "ABinaryCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ABinaryChoiceButton.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "BinaryGameInstance.h"
#include "BinaryTarget.h"                // 적 클래스 헤더
#include "Components/ProgressBar.h"      // HUD 프로그레스바 제어용
#include "Components/TextBlock.h"        // HUD 텍스트 제어용
#include "Blueprint/UserWidget.h"        // 위젯 생성용
#include "Components/BoxComponent.h" // 필수 헤더
#include "Kismet/GameplayStatics.h"
// Sets default values
AABinaryCharacter::AABinaryCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(FName("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(FName("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;
	
    // 기본적으로 탐험 모드(자유 시점)로 시작
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
    
	PlayerStats.MaxHealth = 100.0f;
	PlayerStats.CurrentHealth = PlayerStats.MaxHealth;
	
    // 초기 속도 설정
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), FName("WeaponSocket"));
	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));

	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox"));
	WeaponCollisionBox->SetupAttachment(WeaponMesh);
	
	// 처음엔 꺼둡니다 (평소에 닿으면 안 되니까)
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
}

void AABinaryCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

    // 게임 인스턴스에서 스탯 로드
	UBinaryGameInstance* GI = Cast<UBinaryGameInstance>(GetWorld()->GetGameInstance());
	if (GI)
	{
		PlayerStats = GI->SavedPlayerStats;
		UE_LOG(LogTemp, Warning, TEXT("Stats Loaded! HP: %f"), PlayerStats.CurrentHealth);
	}

    // [신규] HUD 위젯 생성 및 초기화
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// 2. 변환된 PC를 넣어줍니다.
		HUDWidget = CreateWidget<UUserWidget>(PC, HUDClass);
    
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			UpdateHUDTargetInfo(false); 
		}
	}
	WeaponCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AABinaryCharacter::OnWeaponOverlap);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &AABinaryCharacter::OnAttackMontageEnded);
        
		UE_LOG(LogTemp, Error, TEXT(">>>>> Montage Delegate Bound Successfully! : %s"), *AnimInstance->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(">>>>> CRITICAL ERROR: AnimInstance is NULL!"));
	}
}
void AABinaryCharacter::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 나 자신(Player)은 때리면 안 되고, 적(BinaryTarget)만 때려야 함
	if (OtherActor && OtherActor != this && OtherActor->IsA(ABinaryTarget::StaticClass()))
	{
		// 데미지 적용 (예: 20)
		UGameplayStatics::ApplyDamage(OtherActor, 20.0f, GetController(), this, UDamageType::StaticClass());

		UE_LOG(LogTemp, Warning, TEXT("Hit Enemy: %s"), *OtherActor->GetName());
        
		// (선택) 타격 이펙트나 소리를 여기서 재생하면 됨
	}
}

// 4. [신규] 판정 스위치 함수 구현
void AABinaryCharacter::SetWeaponCollisionEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
void AABinaryCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
    {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AABinaryCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AABinaryCharacter::Look);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AABinaryCharacter::Interact);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AABinaryCharacter::Attack);
		
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AABinaryCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AABinaryCharacter::StopSprint);
		
		EnhancedInputComponent->BindAction(lookonAction, ETriggerEvent::Started, this, &AABinaryCharacter::ToggleLockOn);
	}
}

void AABinaryCharacter::Move(const FInputActionValue& Value)
{
	if (bIsAttacking) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
		
        if (isRunning)
        {
             GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
             
        }
        else
        {
            if (CurrentTarget)
            {
                if (MovementVector.Y < 0.0f)
                {
                    GetCharacterMovement()->MaxWalkSpeed = BackWalkSpeed;
                }
                else if (MovementVector.X != 0.0f)
                {
                    GetCharacterMovement()->MaxWalkSpeed = SideSpeed;
                }
                else
                {
                    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
                }
            }
            else
            {
                GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
            }
        }
	}
}

void AABinaryCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AABinaryCharacter::Interact()
{
	FVector Start = FollowCamera->GetComponentLocation();
	FVector Forward = FollowCamera->GetForwardVector();
	FVector End = Start + (Forward * 800.0f);
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			ABinaryChoiceButton* Button = Cast<ABinaryChoiceButton>(HitActor);
			if (Button)
			{
				Button->OnInteracted(this);
			}
		}
	}
}

void AABinaryCharacter::Attack()
{
	if (bIsDead) return;

	if (!bIsAttacking)
	{
		// 공격 시작할 때 콤보 0으로 확실하게 초기화 후 시작
		CurrentCombo = 0; 
		ComboActionBegin();
	}
	else
	{
		// 공격 중일 때만 예약
		// [중요] 마지막 공격 중일 때는 예약을 받지 않아야 콤보가 끝남
		if (CurrentCombo < MaxCombo)
		{
			bInputQueued = true; 
		}
	}
}
void AABinaryCharacter::ComboActionBegin()
{
	if (CurrentCombo >= MaxCombo) return;

	// 콤보 카운트 증가
	CurrentCombo = FMath::Clamp(CurrentCombo + 1, 1, MaxCombo);
	FString SectionName = FString::Printf(TEXT("Combo%d"), CurrentCombo);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ComboActionMontage)
	{
		bIsAttacking = true;

		// [핵심 수정] 몽타주가 이미 재생 중이라면 '점프'만 하고, 아니면 새로 틉니다.
		if (AnimInstance->Montage_IsPlaying(ComboActionMontage))
		{
			// 점프를 하면 '종료 이벤트'가 발생하지 않아 데이터가 유지됩니다!
			AnimInstance->Montage_JumpToSection(FName(*SectionName), ComboActionMontage);
		}
		else
		{
			// 처음 공격할 때는 재생 시작
			AnimInstance->Montage_Play(ComboActionMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName(*SectionName), ComboActionMontage);
		}

		bInputQueued = false; 
	}
}

void AABinaryCharacter::ComboCheck()
{
	UE_LOG(LogTemp, Warning, TEXT(">>>>> ComboCheck Called! InputQueued: %s"), bInputQueued ? TEXT("True") : TEXT("False"));
	if (bInputQueued)
	{
		ComboActionBegin();
	}
}

void AABinaryCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Error, TEXT(">>>>> Montage Ended Called! Interrupted: %s"), bInterrupted ? TEXT("True") : TEXT("False"));
	bIsAttacking = false;
	bInputQueued = false;
	CurrentCombo = 0;
	SetWeaponCollisionEnabled(false);
}
void AABinaryCharacter::StartSprint()
{
	isRunning = true;
    UpdateRotationMode();
}

void AABinaryCharacter::StopSprint()
{
	isRunning = false;
    UpdateRotationMode();
}

void AABinaryCharacter::UpdateRotationMode()
{
    // 조건: 타겟이 있고(AND) 달리는 중이 아닐 때만 -> 전투 태세(Strafe)
    bool bCombatMode = (CurrentTarget != nullptr) && !isRunning;

    if (bCombatMode)
    {
        // 적을 바라보고 게걸음
        bUseControllerRotationYaw = true;
        GetCharacterMovement()->bOrientRotationToMovement = false;
    }
    else
    {
        // 이동하는 방향을 바라봄 (탐험 모드)
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = true;
    }
}

void AABinaryCharacter::ToggleLockOn()
{
    // 1. 락온 해제
	if (CurrentTarget)
	{
        // 기존 타겟의 체력 방송 구독 해제
        if (ABinaryTarget* OldTarget = Cast<ABinaryTarget>(CurrentTarget))
        {
            OldTarget->OnHealthChanged.RemoveDynamic(this, &AABinaryCharacter::OnTargetHealthUpdate);
        }

		CurrentTarget = nullptr;
        UpdateHUDTargetInfo(false); // HUD 끄기

		UE_LOG(LogTemp, Warning, TEXT("Lock-On Disabled"));
	}
    // 2. 락온 시도 (적 찾기)
    else
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABinaryTarget::StaticClass(), FoundActors);

        AActor* ClosestActor = nullptr;
        float MinDistance = LockOnRange;

        for (AActor* Actor : FoundActors)
        {
            if (Actor)
            {
                float Dist = GetDistanceTo(Actor);
                if (Dist < MinDistance)
                {
                    MinDistance = Dist;
                    ClosestActor = Actor;
                }
            }
        }
        
        if (ClosestActor)
        {
            CurrentTarget = ClosestActor;
            
            // 새 타겟의 체력 방송 구독
            if (ABinaryTarget* NewTarget = Cast<ABinaryTarget>(CurrentTarget))
            {
                NewTarget->OnHealthChanged.AddDynamic(this, &AABinaryCharacter::OnTargetHealthUpdate);
            }

            UpdateHUDTargetInfo(true); // HUD 켜기
            UE_LOG(LogTemp, Warning, TEXT("Lock-On Target: %s"), *CurrentTarget->GetName());
        }
    }

    // [핵심] 락온 상태 변화에 따라 회전 모드 갱신
    UpdateRotationMode();
}

void AABinaryCharacter::UpdateLockOnRotation(float DeltaTime)
{
	if (!CurrentTarget) return;

	FVector Start = GetActorLocation();
	FVector Target = CurrentTarget->GetActorLocation();
	Target.Z -= 50.0f; // 적의 허리쯤을 바라보게 조정

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Start, Target);
	FRotator CurrentRotation = GetControlRotation();
	FRotator TargetRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, LockOnInterpSpeed);
	
    if (Controller)
	{
		Controller->SetControlRotation(TargetRotation);
	}
}

// [신규] 적 체력이 변했을 때 호출됨 (델리게이트)
void AABinaryCharacter::OnTargetHealthUpdate(float CurrentHP, float MaxHP)
{
    if (HUDWidget)
    {
        UProgressBar* TargetBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("TargetHealthBar")));
        if (TargetBar && MaxHP > 0)
        {
            TargetBar->SetPercent(CurrentHP / MaxHP);
        }
    }
}

// [신규] 락온 시 HUD 정보(이름, 바) 켜고 끄기
void AABinaryCharacter::UpdateHUDTargetInfo(bool bShow)
{
    if (!HUDWidget) return;

    UProgressBar* TargetBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("TargetHealthBar")));
    UTextBlock* NameText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("TargetNameText")));

    if (bShow)
    {
        if (TargetBar) TargetBar->SetVisibility(ESlateVisibility::Visible);
        if (NameText) 
        {
            NameText->SetVisibility(ESlateVisibility::Visible);
            
            // [수정] 타겟의 이름 변수(FText)를 가져와서 UI에 표시
            if (ABinaryTarget* TargetActor = Cast<ABinaryTarget>(CurrentTarget))
            {
                NameText->SetText(TargetActor->CharacterName);
            }
            else
            {
                NameText->SetText(FText::FromString(TEXT("Target")));
            }
        }
        
        // 켜자마자 현재 체력으로 한 번 업데이트 (꽉 찬 상태가 아닐 수도 있으니)
        if (ABinaryTarget* Target = Cast<ABinaryTarget>(CurrentTarget))
        {
            OnTargetHealthUpdate(Target->CurrentHealth, Target->MaxHealth);
        }
    }
    else
    {
        if (TargetBar) TargetBar->SetVisibility(ESlateVisibility::Hidden);
        if (NameText) NameText->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AABinaryCharacter::OnAttackFinished()
{
	bIsAttacking = false;
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	UE_LOG(LogTemp, Warning, TEXT("Attack Finished!"));
}

void AABinaryCharacter::UpdateHealth(float HealthAmount)
{
	if (bIsDead) return;
	
	PlayerStats.CurrentHealth += HealthAmount;
	PlayerStats.CurrentHealth = FMath::Clamp(PlayerStats.CurrentHealth, 0.0f, PlayerStats.MaxHealth);
	
    if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT("Health Changed: %.1f / %.1f (Amount: %.1f)"), PlayerStats.CurrentHealth, PlayerStats.MaxHealth, HealthAmount);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
	}
	
    if (PlayerStats.CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			DisableInput(PC);
		}
		OnDeath();
	}
}

void AABinaryCharacter::UpdateMaxHealth(float Amount)
{
	PlayerStats.MaxHealth += Amount;
	if (Amount > PlayerStats.MaxHealth)
	{
		UpdateHealth(Amount);
	}
	else
	{
		PlayerStats.CurrentHealth = FMath::Clamp(PlayerStats.CurrentHealth, 0.0f, PlayerStats.MaxHealth);
	}
}

void AABinaryCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
    // [수정] 타겟이 있고 + 달리는 중이 아닐 때만 시점 강제 고정
	if (CurrentTarget && !isRunning)
	{
		UpdateLockOnRotation(DeltaTime);
	}
}