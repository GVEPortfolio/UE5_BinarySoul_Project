#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinarySoulTypes.h" // FEnemyData 구조체
#include "EnemySpawner.generated.h"

class ABinaryTarget;

UCLASS()
class BINARYSOUL_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySpawner();

protected:
	virtual void BeginPlay() override;

	// 에디터에서 보스 데이터들이 담긴 엑셀(DataTable)을 넣어줄 곳
	UPROPERTY(EditAnywhere, Category = "Spawner")
	UDataTable* EnemyDataTable;

	// 실제로 월드에 스폰할 클래스 (BP_BinaryTarget)
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<ABinaryTarget> EnemyClass;
};