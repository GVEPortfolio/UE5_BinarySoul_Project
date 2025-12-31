// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BinarySoulTypes.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ABinaryCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UUserWidget; 
class UProgressBar;
class UTextBlock;
class UBoxComponent;
UCLASS()
class BINARYSOUL_API AABinaryCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AABinaryCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* WeaponCollisionBox;

	// [신규] 무기가 적과 닿았을 때 실행될 함수
	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	bool bIsDead = false;
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category="Camera")
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category="Camera")
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input")
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input")
	UInputAction* LookAction;
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetWeaponCollisionEnabled(bool bEnabled);
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input")
	UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* lookonAction;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float RunSpeed = 450.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float BackWalkSpeed = 200.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float BackRunSpeed = 400;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float SideSpeed = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimSequence* AttackAnim;
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	void UpdateHealth(float Amount);
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	void UpdateMaxHealth(float Amount);
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetCurrentHealth() const { return PlayerStats.CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetMaxHealth() const { return PlayerStats.MaxHealth; }
	
	UFUNCTION(BlueprintImplementableEvent, Category="GameLogic")
	void OnDeath();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Stats")
	FPlayerStats PlayerStats;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float LockOnRange = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float LockOnInterpSpeed = 5.0f;
	void UpdateRotationMode();
	void Attack();
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> HUDClass;
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* HUDWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UStaticMeshComponent* WeaponMesh;
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ComboActionMontage;
protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	bool bIsAttacking = false;
	FTimerHandle AttackTimerHandle;
	void OnAttackFinished();
	void StartSprint();
	void StopSprint();
	bool isRunning = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	AActor* CurrentTarget = nullptr;
	void ToggleLockOn();
    
	void UpdateLockOnRotation(float DeltaTime);
	
	UFUNCTION() 
	void OnTargetHealthUpdate(float CurrentHP, float MaxHP);
    
	void UpdateHUDTargetInfo(bool bShow);
	
	int32 CurrentCombo = 0;      // 현재 몇 타째인지
	int32 MaxCombo = 3;          // 최대 콤보 수 (섹션 개수)
	bool bInputQueued = false;   // 공격 도중 키를 미리 눌렀는지 (선입력)

	// [신규] 콤보 체크 함수 (노티파이에서 호출)
	UFUNCTION(BlueprintCallable)
	void ComboActionBegin(); // 콤보 시작 or 선입력 저장

	UFUNCTION(BlueprintCallable)
	void ComboCheck(); // 다음 콤보로 넘어갈지 결정
    
	// 공격 종료 처리
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};

