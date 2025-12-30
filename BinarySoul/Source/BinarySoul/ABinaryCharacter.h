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

UCLASS()
class BINARYSOUL_API AABinaryCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AABinaryCharacter();

protected:
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
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input")
	UInputAction* InteractAction;
	
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

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
};

