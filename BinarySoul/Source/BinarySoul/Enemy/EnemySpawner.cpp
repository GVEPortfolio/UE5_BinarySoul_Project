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

	FName RowName = FName(*FString::Printf(TEXT("Stage%d"), CurrentStage + 1)); 
    
	FEnemyData* BossData = EnemyDataTable->FindRow<FEnemyData>(RowName, TEXT("Spawner"));

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