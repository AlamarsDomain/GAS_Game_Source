// Fill out your copyright notice in the Description page of Project Settings.


#include "GGAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GGGameplayEffectContext.h"

UGGAttributeSet::UGGAttributeSet()
	: Health(40.0f), MaxHealth(60.0f), Armor(50.0f), MaxArmor(50.0f), CritChance(0.0f), CritMulti(1.5f), LuckyChance(5.0f), DamageAdd(0.0f), DamageMulti(1.0f), Ammo(100.0f),
		Chilled(0.0f), DeChill(10.0f)
{

}

void UGGAttributeSet::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetArmorAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxArmor());
	}
	else if (Attribute == GetChilledAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 100.0f);
	}
}

void UGGAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetInDamageAttribute())
	{
		float InDamageDone = GetInDamage();
		SetInDamage(0.0f);
		if (InDamageDone > 0.0f)
		{
			if (GetArmor() > 0.0f)
			{
				float InDamageDoneToArmor = InDamageDone;
				const FGameplayTag AcidDamageTag = FGameplayTag::RequestGameplayTag(FName("Damage.Type.Acid"), false);
				bool IsAcidDamage = Data.EffectSpec.CapturedSourceTags.GetSpecTags().HasTagExact(AcidDamageTag);
				if (IsAcidDamage)
					InDamageDoneToArmor *= 1.5f;
				float NewArmor = GetArmor();
				const float ArmorDiff = FMath::Min(NewArmor, InDamageDoneToArmor);
				InDamageDone -= ArmorDiff;
				NewArmor -= ArmorDiff;
				SetArmor(FMath::Clamp(NewArmor, 0.0f, GetMaxArmor()));
				if ((GetArmor() <= 0.0f) && !bOutOfArmor)
				{
					if (OnOutOfArmor.IsBound())
					{
						const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
						AActor* Instigator = EffectContext.GetOriginalInstigator();
						AActor* Causer = EffectContext.GetEffectCauser();

						OnOutOfArmor.Broadcast(Instigator, Causer, Data.EffectSpec, Data.EvaluatedData.Magnitude);
					}
				}
				bOutOfArmor = (GetArmor() <= 0.0f);
			}

			if (InDamageDone > 0.0f)
			{
				float InDamageDoneToHealth = InDamageDone;
				const FGameplayTag FireDamageTag = FGameplayTag::RequestGameplayTag(FName("Damage.Type.Fire"), false);
				bool IsFireDamage = Data.EffectSpec.CapturedSourceTags.GetSpecTags().HasTagExact(FireDamageTag);
				if (IsFireDamage)
					InDamageDoneToHealth *= 1.5f;
				const float NewHealth = GetHealth() - InDamageDoneToHealth;

				SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
				if ((GetHealth() <= 0.0f) && !bOutOfHealth)
				{
					if (OnOutOfHealth.IsBound())
					{
						const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
						AActor* Instigator = EffectContext.GetOriginalInstigator();
						AActor* Causer = EffectContext.GetEffectCauser();

						OnOutOfHealth.Broadcast(Instigator, Causer, Data.EffectSpec, Data.EvaluatedData.Magnitude);
					}
				}
			}

			if (OnDamageTaken.IsBound())
			{
				const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
				AActor* Instigator = EffectContext.GetOriginalInstigator();
				AActor* Causer = EffectContext.GetEffectCauser();

				bool IsCrit = false;
				bool IsLucky = false;
				FGGGameplayEffectContext* Context = static_cast<FGGGameplayEffectContext*>(Data.EffectSpec.GetContext().Get());
				if (Context)
				{
					IsCrit = Context->IsCriticalHit();
					IsLucky = Context->IsLuckyHit();
				}

				OnDamageTaken.Broadcast(Instigator, Causer, Data.EffectSpec.CapturedSourceTags.GetSpecTags(), Data.EvaluatedData.Magnitude, IsCrit, IsLucky);
			}
		}
	}
}

void UGGAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttributeOnChange(Attribute, NewValue);
}

void UGGAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttributeOnChange(Attribute, NewValue);
}

void UGGAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, MaxArmor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, CritChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, CritMulti, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, LuckyChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, DamageAdd, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, DamageMulti, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, Ammo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, Chilled, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGAttributeSet, DeChill, COND_None, REPNOTIFY_Always);
}

void UGGAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, Health, OldHealth);
}

void UGGAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, MaxHealth, OldMaxHealth);
}

void UGGAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, Armor, OldArmor);
}

void UGGAttributeSet::OnRep_MaxArmor(const FGameplayAttributeData& OldMaxArmor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, Armor, OldMaxArmor);
}

void UGGAttributeSet::OnRep_CritChance(const FGameplayAttributeData& OldCritChance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, CritChance, OldCritChance);
}

void UGGAttributeSet::OnRep_CritMulti(const FGameplayAttributeData& OldCritMulti)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, CritMulti, OldCritMulti);
}

void UGGAttributeSet::OnRep_LuckyChance(const FGameplayAttributeData& OldLuckyChance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, LuckyChance, OldLuckyChance);
}

void UGGAttributeSet::OnRep_DamageAdd(const FGameplayAttributeData& OldDamageAdd)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, DamageAdd, OldDamageAdd);
}

void UGGAttributeSet::OnRep_DamageMulti(const FGameplayAttributeData& OldDamageMulti)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, DamageMulti, OldDamageMulti);
}

void UGGAttributeSet::OnRep_Ammo(const FGameplayAttributeData& OldAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, Ammo, OldAmmo);
}

void UGGAttributeSet::OnRep_Chilled(const FGameplayAttributeData& OldChilled)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, Chilled, OldChilled);
}

void UGGAttributeSet::OnRep_DeChill(const FGameplayAttributeData& OldDeChill)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGAttributeSet, DeChill, OldDeChill);
}