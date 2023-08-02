// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GGAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// Delegate used to broadcast attribute events.
DECLARE_MULTICAST_DELEGATE_FourParams(FGGAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec& /*EffectSpec*/, float /*EffectMagnitude*/);
DECLARE_MULTICAST_DELEGATE_SixParams(FGGAttributeDamageEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayTagContainer& /*GameplayTagContainer*/, float /*Damage*/, bool /*IsCrit*/, bool /*IsLucky*/);

/**
 * 
 */
UCLASS()
class GAS_GAME_API UGGAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UGGAttributeSet();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, Armor);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxArmor, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxArmor;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, MaxArmor);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData InDamage;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, InDamage);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritChance, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CritChance;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, CritChance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritMulti, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CritMulti;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, CritMulti);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LuckyChance, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData LuckyChance;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, LuckyChance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageAdd, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DamageAdd;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, DamageAdd);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageMulti, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DamageMulti;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, DamageMulti);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Ammo, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Ammo;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, Ammo);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Chilled, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Chilled;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, Chilled);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DeChill, Category = "Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DeChill;
	ATTRIBUTE_ACCESSORS(UGGAttributeSet, DeChill);

	mutable FGGAttributeEvent OnOutOfHealth;
	mutable FGGAttributeEvent OnOutOfArmor;
	mutable FGGAttributeDamageEvent OnDamageTaken;

protected:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Armor(const FGameplayAttributeData& OldArmor);

	UFUNCTION()
	virtual void OnRep_MaxArmor(const FGameplayAttributeData& OldMaxArmor);

	UFUNCTION()
	virtual void OnRep_CritChance(const FGameplayAttributeData& OldCritChance);

	UFUNCTION()
	virtual void OnRep_CritMulti(const FGameplayAttributeData& OldCritMulti);

	UFUNCTION()
	virtual void OnRep_LuckyChance(const FGameplayAttributeData& OldLuckyChance);

	UFUNCTION()
	virtual void OnRep_DamageAdd(const FGameplayAttributeData& OldDamageAdd);

	UFUNCTION()
	virtual void OnRep_DamageMulti(const FGameplayAttributeData& OldDamageMulti);

	UFUNCTION()
	virtual void OnRep_Chilled(const FGameplayAttributeData& OldChilled);

	UFUNCTION()
	virtual void OnRep_DeChill(const FGameplayAttributeData& OldDeChill);

	UFUNCTION()
	virtual void OnRep_Ammo(const FGameplayAttributeData& OldAmmo);

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	bool bOutOfHealth;
	bool bOutOfArmor;
};
