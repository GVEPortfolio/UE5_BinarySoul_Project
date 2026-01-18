// Fill out your copyright notice in the Description page of Project Settings.

#include "ABinaryCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "BinarySoul/Object//ABinaryChoiceButton.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "BinarySoul/BinaryGameInstance.h"
#include "BinarySoul/Enemy/BinaryTarget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Perception/AISense_Sight.h"

// 생성자: 컴포넌트 초기화 및 기본 설정
AABinaryCharacter::AABinaryCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. 카메라 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(FName("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(FName("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;
	
    // 2. 이동/회전 설정
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    
	// 3. 스탯 초기화
	PlayerStats.MaxHealth = 100.0f;
	PlayerStats.CurrentHealth = PlayerStats.MaxHealth;
	
	// 4. 무기 및 충돌체 설정
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), FName("WeaponSocket"));
	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));

	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox"));
	WeaponCollisionBox->SetupAttachment(WeaponMesh);
	
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
    
	StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
	StimuliSource->RegisterWithPerceptionSystem();
}

// BeginPlay: 게임 시작 시 로직 (입력 매핑, 데이터 로드, UI 생성, 델리게이트 연결)
void AABinaryCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 1. Enhanced Input 매핑
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

    // 2. 게임 인스턴스에서 스탯 로드
	UBinaryGameInstance* GI = Cast<UBinaryGameInstance>(GetWorld()->GetGameInstance());
	if (GI)
	{
		PlayerStats = GI->SavedPlayerStats;
		UE_LOG(LogTemp, Warning, TEXT("Stats Loaded! HP: %f"), PlayerStats.CurrentHealth);
	}

    // 3. HUD 위젯 생성
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		HUDWidget = CreateWidget<UUserWidget>(PC, HUDClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			UpdateHUDTargetInfo(false); 
		}
	}

	// 4. 델리게이트(이벤트) 연결
	WeaponCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AABinaryCharacter::OnWeaponOverlap);
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &AABinaryCharacter::OnMontageEnded);
		UE_LOG(LogTemp, Warning, TEXT(">>>>> Montage Delegate Bound Successfully!"));
	}
}

// Tick: 매 프레임 실행 (락온 시점 갱신)
void AABinaryCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
	if (isDodge)
	{
		GetCharacterMovement()->Velocity = CurrentDodgeDirection * DodgeSpeed;
	}
	if (CurrentTarget)
	{
		ABinaryTarget* TargetEnemy = Cast<ABinaryTarget>(CurrentTarget);
		if (TargetEnemy->IsDead())
		{
			ToggleLockOn();
		}
		else
		{
			UpdateLockOnRotation(DeltaTime);
		}
	}
	bool bIsMoving = GetVelocity().SizeSquared() > 1.0f;
	if (isRunning&& bIsMoving)
	{
		float SprintCost = 5.0f; 
		PlayerStats.CurrentStamina -= SprintCost * DeltaTime;
		if (PlayerStats.CurrentStamina <= 0.0f)
		{
			PlayerStats.CurrentStamina = 0.0f;
			StopSprint();
		}
	}
	else 
	{
		bool bIsActing = isDodge || bIsAttacking;

		if (!bIsActing && PlayerStats.CurrentStamina < PlayerStats.MaxStamina)
		{
			PlayerStats.CurrentStamina += PlayerStats.StaminaRegenRate * DeltaTime;
            
			if (PlayerStats.CurrentStamina > PlayerStats.MaxStamina)
			{
				PlayerStats.CurrentStamina = PlayerStats.MaxStamina;
			}
		}
	}
}

// 입력 바인딩 설정
void AABinaryCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
    {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AABinaryCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AABinaryCharacter::Look);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AABinaryCharacter::Interact);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AABinaryCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &AABinaryCharacter::Dodge);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AABinaryCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AABinaryCharacter::StopSprint);
		
		EnhancedInputComponent->BindAction(lookonAction, ETriggerEvent::Started, this, &AABinaryCharacter::ToggleLockOn);

	}
}

/* -------------------------------------------------------------------------- */
/* Input Handler Functions                         */
/* -------------------------------------------------------------------------- */

