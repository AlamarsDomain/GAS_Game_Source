// Fill out your copyright notice in the Description page of Project Settings.


#include "GGAbilitySystemGlobals.h"

#include "GGGameplayEffectContext.h"

FGameplayEffectContext* UGGAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FGGGameplayEffectContext();
}
