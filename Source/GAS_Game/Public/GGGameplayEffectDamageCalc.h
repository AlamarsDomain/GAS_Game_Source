// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GGGameplayEffectDamageCalc.generated.h"

/**
 * 
 */
UCLASS()
class GAS_GAME_API UGGGameplayEffectDamageCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UGGGameplayEffectDamageCalc();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
