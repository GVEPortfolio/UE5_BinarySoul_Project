
#include "BinaryGameInstance.h"
#include "BinarySoulTypes.h"
#include "Kismet/GameplayStatics.h"
#include "ABinaryCharacter.h"

UBinaryGameInstance::UBinaryGameInstance()
{
	LED_Array.Init(EFactionColor::None, 10);
	CurrentStageIndex = 0;
}
void UBinaryGameInstance::ProcessChoice(FChoiceData SelectedData)
{
	if (!LED_Array.IsValidIndex(CurrentStageIndex)) return;

	CurrentFaction = SelectedData.FactionType;
	UpdateLED(CurrentFaction);
	SaveStatsFromCharacter();
	UE_LOG(LogTemp, Warning, TEXT("Stage %d Choice: %s (Faction: %d)"), 
		CurrentStageIndex, *SelectedData.Description, (uint8)CurrentFaction);

	UGameplayStatics::OpenLevel(GetWorld(), FName("BattleLevel"));
}
void UBinaryGameInstance::UpdateLED(EFactionColor WinFaction)
{
	if (LED_Array.IsValidIndex(CurrentStageIndex))
	{
		LED_Array[CurrentStageIndex] = WinFaction;
        
		if (OnLEDUpdated.IsBound())
		{
			OnLEDUpdated.Broadcast();
		}
	}
}
void UBinaryGameInstance::OnBattleWon()
{
	CurrentStageIndex++;
	UE_LOG(LogTemp, Warning, TEXT("Battle Won! Moving to Stage: %d"), CurrentStageIndex);
	if (CurrentStageIndex >= 10)
	{
		CheckChoicePhaseEnd();
	}
	else
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName("MainLevel"));
	}
}
void UBinaryGameInstance::GetRandomChoices(FChoiceData& OutRed, FChoiceData& OutBlue)
{
	if (!ChoiceDataTable) return;
	
	TArray<FName> RowNames = ChoiceDataTable->GetRowNames();
	if (RowNames.Num() <= 2) return;
	
	for (int32 i = RowNames.Num() - 1; i > 0; --i)
	{
		int32 j = FMath::RandRange(0,i);
		RowNames.Swap(i, j);
	}
	FChoiceData* RowA = ChoiceDataTable->FindRow<FChoiceData>(RowNames[0],TEXT(""));
	FChoiceData* RowB = ChoiceDataTable->FindRow<FChoiceData>(RowNames[1],TEXT(""));
	
	if(RowA) OutRed = *RowA;
	if(RowB) OutBlue = *RowB;

}
void UBinaryGameInstance::CheckChoicePhaseEnd()
{
	int32 RedCount = 0;
	int32 BlueCount = 0;
	for (EFactionColor Faction : LED_Array)
	{
		if (Faction == EFactionColor::Red) RedCount++;
		else if (Faction == EFactionColor::Blue) BlueCount++;
	}

	if (RedCount > BlueCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("GAME CLEAR! Winner: RED"));
		// TODO: RedEndingLevel 로 이동
	}
	else if (BlueCount > RedCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("GAME CLEAR! Winner: BLUE"));
		// TODO: BlueEndingLevel 로 이동
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GAME CLEAR! DRAW"));
	}
	UGameplayStatics::OpenLevel(GetWorld(), FName("BattleLevel"));
}
void UBinaryGameInstance::SaveStatsFromCharacter()
{
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (AABinaryCharacter* BP = Cast<AABinaryCharacter>(PlayerChar))
	{
		SavedPlayerStats = BP->PlayerStats; 
        
		UE_LOG(LogTemp, Warning, TEXT("Stats Saved! HP: %.1f"), SavedPlayerStats.CurrentHealth);
	}
}