// 이동 입력 처리
void AABinaryCharacter::Move(const FInputActionValue& Value)
{
	if (bIsAttacking||isDodge) return;
	
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
            // 락온 상태일 때의 속도 제어 (뒷걸음질 등)
            if (CurrentTarget)
            {
                if (MovementVector.Y < 0.0f) GetCharacterMovement()->MaxWalkSpeed = BackWalkSpeed;
                else if (MovementVector.X != 0.0f) GetCharacterMovement()->MaxWalkSpeed = SideSpeed;
                else GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
            }
            else
            {
                GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
            }
        }
	}
}

// 시점 회전 입력 처리
void AABinaryCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// 상호작용 (버튼 등)
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
			if (Button) Button->OnInteracted(this);
		}
	}
}

// 달리기 시작
void AABinaryCharacter::StartSprint()
{
	isRunning = true;
    UpdateRotationMode();
}

// 달리기 종료
void AABinaryCharacter::StopSprint()
{
	isRunning = false;
    UpdateRotationMode();
}

// 락온 토글 (켜기/끄기)
void AABinaryCharacter::ToggleLockOn()
{
    // 1. 락온 해제
	if (CurrentTarget)
	{
        if (ABinaryTarget* OldTarget = Cast<ABinaryTarget>(CurrentTarget))
        {
            OldTarget->OnHealthChanged.RemoveDynamic(this, &AABinaryCharacter::OnTargetHealthUpdate);
        }
		CurrentTarget = nullptr;
        UpdateHUDTargetInfo(false); 
	}
    // 2. 락온 시도
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
            if (ABinaryTarget* NewTarget = Cast<ABinaryTarget>(CurrentTarget))
            {
                NewTarget->OnHealthChanged.AddDynamic(this, &AABinaryCharacter::OnTargetHealthUpdate);
            }
            UpdateHUDTargetInfo(true);
        }
    }
    UpdateRotationMode();
}
void AABinaryCharacter::Dodge()
{
	if (isDodge || bIsAttacking) return;
    
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DodgeActionMontage)
	{
		if (PlayerStats.CurrentStamina < 10.0f) return; 
		isDodge = true;
		UpdateStamina(10.0f);
		UpdateRotationMode();
		FVector InputDir = GetLastMovementInputVector();
		if (InputDir.SizeSquared() > 0.0f)
		{
			CurrentDodgeDirection = InputDir.GetSafeNormal();
		}
		else
		{
			CurrentDodgeDirection = GetActorForwardVector();
		}

		SetActorRotation(CurrentDodgeDirection.Rotation());
		AnimInstance->Montage_Play(DodgeActionMontage, 1.3f);
       
	}
}

void AABinaryCharacter::SetInvincibleEnabled(bool bEnabled)
{
	bIsInvincible=bEnabled;
}
void AABinaryCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ComboActionMontage)
	{
		bIsAttacking = false;
		bInputQueued = false;
		CurrentCombo = 0;
		SetWeaponCollisionEnabled(false);
	}
	else if (Montage == DodgeActionMontage)
	{
		isDodge = false;
		SetInvincibleEnabled(false);
		UpdateRotationMode();
	}
}
/* -------------------------------------------------------------------------- */
/* Combat Functions                             */
/* -------------------------------------------------------------------------- */

// 공격 입력 처리 (콤보 시작 또는 예약)
void AABinaryCharacter::Attack()
{
	if (bIsDead) return;
	if (PlayerStats.CurrentStamina < 20.0f) return;
	if (!bIsAttacking && !isDodge)
	{
		CurrentCombo = 0; 
		ComboActionBegin();
	}
	else
	{
		if (CurrentCombo < MaxCombo)
		{
			bInputQueued = true; 
			UE_LOG(LogTemp, Warning, TEXT("Combo Queued!"));
		}	
	}
}

// 콤보 실행 로직 (Montage Jump 활용)
void AABinaryCharacter::ComboActionBegin()
{
	if (CurrentCombo >= MaxCombo) return;
	CurrentCombo = FMath::Clamp(CurrentCombo + 1, 1, MaxCombo);
	FString SectionName = FString::Printf(TEXT("Combo%d"), CurrentCombo);
	UpdateStamina(20.0f);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ComboActionMontage)
	{
		bIsAttacking = true;

		if (AnimInstance->Montage_IsPlaying(ComboActionMontage))
		{
			AnimInstance->Montage_JumpToSection(FName(*SectionName), ComboActionMontage);
		}
		else
		{
			AnimInstance->Montage_Play(ComboActionMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName(*SectionName), ComboActionMontage);
		}
		bInputQueued = false; 
	}
}

