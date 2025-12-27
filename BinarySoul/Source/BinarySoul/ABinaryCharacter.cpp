// Fill out your copyright notice in the Description page of Project Settings.


#include "ABinaryCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ABinaryChoiceButton.h"

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
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}

void AABinaryCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController ->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
}
void AABinaryCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// MoveAction이 발동(Triggered)되면 Move 함수 실행
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AABinaryCharacter::Move);

		// LookAction이 발동되면 Look 함수 실행
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AABinaryCharacter::Look);

		// InteractAction이 시작(Started)되면 Interact 함수 실행 (한 번만 실행)
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AABinaryCharacter::Interact);
	}

}

void AABinaryCharacter::Move(const FInputActionValue& Value)
{
	// 입력값 (Vector2D: X, Y)
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 카메라가 보고 있는 방향을 기준으로 앞/뒤/왼/오 계산
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// 전방 벡터
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// 우측 벡터
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
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
	// Unity의 Physics.Raycast와 대응되는 LineTrace
	FVector Start = FollowCamera->GetComponentLocation();
	FVector Forward = FollowCamera->GetForwardVector();
	FVector End = Start + (Forward * 800.0f);
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // 나는 무시

	// 트레이스 발사 (ECC_Visibility 채널 사용)
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			// 맞은 물체가 버튼인지 확인하고 Cast
			ABinaryChoiceButton* Button = Cast<ABinaryChoiceButton>(HitActor);
			if (Button)
			{
				Button->OnInteracted(this);
				
				// 디버그용 구체 그리기 (Unity의 Gizmos.DrawWireSphere)
				// #include "DrawDebugHelpers.h" 필요
				// DrawDebugSphere(GetWorld(), HitResult.Location, 20.f, 12, FColor::Green, false, 2.f);
			}
		}
	}
}
void AABinaryCharacter::UpdateHealth(float HealthAmount)
{
	if (bIsDead) return;
	
	CurrentHealth -= HealthAmount;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
	
	// 디버그 로그 출력 (화면 왼쪽 상단에 빨간 글씨)
	if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT("Health Changed: %.1f / %.1f (Amount: %.1f)"), CurrentHealth, MaxHealth, HealthAmount);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
	}
	if (CurrentHealth <= 0.0f)
	{
		bIsDead=false;
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			DisableInput(PC);
		}
		OnDeath();
	}
}

void AABinaryCharacter::UpdateMaxHealth(float Amount)
{
	MaxHealth += Amount;
	if (Amount > MaxHealth)
	{
		UpdateHealth(-Amount);
	}
	else
	{
		CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
	}
}

void AABinaryCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
