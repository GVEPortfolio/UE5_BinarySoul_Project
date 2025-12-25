// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/GameplayStatics.h"
#include "ABinaryChoiceButton.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "BinaryGameInstance.h"

ABinaryChoiceButton::ABinaryChoiceButton()
{
	PrimaryActorTick.bCanEverTick = false;

	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	RootComponent = ButtonMesh;

	DescriptionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DescriptionWidget"));
	DescriptionWidget->SetupAttachment(RootComponent);
    
	DescriptionWidget->SetWidgetSpace(EWidgetSpace::World);
	DescriptionWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	
}
void ABinaryChoiceButton::InitializeButton(FChoiceData NewData)
{
	ButtonData = NewData;

	// TODO: 여기서 DescriptionWidget에 할당된 실제 위젯(WBP)의 텍스트를 
	// ButtonData.Description 내용으로 갱신하는 로직을 추가할 수 있습니다.
}

void ABinaryChoiceButton::OnInteracted()
{
	// [수정] AActor에서는 GetWorld()->GetGameInstance()로 접근해야 합니다.
	// Unity의 FindObjectOfType<GameManager>()와 비슷합니다.
	UBinaryGameInstance* GI = Cast<UBinaryGameInstance>(GetWorld()->GetGameInstance());
	if (GI)
	{
		GI->ProcessChoice(ButtonData);
	}
}


