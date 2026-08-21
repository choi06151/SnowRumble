// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowIslandWaterPressureActor_J.h"

#include "../Game/SnowRumbleGameState_C.h"
#include "../Item/Campfire_C.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ASnowIslandWaterPressureActor::ASnowIslandWaterPressureActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(10.0f);
	SetMinNetUpdateFrequency(2.0f);
}

void ASnowIslandWaterPressureActor::BeginPlay()
{
	Super::BeginPlay();

	InitializeControlledWater();
	CurrentWaterZ = InitialWaterZ;
	ApplyWaterZ(CurrentWaterZ);

	if (HasAuthority() && DamageCheckIntervalSeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			DamageTimerHandle,
			this,
			&ASnowIslandWaterPressureActor::HandleDamageTimerElapsed,
			DamageCheckIntervalSeconds,
			true,
			DamageCheckIntervalSeconds);
	}
}

void ASnowIslandWaterPressureActor::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ASnowIslandWaterPressureActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bUseMapShrinkEventTiming)
	{
		CurrentWaterZ = CalculateEventDrivenWaterZ();
	}
	else
	{
		const float RoundElapsedSeconds = GetRoundElapsedSeconds();
		CurrentWaterStage = CalculateWaterStage(RoundElapsedSeconds);
		CurrentWaterZ = CalculateWaterZ(RoundElapsedSeconds);
	}
	ApplyWaterZ(CurrentWaterZ);
}

void ASnowIslandWaterPressureActor::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowIslandWaterPressureActor, CurrentWaterStage);
	DOREPLIFETIME(ASnowIslandWaterPressureActor, CurrentWaterZ);
	DOREPLIFETIME(ASnowIslandWaterPressureActor, bUseMapShrinkEventTiming);
	DOREPLIFETIME(ASnowIslandWaterPressureActor, MapShrinkWaterStartServerTime);
	DOREPLIFETIME(ASnowIslandWaterPressureActor, MapShrinkWaterDurationSeconds);
	DOREPLIFETIME(ASnowIslandWaterPressureActor, MapShrinkWaterStartZ);
	DOREPLIFETIME(ASnowIslandWaterPressureActor, MapShrinkWaterTargetZ);
}

ESnowIslandWaterPressureStage
ASnowIslandWaterPressureActor::GetCurrentWaterStage() const
{
	return CurrentWaterStage;
}

float ASnowIslandWaterPressureActor::GetCurrentWaterZ() const
{
	return CurrentWaterZ;
}

void ASnowIslandWaterPressureActor::StartWaterPressureFromMapShrink(
	int32 ShrinkStage,
	float RoundElapsedSeconds,
	float ShrinkDurationSeconds)
{
	if (!HasAuthority() || ShrinkStage <= 0)
	{
		return;
	}

	bUseMapShrinkEventTiming = true;
	CurrentWaterStage = CalculateWaterStageFromShrinkStage(ShrinkStage);
	MapShrinkWaterDurationSeconds = FMath::Max(0.0f, ShrinkDurationSeconds);
	MapShrinkWaterStartZ = CurrentWaterZ;
	MapShrinkWaterTargetZ = CalculateWaterTargetZFromShrinkStage(ShrinkStage);

	if (const UWorld* World = GetWorld())
	{
		const AGameStateBase* GameState = World->GetGameState();
		MapShrinkWaterStartServerTime = GameState
			? GameState->GetServerWorldTimeSeconds()
			: RoundElapsedSeconds;
	}
	else
	{
		MapShrinkWaterStartServerTime = RoundElapsedSeconds;
	}

	CurrentWaterZ = CalculateEventDrivenWaterZ();
	ApplyWaterZ(CurrentWaterZ);
	ForceNetUpdate();
}

void ASnowIslandWaterPressureActor::OnRep_WaterPressureState()
{
	ApplyWaterZ(CurrentWaterZ);
}

USceneComponent* ASnowIslandWaterPressureActor::ResolveWaterComponent(
	AActor* WaterActor) const
{
	if (!WaterActor)
	{
		return nullptr;
	}

	if (!WaterComponentName.IsNone())
	{
		TArray<USceneComponent*> SceneComponents;
		WaterActor->GetComponents<USceneComponent>(SceneComponents);
		for (USceneComponent* SceneComponent : SceneComponents)
		{
			if (SceneComponent && SceneComponent->GetFName() == WaterComponentName)
			{
				return SceneComponent;
			}
		}
	}

	return WaterActor->GetRootComponent();
}

