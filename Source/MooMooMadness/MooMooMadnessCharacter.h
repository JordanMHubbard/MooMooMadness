// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MooMooMadnessCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UAnimMontage;


DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AMooMooMadnessCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** HeadButt Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeadButtAction;

public:
	AMooMooMadnessCharacter();

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for sprint input */
	void Sprint();

	UFUNCTION (BlueprintCallable)
	void StopSprinting();

	UFUNCTION (Server, Reliable, WithValidation)
	void Server_Sprint();
	bool Server_Sprint_Validate();
	void Server_Sprint_Implementation();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multi_Sprint();
	bool Multi_Sprint_Validate();
	void Multi_Sprint_Implementation();

	UFUNCTION (Server, Reliable, WithValidation)
	void Server_StopSprinting();
	bool Server_StopSprinting_Validate();
	void Server_StopSprinting_Implementation();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multi_StopSprinting();
	bool Multi_StopSprinting_Validate();
	void Multi_StopSprinting_Implementation();

	/** Called for HeadButt input */
	UFUNCTION (BlueprintImplementableEvent)
	void ChargeHeadButt();

	void ReleaseHeadButt();

	UFUNCTION (BlueprintImplementableEvent)
	void StopCharge();

	UFUNCTION (Server, Reliable, WithValidation)
	void Server_ReleaseHeadButt();
	bool Server_ReleaseHeadButt_Validate();
	void Server_ReleaseHeadButt_Implementation();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multi_ReleaseHeadButt();
	bool Multi_ReleaseHeadButt_Validate();
	void Multi_ReleaseHeadButt_Implementation();

	//Determines how powerful the head butt will be
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HeadButtStrength;

	//Head Butt Cooldown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool HBOnCooldown;

	//Refresh Head butt cooldown
	UFUNCTION (BlueprintImplementableEvent)
	void StartHBCooldown();

	//Charging head butt animation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HeadButtChargeAnim;

	//Head butt animation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HeadButtAnim;

	//Kick animation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* KickAnim;

	//Line Trace for combat attacks
	void CombatLineTrace(FName StartBone, FName EndBone, float Distance, FName Attack);
	FTimerHandle LT_TimerHandle;

	UFUNCTION (BlueprintImplementableEvent)
	void Stun(FVector Direction);
	
	//Stamina
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Stamina = 1.f;

	UFUNCTION (BlueprintImplementableEvent)
	void DepleteStamina();
	
	UFUNCTION (BlueprintImplementableEvent)
	void PauseStamina();

	//Score
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 Score = 0;

	UFUNCTION (BlueprintImplementableEvent)
	void UpdateScore(int32 Points);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float MouseSens = 0.6f;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

