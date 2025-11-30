// Fill out your copyright notice in the Description page of Project Settings.


#include "Tank.h"
#include "Camera/CameraComponent.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

ATank::ATank()
{
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(CapsuleComp);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
}

void ATank::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController) {
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer()) {

			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer)) {
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	SetPlayerEnabled(false);
}

void ATank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PlayerController) {
		FHitResult HitResult;
		PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
		//HitResult.ImpactPoint;
		//DrawDebugLine(GetWorld(), GetActorLocation(), HitResult.ImpactPoint, FColor(255, 0, 0), false, 0);

		RotateTurret(HitResult.ImpactPoint);

		//DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 25.0f, 12, FColor::Red);
	}
}

//Called to bind functionality to input
void ATank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATank::MoveInput);

		EIC->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ATank::TurnInput);

		EIC->BindAction(FireAction, ETriggerEvent::Started, this, &ATank::Fire);
	}
}

void ATank::MoveInput(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();

	FVector DeltaLocation = FVector(0.0f, 0.0f, 0.0f);
	if (GetWorld()) {
		DeltaLocation.X = Speed * InputValue * GetWorld()->GetDeltaSeconds();
	}

	//DeltaLocation.X = Speed * InputValue * UGameplayStatics::GetWorldDeltaSeconds(GetWorld());

	AddActorLocalOffset(DeltaLocation, true);

}

void ATank::TurnInput(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();

	FRotator DeltaRotation = FRotator(0.0f, 0.0f, 0.0f);
	DeltaRotation.Yaw = TurnRate * InputValue * UGameplayStatics::GetWorldDeltaSeconds(GetWorld());


	AddActorLocalRotation(DeltaRotation, true);
}

void ATank::HandleDestruction() {
	Super::HandleDestruction();

	SetActorHiddenInGame(true);
	UE_LOG(LogTemp, Display, TEXT("Tank Destruction"));
	SetActorTickEnabled(false);
	SetPlayerEnabled(false);
	IsAlive = false;
}

void ATank::SetPlayerEnabled(bool Enabled)
{
	if (PlayerController) {
		if (Enabled) {
			EnableInput(PlayerController);
		}
		else {
			DisableInput(PlayerController);
		}
	}

	PlayerController->SetShowMouseCursor(Enabled);
}

void ATank::ResetFire()
{
	CanFire = true;
}

void ATank::Fire() {
	if (CanFire == false) {
		return;
	}

	Super::Fire();

	CanFire = false;

	GetWorldTimerManager().SetTimer(FireRateHandler, this, &ATank::ResetFire, FireRateDelay, false);
}