void ASnowIslandWaterPressureActor::InitializeControlledWater()
{
	ControlledWaterComponent = ResolveWaterComponent(ControlledWaterActor);
	InitialWaterZ = ManualInitialWaterZ;

	if (ControlledWaterComponent.IsValid() && bUseControlledWaterInitialZ)
	{
		InitialWaterZ = ControlledWaterComponent->GetComponentLocation().Z;
	}
}

ESnowIslandWaterPressureStage
ASnowIslandWaterPressureActor::CalculateWaterStage(
	float RoundElapsedSeconds) const
{
	if (RoundElapsedSeconds < StableEndSeconds)
	{
		return ESnowIslandWaterPressureStage::Stable;
	}
	if (RoundElapsedSeconds < OuterFloodEndSeconds)
	{
		return ESnowIslandWaterPressureStage::OuterFlood;
	}
	if (RoundElapsedSeconds < CentralFloodEndSeconds)
	{
		return ESnowIslandWaterPressureStage::CentralFlood;
	}
	return ESnowIslandWaterPressureStage::Overtime;
}

float ASnowIslandWaterPressureActor::CalculateWaterZ(
	float RoundElapsedSeconds) const
{
	if (RoundElapsedSeconds < StableEndSeconds)
	{
		return InitialWaterZ;
	}

	const float OuterFloodDuration =
		FMath::Max(KINDA_SMALL_NUMBER, OuterFloodEndSeconds - StableEndSeconds);
	if (RoundElapsedSeconds < OuterFloodEndSeconds)
	{
		const float Alpha = FMath::Clamp(
			(RoundElapsedSeconds - StableEndSeconds) / OuterFloodDuration,
			0.0f,
			1.0f);
		return FMath::Lerp(InitialWaterZ, OuterFloodWaterZ, Alpha);
	}

	const float CentralFloodDuration =
		FMath::Max(KINDA_SMALL_NUMBER, CentralFloodEndSeconds - OuterFloodEndSeconds);
	if (RoundElapsedSeconds < CentralFloodEndSeconds)
	{
		const float Alpha = FMath::Clamp(
			(RoundElapsedSeconds - OuterFloodEndSeconds) / CentralFloodDuration,
			0.0f,
			1.0f);
		return FMath::Lerp(OuterFloodWaterZ, CentralFloodWaterZ, Alpha);
	}

	return CentralFloodWaterZ
		+ FMath::Max(0.0f, RoundElapsedSeconds - CentralFloodEndSeconds)
		* OvertimeRiseSpeed;
}

ESnowIslandWaterPressureStage
ASnowIslandWaterPressureActor::CalculateWaterStageFromShrinkStage(
	int32 ShrinkStage) const
{
	if (ShrinkStage <= 0)
	{
		return ESnowIslandWaterPressureStage::Stable;
	}
	if (ShrinkStage == 1)
	{
		return ESnowIslandWaterPressureStage::OuterFlood;
	}
	if (ShrinkStage == 2)
	{
		return ESnowIslandWaterPressureStage::CentralFlood;
	}
	return ESnowIslandWaterPressureStage::Overtime;
}

float ASnowIslandWaterPressureActor::CalculateWaterTargetZFromShrinkStage(
	int32 ShrinkStage) const
{
	if (ShrinkStage <= 1)
	{
		return OuterFloodWaterZ;
	}
	if (ShrinkStage == 2)
	{
		return CentralFloodWaterZ;
	}

	const float OvertimeStepSeconds =
		FMath::Max(0.0f, MapShrinkWaterDurationSeconds);
	return CentralFloodWaterZ
		+ FMath::Max(0, ShrinkStage - 2)
			* OvertimeStepSeconds
			* OvertimeRiseSpeed;
}

