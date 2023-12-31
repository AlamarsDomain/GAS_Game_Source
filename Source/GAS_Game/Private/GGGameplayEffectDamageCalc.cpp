// Fill out your copyright notice in the Description page of Project Settings.


#include "GGGameplayEffectDamageCalc.h"
#include "GGAttributeSet.h"
#include "GGGameplayEffectContext.h"

struct FDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(InDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritMulti);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LuckyChance);

	FDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGGAttributeSet, InDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGGAttributeSet, CritChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGGAttributeSet, CritMulti, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGGAttributeSet, LuckyChance, Source, false);
	}
};

static const FDamageStatics DamageStatics()
{
	static FDamageStatics DmgStatics;
	return DmgStatics;
}

UGGGameplayEffectDamageCalc::UGGGameplayEffectDamageCalc()
{
	RelevantAttributesToCapture.Add(DamageStatics().InDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CritChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CritMultiDef);
	RelevantAttributesToCapture.Add(DamageStatics().LuckyChanceDef);
}

void UGGGameplayEffectDamageCalc::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	AActor* TargetActor = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Retrieve the InDamage value passed to Assign Tag Set By Caller Magnitude (On output of Make Outgoing Spec)
	float InDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().InDamageDef, EvaluationParameters, InDamage);

	bool IsCrit = false;
	bool IsLucky = false;
	float CritChance = 0.0f;
	float CritMulti = 0.0f;
	float LuckyChance = 0.0f;

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritChanceDef, EvaluationParameters, CritChance);
	const FHitResult* Hit = Spec.GetContext().GetHitResult();
	if (Hit && Hit->BoneName == "head")
	{
		CritChance += 100.0f;
	}

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritMultiDef, EvaluationParameters, CritMulti);
	if (CritMulti > 1.0f)
	{
		IsCrit = FMath::RandRange(0.01f, 100.0f) <= CritChance;
		if (IsCrit)
		{
			InDamage *= CritMulti;
		}
	}

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().LuckyChanceDef, EvaluationParameters, LuckyChance);
	if (LuckyChance > 0.0f)
	{
		float LuckyMulti = 1.0f;
		while (FMath::RandRange(0.01f, 100.0f) <= LuckyChance)
		{
			LuckyChance -= 100.0f;
			LuckyMulti += 1.0f;
			IsLucky = true;
		}

		if (IsLucky)
		{
			InDamage *= LuckyMulti;
		}
	}

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().InDamageProperty, EGameplayModOp::Additive, InDamage));

	FGameplayEffectSpec* MutableSpec = ExecutionParams.GetOwningSpecForPreExecuteMod();
	FGGGameplayEffectContext* Context = static_cast<FGGGameplayEffectContext*>(MutableSpec->GetContext().Get());
	if (Context)
	{
		Context->SetIsCriticalHit(IsCrit);
		Context->SetIsLuckyHit(IsLucky);
	}

	//UE_LOG(LogTemp, Log, TEXT("Damage - %f"), InDamage);
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("Damage - %f"), InDamage));
}

