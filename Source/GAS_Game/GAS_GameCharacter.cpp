// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS_GameCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "GGGameplayAbility.h"
#include "GGAttributeSet.h"

//////////////////////////////////////////////////////////////////////////
// AGAS_GameCharacter

AGAS_GameCharacter::AGAS_GameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComp");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UGGAttributeSet>("AttributeSet");

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

UAbilitySystemComponent* AGAS_GameCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGAS_GameCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	InitializeAbilities();
	InitializeEffects();
}

void AGAS_GameCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	BindInput();

	InitializeEffects();
}

void AGAS_GameCharacter::InitializeAbilities()
{
	// Give Abilities, Server only
	if (!HasAuthority() || !AbilitySystemComponent)
		return;

	for (TSubclassOf<UGGGameplayAbility>& Ability : DefaultAbilities)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this));
	}
}

void AGAS_GameCharacter::InitializeEffects()
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

void AGAS_GameCharacter::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged(Data.OldValue, Data.NewValue);
}

void AGAS_GameCharacter::OnArmorAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnArmorChanged(Data.OldValue, Data.NewValue);
}

void AGAS_GameCharacter::OnOutOfHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
	OnOutOfHealth(DamageInstigator, DamageCauser, DamageEffectSpec, DamageMagnitude);
}

void AGAS_GameCharacter::OnOutOfArmorChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
	OnOutOfArmor(DamageInstigator, DamageCauser, DamageEffectSpec, DamageMagnitude);
}

void AGAS_GameCharacter::OnDamageTakenChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayTagContainer& GameplayTagContainer, float Damage, bool IsCrit, bool IsLucky)
{
	OnDamageTaken(DamageInstigator, DamageCauser, GameplayTagContainer, Damage, IsCrit, IsLucky);
}

void AGAS_GameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &AGAS_GameCharacter::OnHealthAttributeChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetArmorAttribute()).AddUObject(this, &AGAS_GameCharacter::OnArmorAttributeChanged);

	AttributeSet->OnOutOfHealth.AddUObject(this, &AGAS_GameCharacter::OnOutOfHealthChanged);
	AttributeSet->OnOutOfArmor.AddUObject(this, &AGAS_GameCharacter::OnOutOfArmorChanged);
	AttributeSet->OnDamageTaken.AddUObject(this, &AGAS_GameCharacter::OnDamageTakenChanged);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGAS_GameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGAS_GameCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGAS_GameCharacter::Look);

		EnhancedInputComponent->BindAction(FireAbilityAction, ETriggerEvent::Triggered, this, &AGAS_GameCharacter::OnFireAbility);
	}

	BindInput();
}

void AGAS_GameCharacter::BindInput()
{
	if (bIsInputBound || !AbilitySystemComponent || !IsValid(InputComponent))
		return;

	FTopLevelAssetPath EnumAssetPath = FTopLevelAssetPath(FName("/Script/GAS_Game"), FName("EAbilityInputID"));
	AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(FString("Confirm"), FString("Cancel"), EnumAssetPath, static_cast<int32>(EAbilityInputID::Confirm), static_cast<int32>(EAbilityInputID::Cancel)));

	bIsInputBound = true;
}

void AGAS_GameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AGAS_GameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AGAS_GameCharacter::OnFireAbility(const FInputActionValue& Value)
{
	SendAbilityLocalInput(Value, static_cast<int32>(EAbilityInputID::FireAbility));
}

void AGAS_GameCharacter::OnActionBar01Ability(const FInputActionValue& Value)
{
	SendAbilityLocalInput(Value, static_cast<int32>(EAbilityInputID::ActionBar01Ability));
}

void AGAS_GameCharacter::OnActionBar02Ability(const FInputActionValue& Value)
{
	SendAbilityLocalInput(Value, static_cast<int32>(EAbilityInputID::ActionBar02Ability));
}

void AGAS_GameCharacter::SendAbilityLocalInput(const FInputActionValue& Value, int32 InputID)
{
	if (!AbilitySystemComponent)
		return;

	if (Value.Get<bool>())
	{
		AbilitySystemComponent->AbilityLocalInputPressed(InputID);
	}
	else
	{
		AbilitySystemComponent->AbilityLocalInputReleased(InputID);
	}
}



