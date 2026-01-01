// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BinarySoulTypes.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ABinaryCharacter.generated.h"

// Forward Declarations
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

	/* -------------------------------------------------------------------------- */
	/* Components                                 */
	/* -------------------------------------------------------------------------- */
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UStaticMeshComponent* WeaponMesh;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* WeaponCollisionBox;

	/* -------------------------------------------------------------------------- */
	/* Input & Settings                             */
	/* -------------------------------------------------------------------------- */
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* lookonAction;

	// Movement Settings
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float RunSpeed = 450.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float BackWalkSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SideSpeed = 100.0f;

	/* -------------------------------------------------------------------------- */
	/* Combat & Stats                               */
	/* -------------------------------------------------------------------------- */
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	FPlayerStats PlayerStats;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ComboActionMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float LockOnRange = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float LockOnInterpSpeed = 5.0f;

	/* -------------------------------------------------------------------------- */
	/* UI                                     */
	/* -------------------------------------------------------------------------- */
public:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> HUDClass;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* HUDWidget;

	
	/* -------------------------------------------------------------------------- */
	/* Internal State                               */
	/* -------------------------------------------------------------------------- */
protected:
	bool bIsDead = false;
	bool bIsAttacking = false;
	bool isRunning = false;
	bool bInputQueued = false;
	
	int32 CurrentCombo = 0;
	int32 MaxCombo = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	AActor* CurrentTarget = nullptr;

	/* -------------------------------------------------------------------------- */
	/* Functions                                  */
	/* -------------------------------------------------------------------------- */
protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Input Handlers
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void StartSprint();
	void StopSprint();
	void ToggleLockOn();
	void Attack();

	// Combat Logic
	void UpdateRotationMode();
	void UpdateLockOnRotation(float DeltaTime);

	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetWeaponCollisionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable)
	void ComboActionBegin(); 

	UFUNCTION(BlueprintCallable)
	void ComboCheck();
    
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
public:
	UFUNCTION(BlueprintCallable, Category="Stats")
	void UpdateHealth(float Amount);
protected:
	// Stats & UI Logic
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	void UpdateMaxHealth(float Amount);
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetCurrentHealth() const { return PlayerStats.CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetMaxHealth() const { return PlayerStats.MaxHealth; }
	
	UFUNCTION(BlueprintImplementableEvent, Category="GameLogic")
	void OnDeath();

	UFUNCTION() 
	void OnTargetHealthUpdate(float CurrentHP, float MaxHP);
    
	void UpdateHUDTargetInfo(bool bShow);
};