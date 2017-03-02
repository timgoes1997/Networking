// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "Components/TextRenderComponent.h"
#include "GameFramework/Character.h"
#include "UnrealCNetworkCharacter.generated.h"

UCLASS(config=Game)
class AUnrealCNetworkCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AUnrealCNetworkCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface


	//Tutorial methods
protected:

	/** The health of the character */
	UPROPERTY(VisibleAnywhere, Transient, ReplicatedUsing = OnRep_Health, Category = Stats)
		float Health;

	/** The max health of the character */
	UPROPERTY(EditAnywhere, Category = Stats)
		float MaxHealth = 100.f;

	/** The number of bombs that the character carries */
	UPROPERTY(VisibleAnyWhere, Transient, ReplicatedUsing = OnRep_BombCount, Category = Stats)
		int32 BombCount;

	/** The max number of bombs that a character can have */
	UPROPERTY(EditAnyWhere, Category = Stats)
		int32 MaxBombCount = 3;

	/** Text render component */
	UPROPERTY(VisibleAnywhere)
		UTextRenderComponent* CharText;

private:

	/** Called when the Health variable gets updated */
	UFUNCTION()
		void OnRep_Health();

	/** Called when the BombCount variable gets updated */
	UFUNCTION()
		void OnRep_BombCount();

	/** Initializes health */
	void InitHealth();

	/** Initializes the bomb count */
	void InitBombCount();

	/** Updates the character's text to match with the updated stats */
	void UpdateCharText();

	//Second part

	/**
	* TakeDamage Server version. Call this instead of TakeDamage when you're a client.
	* You don't have to generate an implementation. It will automatically call the ServerTakeDamage_Implementation function.
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerTakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	/** Contains the actual implementation of the ServerTakeDamage function */
	void ServerTakeDamage_Implementation(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	/** Validates the client. If the result is false the client will be disconnected */
	bool ServerTakeDamage_Validate(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	//Bomb related functions

	/** Will try to spawn a bomb */
	void AttempToSpawnBomb();

	/** Returns true if we can throw a bomb */
	bool HasBombs() { return BombCount > 0; }

	/**
	* Spawns a bomb. Call this function when you're authorized to.
	* In case you're not authorized, use the ServerSpawnBomb function.
	*/
	void SpawnBomb();

	/**
	* SpawnBomb Server version. Call this instead of SpawnBomb when you're a client.
	* You don't have to generate an implementation for this. It will automatically call the ServerSpawnBomb_Implementation function.
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerSpawnBomb();

	/** Contains the actual implementation of the ServerSpawnBomb function */
	void ServerSpawnBomb_Implementation();

	/**Validates the client. If the result is false the client will be disconnected */
	bool ServerSpawnBomb_Validate();

public:

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	//End tutorial methods

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

