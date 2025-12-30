#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "BinarySoulTypes.h"
#include "BinaryGameInstance.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLEDDataChanged);
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BinarySoul|Result")
	EFactionColor FinalWinningFaction = EFactionColor::None;
	
	UPROPERTY(BlueprintAssignable, Category = "BinarySoul|Event")
	FOnLEDDataChanged OnLEDUpdated;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BinarySoul|Stage")
	int32 CurrentStageIndex = 0;

	// [신규] 전투 승리 시 호출할 함수 (다음 스테이지로 이동 or 엔딩)
	UFUNCTION(BlueprintCallable, Category = "BinarySoul|Stage")
	void OnBattleWon();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BinarySoul|SaveData")
	FPlayerStats SavedPlayerStats;

	UFUNCTION(BlueprintCallable, Category = "BinarySoul|SaveData")
	void SaveStatsFromCharacter();
	
protected:
	void CheckChoicePhaseEnd();
};

