// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinarySoulTypes.h"
#include "ABinaryChoiceButton.generated.h"
class UWidgetComponent;
class UStaticMeshComponent;
class AABinaryCharacter;
UCLASS()
class BINARYSOUL_API ABinaryChoiceButton : public AActor
{
	GENERATED_BODY()
	
public:	
	ABinaryChoiceButton();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ButtonMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* DescriptionWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	FChoiceData ButtonData;
	
public:	
	UFUNCTION(BlueprintCallable, Category = "Choice")
	void InitializeButton(FChoiceData NewData);
	UFUNCTION(BlueprintCallable, Category="Choice")
	void OnInteracted(AABinaryCharacter* PlayerCharacter);
};
