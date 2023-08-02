// Fill out your copyright notice in the Description page of Project Settings.


#include "GGEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "GGGameplayAbility.h"
#include "GGAttributeSet.h"

// Sets default values
AGGEnemyCharacter::AGGEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComp");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UGGAttributeSet>("AttributeSet");

}

// Called when the game starts or when spawned
void AGGEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &AGGEnemyCharacter::OnHealthAttributeChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetArmorAttribute()).AddUObject(this, &AGGEnemyCharacter::OnArmorAttributeChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetChilledAttribute()).AddUObject(this, &AGGEnemyCharacter::OnChilledAttributeChanged);

	AttributeSet->OnOutOfHealth.AddUObject(this, &AGGEnemyCharacter::OnOutOfHealthChanged);
	AttributeSet->OnOutOfArmor.AddUObject(this, &AGGEnemyCharacter::OnOutOfArmorChanged);
	AttributeSet->OnDamageTaken.AddUObject(this, &AGGEnemyCharacter::OnDamageTakenChanged);
}

UAbilitySystemComponent* AGGEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGGEnemyCharacter::InitializeAbilities()
{
	// Give Abilities, Server only
	if (!HasAuthority() || !AbilitySystemComponent)
		return;

	for (TSubclassOf<UGGGameplayAbility>& Ability : DefaultAbilities)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this));
	}
}

void AGGEnemyCharacter::InitializeEffects()
{
	if (!AbilitySystemComponent)
		return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (TSubclassOf<UGameplayEffect>& Effect : DefaultEffects)
	{
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle GEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void AGGEnemyCharacter::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged(Data.OldValue, Data.NewValue);
}

void AGGEnemyCharacter::OnArmorAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnArmorChanged(Data.OldValue, Data.NewValue);
}

void AGGEnemyCharacter::OnOutOfHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
	OnOutOfHealth(DamageInstigator, DamageCauser, DamageEffectSpec, DamageMagnitude);
}

void AGGEnemyCharacter::OnOutOfArmorChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
	OnOutOfArmor(DamageInstigator, DamageCauser, DamageEffectSpec, DamageMagnitude);
}

void AGGEnemyCharacter::OnChilledAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnChilledChanged(Data.OldValue, Data.NewValue);
}

void AGGEnemyCharacter::OnDamageTakenChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayTagContainer& GameplayTagContainer, float Damage, bool IsCrit, bool IsLucky)
{
	OnDamageTaken(DamageInstigator, DamageCauser, GameplayTagContainer, Damage, IsCrit, IsLucky);
}

void AGGEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	InitializeEffects();
	InitializeAbilities();
}

// Called every frame
void AGGEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGGEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

