// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BinarySoul/BinarySoulTypes.h"
#include "UBinaryLEDWidget.generated.h"

class UHorizontalBox;
class UImage;

UCLASS()
class BINARYSOUL_API UUBinaryLEDWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* Led_Container;
	
	UPROPERTY()
	TArray<UImage*> Led_Image;
	
	virtual void NativeConstruct() override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "BinarySoul|UI")
	void UpdateLEDVisuals();
};