float ASnowIslandWaterPressureActor::CalculateEventDrivenWaterZ() const
{
	if (MapShrinkWaterDurationSeconds <= 0.0f)
	{
		return MapShrinkWaterTargetZ;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	const float CurrentServerTime = GameState
		? GameState->GetServerWorldTimeSeconds()
		: MapShrinkWaterStartServerTime;
	const float Alpha = FMath::Clamp(
		(CurrentServerTime - MapShrinkWaterStartServerTime)
			/ MapShrinkWaterDurationSeconds,
		0.0f,
		1.0f);
	return FMath::Lerp(MapShrinkWaterStartZ, MapShrinkWaterTargetZ, Alpha);
}

void ASnowIslandWaterPressureActor::ApplyWaterZ(float WaterZ)
{
	USceneComponent* WaterComponent = ControlledWaterComponent.Get();
	if (!WaterComponent)
	{
		return;
	}

	FVector NewWorldLocation = WaterComponent->GetComponentLocation();
	NewWorldLocation.Z = WaterZ;
	WaterComponent->SetWorldLocation(NewWorldLocation);
}

void ASnowIslandWaterPressureActor::HandleDamageTimerElapsed()
{
	if (!HasAuthority()
		|| CurrentWaterStage == ESnowIslandWaterPressureStage::Stable
		|| DamageCheckIntervalSeconds <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ExtinguishSubmergedCampfires();

	if (DamagePerTick <= 0.0f || DamageApplyIntervalSeconds <= 0.0f)
	{
		return;
	}

	TSet<TWeakObjectPtr<ASnowRumbleCharacter>> SeenCharacters;
	for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
	{
		ASnowRumbleCharacter* Character = *It;
		if (!IsValid(Character))
		{
			continue;
		}

		const TWeakObjectPtr<ASnowRumbleCharacter> CharacterKey(Character);
		SeenCharacters.Add(CharacterKey);
		FWaterSubmersionState& SubmersionState =
			SubmersionStates.FindOrAdd(CharacterKey);

		float CapsuleHalfHeight = 0.0f;
		const float SampleZ = CalculateSubmersionSampleZ(
			Character,
			CapsuleHalfHeight);
		const bool bSubmerged = IsCharacterSubmerged(Character);
		const bool bIsFrozen = Character->IsFrozen();
		const bool bIsDead = Character->IsDead();
		Character->SetWaterSubmergedFromServer(
			bSubmerged && !bIsFrozen && !bIsDead);
		const USnowRumbleHealthComponent* HealthComponent =
			Character->FindComponentByClass<USnowRumbleHealthComponent>();
		const float HealthBefore = HealthComponent
			? HealthComponent->GetCurrentHealth()
			: -1.0f;

		float DamageApplied = 0.0f;
		if (bIsFrozen || bIsDead)
		{
			SubmersionState = FWaterSubmersionState();
		}
		else if (bSubmerged)
		{
			SubmersionState.TimeSinceExitSeconds = 0.0f;
			SubmersionState.DamageProgressSeconds += DamageCheckIntervalSeconds;
			ApplyWaterBuoyancyToCharacter(Character, SampleZ);

			if (SubmersionState.DamageProgressSeconds >= DamageApplyIntervalSeconds)
			{
				RequestHazardDamage(Character, DamagePerTick);
				DamageApplied = DamagePerTick;
				SubmersionState.DamageProgressSeconds = 0.0f;
			}
		}
		else
		{
			SubmersionState.TimeSinceExitSeconds += DamageCheckIntervalSeconds;
			if (SubmersionState.TimeSinceExitSeconds >= ExitGraceSeconds)
			{
				SubmersionState.DamageProgressSeconds = 0.0f;
				SubmersionState.TimeSinceExitSeconds = ExitGraceSeconds;
			}
		}

		const float HealthAfter = HealthComponent
			? HealthComponent->GetCurrentHealth()
			: -1.0f;

		if (bLogWaterDamageDebug)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[WaterDamage] Character=%s Submerged=%d DamageProgressSeconds=%.2f TimeSinceExitSeconds=%.2f CurrentWaterZ=%.2f SampleZ=%.2f ActorZ=%.2f CapsuleHalfHeight=%.2f RequiredSubmersionDepth=%.2f IsFrozen=%d IsDead=%d DamageApplied=%.2f HealthBefore=%.2f HealthAfter=%.2f"),
				*GetNameSafe(Character),
				bSubmerged ? 1 : 0,
				SubmersionState.DamageProgressSeconds,
				SubmersionState.TimeSinceExitSeconds,
				CurrentWaterZ,
				SampleZ,
				Character->GetActorLocation().Z,
				CapsuleHalfHeight,
				RequiredSubmersionDepth,
				bIsFrozen ? 1 : 0,
				bIsDead ? 1 : 0,
				DamageApplied,
				HealthBefore,
				HealthAfter);
		}
	}

	for (auto It = SubmersionStates.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !SeenCharacters.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}
}

