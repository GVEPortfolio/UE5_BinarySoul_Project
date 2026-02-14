#include "EnemySpawner.h"
#include "BinarySoul/BinaryGameInstance.h"
#include "BinarySoul/Player//ABinaryCharacter.h"
#include "BinaryTarget.h"
#include "Kismet/GameplayStatics.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	UBinaryGameInstance* GI = Cast<UBinaryGameInstance>(GetWorld()->GetGameInstance());
	if (!GI || !EnemyDataTable) return;

	int32 CurrentStage = GI->CurrentStageIndex; // 예: 0, 1, 2...

	TArray<FName> RowNames = EnemyDataTable->GetRowNames();    
	if (RowNames.IsValidIndex(CurrentStage))
	{
		// 3. 해당 순서의 '진짜 이름'을 알아냅니다. (예: 0번째 줄의 이름이 "NewRow"라면 그걸 가져옴)
		FName TargetRowName = RowNames[CurrentStage];

		// 4. 그 이름으로 데이터를 찾습니다.
		FEnemyData* BossData = EnemyDataTable->FindRow<FEnemyData>(TargetRowName, TEXT("Spawner"));
		if (BossData && EnemyClass)
		{
			FVector SpawnLocation = GetActorLocation();
			FRotator SpawnRotation = GetActorRotation();
        
			ABinaryTarget* Boss = GetWorld()->SpawnActorDeferred<ABinaryTarget>(
				EnemyClass, 
				FTransform(SpawnRotation, SpawnLocation)
			);

			if (Boss)
			{
				Boss->InitializeEnemy(*BossData);
				UGameplayStatics::FinishSpawningActor(Boss, FTransform(SpawnRotation, SpawnLocation));
				UE_LOG(LogTemp, Warning, TEXT("Spawned Boss: %s"), *BossData->Name.ToString());
			}
		}
	}
}