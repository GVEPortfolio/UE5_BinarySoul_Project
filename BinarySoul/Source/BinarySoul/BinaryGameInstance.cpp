
#include "BinaryGameInstance.h"
#include "BinarySoulTypes.h"
UBinaryGameInstance::UBinaryGameInstance()
{
	LED_Array.Init(EFactionColor::None, 10);
}
void UBinaryGameInstance::ProcessChoice(FChoiceData SelectedData)
{
	// 1. 선택한 진영 데이터를 GameInstance에 저장 [cite: 47]
	CurrentFaction = SelectedData.FactionType;

	// 2. 스탯 수정 대신 우선 로그로 확인 (아직 플레이어 체력이 없으므로) [cite: 46]
	// %s는 문자열, %f는 실수, %d는 정수를 출력합니다.
	UE_LOG(LogTemp, Warning, TEXT("Choice Processed! Faction: %d, Cost: %f, Desc: %s"), 
		(uint8)CurrentFaction, SelectedData.HealthCost, *SelectedData.Description);

	// 3. 레벨 전환 (BattleLevel이라는 이름의 맵이 에디터에 있어야 작동합니다) [cite: 47]
	// 맵이 없다면 아래 줄을 주석 처리(//)하면 팅기지 않습니다.
	// UGameplayStatics::OpenLevel(GetWorld(), FName("BattleLevel"));
}
void UBinaryGameInstance::UpdateLED(EFactionColor WinFaction)
{
	for (int32 i = 0; i < LED_Array.Num(); ++i)
	{
		if (LED_Array[i] == EFactionColor::None)
		{
			LED_Array[i] = WinFaction;
			break;
		}
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