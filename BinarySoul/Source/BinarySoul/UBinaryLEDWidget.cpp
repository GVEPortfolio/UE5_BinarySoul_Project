// Fill out your copyright notice in the Description page of Project Settings.


#include "UBinaryLEDWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "BinaryGameInstance.h"

void UUBinaryLEDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Led_Container)
	{
		Led_Image.Empty();
		
		int32 ChildCount = Led_Container ->GetChildrenCount();
		for (int32 i = 0; i < ChildCount; i++)
		{
			UImage* ImageWidget = Cast<UImage>(Led_Container-> GetChildAt(i));
			if (ImageWidget)
			{
				Led_Image.Add(ImageWidget);
			}
		}
	}
	UBinaryGameInstance* GI = Cast<UBinaryGameInstance>(GetWorld()->GetGameInstance());
	if (GI)
	{
		GI->OnLEDUpdated.AddDynamic(this, &UUBinaryLEDWidget::UpdateLEDVisuals);
	}
	UpdateLEDVisuals();
}

void UUBinaryLEDWidget::UpdateLEDVisuals()
{
	UBinaryGameInstance* GI =Cast<UBinaryGameInstance>(GetWorld()->GetGameInstance());
	if (!GI) return;
	int32 LoopCount = FMath::Min(GI->LED_Array.Num(), Led_Image.Num());
	
	for (int32 i = 0; i < LoopCount; i++)
	{
		EFactionColor Faction = GI ->LED_Array[i];
		UImage* TargetImage = Led_Image[i];
		FLinearColor NewColor= FLinearColor::Black;

		switch (Faction)
		{
		case EFactionColor::Red:
			NewColor = FLinearColor::Red;
			break;
		case EFactionColor::Blue:
			NewColor = FLinearColor::Blue;
			break;
		case EFactionColor::None:
		default:
			NewColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
			break;
		}	
		if (TargetImage)
		{
			TargetImage->SetBrushTintColor(NewColor);
		}
	}
	
}
