
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BinarySoulTypes.generated.h"

UENUM(BlueprintType)

enum class EFactionColor : uint8
{
	None = 0 UMETA( DisplayName = "미점등" ),
	Red = 1 UMETA( DisplayName = "빨강 진영" ),
	Blue = 2 UMETA( DisplayName = "파란 진영")
};

USTRUCT(BlueprintType)
struct FChoiceData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthCost;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactionColor FactionType;
	
	
};
USTRUCT(BlueprintType)
struct FPlayerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHealth = 100.0f;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// float AttackPower = 10.0f;
    
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// int32 Gold = 0;
};
USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 1. 기본 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name; // 적 이름 (예: "GwangGaeTo")

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackPower = 10.0f;

	// 2. 외형 (이게 중요합니다! 보스마다 생김새가 다르니까요)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimClass; // 전용 애니메이션 블루프린트

	// 3. 몽타주 (피격, 사망 모션 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* DeathMontage;
};