// 노티파이에서 호출: 다음 콤보가 예약되어 있는지 확인
void AABinaryCharacter::ComboCheck()
{
	if (bInputQueued)
	{
		ComboActionBegin();
	}
}

// 무기 충돌 감지 처리
void AABinaryCharacter::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor->IsA(ABinaryTarget::StaticClass()))
	{
		UGameplayStatics::ApplyDamage(OtherActor, 20.0f, GetController(), this, UDamageType::StaticClass());
		UE_LOG(LogTemp, Warning, TEXT("Hit Enemy: %s"), *OtherActor->GetName());
	}
}

// 무기 충돌체 활성화/비활성화 (노티파이 연동)
void AABinaryCharacter::SetWeaponCollisionEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		WeaponCollisionBox->SetHiddenInGame(true);
		WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// 전투/비전투 상태에 따른 회전 모드 변경
void AABinaryCharacter::UpdateRotationMode()
{
    bool bCombatMode = (CurrentTarget != nullptr) && !isRunning && !isDodge;

    if (bCombatMode)
    {
        bUseControllerRotationYaw = true;
        GetCharacterMovement()->bOrientRotationToMovement = false;
    }
    else
    {
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = true;
    }
}

// 락온 시 적을 바라보도록 회전 보간
void AABinaryCharacter::UpdateLockOnRotation(float DeltaTime)
{
	if (!CurrentTarget) return;

	FVector Start = GetActorLocation();
	FVector Target = CurrentTarget->GetActorLocation();
	Target.Z -= 50.0f; 

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Start, Target);
	FRotator CurrentRotation = GetControlRotation();
	FRotator TargetRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, LockOnInterpSpeed);
	
    if (Controller)
	{
		Controller->SetControlRotation(TargetRotation);
	}
}
float AABinaryCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 1. 부모 클래스 로직 (방어력 계산 등이 필요할 때 유용)
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
	// 2. 데미지가 없거나 0이하면 무시
	if (ActualDamage <= 0.0f) return 0.0f;

	UpdateHealth(-ActualDamage);

	// 4. 실제로 들어간 데미지 리턴
	return ActualDamage;
}
/* -------------------------------------------------------------------------- */
/* Stats & UI                                   */
/* -------------------------------------------------------------------------- */

void AABinaryCharacter::UpdateHealth(float HealthAmount)
{
	if (bIsDead) return;

	if (HealthAmount < 0.0f && bIsInvincible)
	{
		UE_LOG(LogTemp, Log, TEXT("Ignored Damage due to Invincibility!"));
		return; 
	}

	PlayerStats.CurrentHealth += HealthAmount;
    
	PlayerStats.CurrentHealth = FMath::Clamp(PlayerStats.CurrentHealth, 0.0f, PlayerStats.MaxHealth);

	if (HealthAmount < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("🩸 Ouch! HP Left: %f"), PlayerStats.CurrentHealth);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("💊 Healed! HP: %f"), PlayerStats.CurrentHealth);
	}

	if (PlayerStats.CurrentHealth <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("💀 PLAYER DIED!"));
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
void AABinaryCharacter::UpdateStamina(float StaminaAmount)
{
	if (bIsDead) return;

	PlayerStats.CurrentStamina -= StaminaAmount;
	PlayerStats.CurrentStamina = FMath::Clamp(PlayerStats.CurrentStamina, 0.0f, PlayerStats.MaxStamina);
}
// 델리게이트: 타겟 체력 변경 시 UI 업데이트
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

// HUD 타겟 정보 표시/숨김
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
            if (ABinaryTarget* TargetActor = Cast<ABinaryTarget>(CurrentTarget))
            {
                NameText->SetText(TargetActor->CharacterName);
            }
            else
            {
                NameText->SetText(FText::FromString(TEXT("Target")));
            }
        }
        
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