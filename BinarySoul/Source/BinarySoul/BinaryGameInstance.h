#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "BinarySoulTypes.h"
#include "BinaryGameInstance.generated.h"

UCLASS()
class BINARYSOUL_API UBinaryGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UBinaryGameInstance();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BinarySoul|Selection")
	EFactionColor CurrentFaction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BinarySoul|Meta")
	TArray<EFactionColor> LED_Array;
	
	UFUNCTION(BlueprintCallable, Category = "BinarySoul|Meta")
	void UpdateLED(EFactionColor WinFaction);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BinarySoul|Data")
	UDataTable* ChoiceDataTable;
	
	UFUNCTION(BlueprintCallable, Category = "BinarySoul|Data")
	void GetRandomChoices(FChoiceData& OutRed, FChoiceData& OutBlue);
	
	UFUNCTION(BlueprintCallable, Category = "BinarySoul|Selection")
	void ProcessChoice(FChoiceData SelectedData);
};