void ASnowIslandWaterPressureActor::ExtinguishSubmergedCampfires()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
	{
		return;
	}

	for (TActorIterator<ACampfire> It(World); It; ++It)
	{
		ACampfire* Campfire = *It;
		if (IsValid(Campfire) && IsCampfireSubmerged(Campfire))
		{
			Campfire->ExtinguishFromWater();
		}
	}
}

bool ASnowIslandWaterPressureActor::IsCampfireSubmerged(
	const ACampfire* Campfire) const
{
	if (!Campfire)
	{
		return false;
	}

	return Campfire->GetActorLocation().Z
		<= CurrentWaterZ + RequiredSubmersionDepth;
}

float ASnowIslandWaterPressureActor::CalculateSubmersionSampleZ(
	const ASnowRumbleCharacter* Character,
	float& OutCapsuleHalfHeight) const
{
	OutCapsuleHalfHeight = 0.0f;
	if (!Character)
	{
		return 0.0f;
	}

	float SampleZ = Character->GetActorLocation().Z;
	if (const UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
	{
		OutCapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
		SampleZ -= OutCapsuleHalfHeight;
	}

	return SampleZ + SubmersionSampleOffsetZ;
}

bool ASnowIslandWaterPressureActor::IsCharacterSubmerged(
	const ASnowRumbleCharacter* Character) const
{
	if (!Character)
	{
		return false;
	}

	float CapsuleHalfHeight = 0.0f;
	const float SampleZ = CalculateSubmersionSampleZ(
		Character,
		CapsuleHalfHeight);
	return SampleZ <= CurrentWaterZ + RequiredSubmersionDepth;
}

void ASnowIslandWaterPressureActor::ApplyWaterBuoyancyToCharacter(
	ASnowRumbleCharacter* Character,
	float SubmersionSampleZ) const
{
	if (!bApplyWaterBuoyancy || !Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent =
		Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	const float TargetSampleZ =
		CurrentWaterZ - FMath::Max(0.0f, BuoyancyTargetSubmersionDepth);
	const float HeightError = TargetSampleZ - SubmersionSampleZ;
	if (HeightError <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float MaximumUpwardVelocity =
		FMath::Max(0.0f, BuoyancyMaximumUpwardVelocity);
	const float MinimumUpwardVelocity =
		FMath::Min(
			FMath::Max(0.0f, BuoyancyMinimumUpwardVelocity),
			MaximumUpwardVelocity);
	const float DesiredUpwardVelocity = FMath::Clamp(
		HeightError * FMath::Max(0.0f, BuoyancyCorrectionSpeed),
		MinimumUpwardVelocity,
		MaximumUpwardVelocity);

	float BounceUpwardVelocity = 0.0f;
	if (bApplyWaterBounce
		&& WaterBounceFrequency > 0.0f
		&& WaterBounceUpwardVelocity > 0.0f)
	{
		const UWorld* World = GetWorld();
		const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
		const float CharacterPhase =
			static_cast<float>(Character->GetUniqueID() % 100) * 0.137f;
		const float BounceAlpha = FMath::Max(
			0.0f,
			FMath::Sin(
				(CurrentTime * WaterBounceFrequency + CharacterPhase)
				* UE_TWO_PI));
		BounceUpwardVelocity = BounceAlpha * WaterBounceUpwardVelocity;
	}

	FVector NewVelocity = MovementComponent->Velocity;
	NewVelocity.Z = FMath::Max(
		NewVelocity.Z,
		DesiredUpwardVelocity + BounceUpwardVelocity);
	MovementComponent->Velocity = NewVelocity;
	Character->StopJumping();
}

void ASnowIslandWaterPressureActor::RequestHazardDamage(
	AActor* Target,
	float DamageAmount)
{
	if (!HasAuthority() || !Target || DamageAmount <= 0.0f)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(
		Target,
		DamageAmount,
		nullptr,
		this,
		UDamageType::StaticClass());
}

float ASnowIslandWaterPressureActor::GetRoundElapsedSeconds() const
{
	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	return SnowRumbleGameState
		? SnowRumbleGameState->GetRoundElapsedSeconds()
		: 0.0f;
}
