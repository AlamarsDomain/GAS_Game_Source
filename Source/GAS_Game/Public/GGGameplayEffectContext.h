// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayEffectTypes.h"
#include "GGGameplayEffectContext.generated.h"

USTRUCT()
struct GAS_GAME_API FGGGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_USTRUCT_BODY()

public:

	bool IsCriticalHit() const { return bIsCriticalHit; }
	void SetIsCriticalHit(bool b) { bIsCriticalHit = b; }

	bool IsLuckyHit() const { return bIsLuckyHit; }
	void SetIsLuckyHit(bool b) { bIsLuckyHit = b; }

protected:

	UPROPERTY()
	bool bIsCriticalHit;

	UPROPERTY()
	bool bIsLuckyHit;

public:

	/** Returns the actual struct used for serialization, subclasses must override this! */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGGGameplayEffectContext::StaticStruct();
	}

	/** Creates a copy of this context, used to duplicate for later modifications */
	virtual FGGGameplayEffectContext* Duplicate() const override
	{
		FGGGameplayEffectContext* NewContext = new FGGGameplayEffectContext();
		*NewContext = *this;
		NewContext->AddActors(Actors);
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	/** Custom serialization, subclasses must override this */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
};

template<>
struct TStructOpsTypeTraits<FGGGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FGGGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true		// Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};
