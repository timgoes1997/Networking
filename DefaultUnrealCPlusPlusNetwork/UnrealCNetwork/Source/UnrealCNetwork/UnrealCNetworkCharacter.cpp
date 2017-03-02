// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "UnrealCNetwork.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "UnrealCNetworkCharacter.h"

//////////////////////////////////////////////////////////////////////////
// AUnrealCNetworkCharacter

AUnrealCNetworkCharacter::AUnrealCNetworkCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	//Tutorial code
	//Init. text render comp
	CharText = CreateDefaultSubobject<UTextRenderComponent>(FName("CharText"));

	//Set a relative location
	CharText->SetRelativeLocation(FVector(0, 0, 100));

	//Attach it to root comp
	CharText->SetupAttachment(GetRootComponent());

	//Centers the text alignment
	CharText->SetHorizontalAlignment(EHTA_Center);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AUnrealCNetworkCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	//Input tutorial
	PlayerInputComponent->BindAction("ThrowBomb", IE_Pressed, this, &AUnrealCNetworkCharacter::AttempToSpawnBomb);

	PlayerInputComponent->BindAxis("MoveForward", this, &AUnrealCNetworkCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUnrealCNetworkCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AUnrealCNetworkCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AUnrealCNetworkCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AUnrealCNetworkCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AUnrealCNetworkCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AUnrealCNetworkCharacter::OnResetVR);
}


void AUnrealCNetworkCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AUnrealCNetworkCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AUnrealCNetworkCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AUnrealCNetworkCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AUnrealCNetworkCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AUnrealCNetworkCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AUnrealCNetworkCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

//From here I have added the tutorial code

void AUnrealCNetworkCharacter::OnRep_Health()
{
	UpdateCharText();
}

void AUnrealCNetworkCharacter::OnRep_BombCount()
{
	UpdateCharText();
}

void AUnrealCNetworkCharacter::InitHealth()
{
	Health = MaxHealth;
	UpdateCharText();
}

void AUnrealCNetworkCharacter::InitBombCount()
{
	BombCount = MaxBombCount;
	UpdateCharText();
}

void AUnrealCNetworkCharacter::UpdateCharText()
{
	//Create a string that will display the health and bomb count values;
	FString NewText = FString("Health: ") + FString::SanitizeFloat(Health) + FString(" Bomb Count: ") + FString::FromInt(BombCount);

	//Set the created string to the text render comp
	CharText->SetText(FText::FromString(NewText));
}

void AUnrealCNetworkCharacter::ServerTakeDamage_Implementation(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

bool AUnrealCNetworkCharacter::ServerTakeDamage_Validate(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	//Assume that everything is ok without any further checks and return true
	return true;
}

void AUnrealCNetworkCharacter::AttempToSpawnBomb()
{
	if (HasBombs()) {
		//If we don't have authority, meaning that we're not the server.
		//tell the server to spawn the bomb.
		//If we're the server, just spawn the bomb - we trust ourselves.
		if (Role < ROLE_Authority) {
			ServerSpawnBomb();
		}
		else {
			SpawnBomb();
		}

		//todo: this code will be removed in the next part
		FDamageEvent DmgEvent;
		if (Role < ROLE_Authority) {
			ServerTakeDamage(25.f, DmgEvent, GetController(), this);
		}
		else {
			TakeDamage(25.f, DmgEvent, GetController(), this);
		}
	}
}

void AUnrealCNetworkCharacter::SpawnBomb()
{
	//Decrease the bomb count and update the text in the local client
	//OnRep_BombCount will be called in every other client
	BombCount--;
	UpdateCharText();

	//todo: spawn the actual bomb in the next part of the tutorial.
}

void AUnrealCNetworkCharacter::ServerSpawnBomb_Implementation()
{
	SpawnBomb();
}

bool AUnrealCNetworkCharacter::ServerSpawnBomb_Validate()
{
	//Assuming everything is ok without any further checks and return true
	return true;
}

float AUnrealCNetworkCharacter::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	//Decrease the character's hp

	Health -= Damage;
	if (Health <= 0) InitHealth(); //Reset the players health

	//Call the update text on the local client
	//OnRep_Health will be called in every other client so the character's text will contain a text with the right values.
	UpdateCharText();

	return Health;
}

void AUnrealCNetworkCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Tell the engine to call the OnRepHealth and OnRep_BombCount eachTime
	//a variable changes
	DOREPLIFETIME(AUnrealCNetworkCharacter, Health);
	DOREPLIFETIME(AUnrealCNetworkCharacter, BombCount);
}

void AUnrealCNetworkCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitHealth();
	InitBombCount();
}
