// Copyright Epic Games, Inc. All Rights Reserved.

#include "MooMooMadnessCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Destroyable.h"
#include "Net/UnrealNetwork.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMooMooMadnessCharacter

AMooMooMadnessCharacter::AMooMooMadnessCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 60.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 750.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 250.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 550.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	bReplicates = true;
}

void AMooMooMadnessCharacter::BeginPlay()
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
		FRotator Rotation(-25.f, 180.f, 0.f);
		PlayerController->SetControlRotation(Rotation);
	}
	
}

void AMooMooMadnessCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMooMooMadnessCharacter, Invincible);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMooMooMadnessCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMooMooMadnessCharacter::Move);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AMooMooMadnessCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AMooMooMadnessCharacter::StopSprinting);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMooMooMadnessCharacter::Look);

		// HeadButt
		//EnhancedInputComponent->BindAction(HeadButtAction, ETriggerEvent::Started, this, &AMooMooMadnessCharacter::ChargeHeadButt);
		EnhancedInputComponent->BindAction(HeadButtAction, ETriggerEvent::Triggered, this, &AMooMooMadnessCharacter::ReleaseHeadButt);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMooMooMadnessCharacter::Move(const FInputActionValue& Value)
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

void AMooMooMadnessCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X*MouseSens);
		AddControllerPitchInput(LookAxisVector.Y*MouseSens);
	}
}

//Sprint
void AMooMooMadnessCharacter::Sprint()
{
	if (Controller != nullptr && GetCharacterMovement()->GetMaxSpeed() < 650.f && Stamina > 0.f)
	{
		Server_Sprint();
		//DepleteStamina();
	}
}

bool AMooMooMadnessCharacter::Server_Sprint_Validate()
{
	return true;
}

void AMooMooMadnessCharacter::Server_Sprint_Implementation()
{
	Multi_Sprint();
	
	// Start the timer
	float Delay = 0.1f;
	FTimerDelegate TimerDelegate;
	FName Attack = "Charge";
	TimerDelegate.BindUObject(this, &AMooMooMadnessCharacter::CombatTrace, 50.f, Attack);
	GetWorldTimerManager().SetTimer(LT_TimerHandle, TimerDelegate, Delay, true);
}

bool AMooMooMadnessCharacter::Multi_Sprint_Validate()
{
	return true;
}

void AMooMooMadnessCharacter::Multi_Sprint_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 650.f;
}

//Stop Sprinting
void AMooMooMadnessCharacter::StopSprinting()
{
	if (Controller != nullptr)
	{
		Server_StopSprinting();
		//PauseStamina();
	}
}

bool AMooMooMadnessCharacter::Server_StopSprinting_Validate()
{
	return true;
}

void AMooMooMadnessCharacter::Server_StopSprinting_Implementation()
{
	GetWorldTimerManager().ClearTimer(LT_TimerHandle);
	Multi_StopSprinting();
}

bool AMooMooMadnessCharacter::Multi_StopSprinting_Validate()
{
	return true;
}

void AMooMooMadnessCharacter::Multi_StopSprinting_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 250.f;
}

void AMooMooMadnessCharacter::ReleaseHeadButt()
{
	if (Controller != nullptr && !HBOnCooldown && Stamina > 0.f)
	{
		//Release head butt charge if player is currently charging
		UAnimInstance* CowMeshInstance = GetMesh()->GetAnimInstance();
		if (HeadButtAnim && !CowMeshInstance->Montage_IsActive(HeadButtChargeAnim))
		{
			//Start cooldown and call replicated function
			HBOnCooldown = true;
			StartHBCooldown();
			Server_ReleaseHeadButt();
		}
		HeadButtStrength = 0.0;
	}
}

bool AMooMooMadnessCharacter::Server_ReleaseHeadButt_Validate()
{
	return true;
}

void AMooMooMadnessCharacter::Server_ReleaseHeadButt_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Server Implementation."))

	Multi_ReleaseHeadButt();
	
	// Start the timer
	float Delay = 0.1f;
	FTimerDelegate TimerDelegate;
	FName Attack = "Headbutt";
	TimerDelegate.BindUObject(this, &AMooMooMadnessCharacter::CombatTrace, 50.f, Attack);
	GetWorldTimerManager().SetTimer(LT_TimerHandle, TimerDelegate, Delay, true);
}

bool AMooMooMadnessCharacter::Multi_ReleaseHeadButt_Validate()
{
	return true;
}

void AMooMooMadnessCharacter::Multi_ReleaseHeadButt_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast Implementation."))
	//Call bp function to stop charging and play release anim
	StopCharge();
	PlayAnimMontage(HeadButtAnim, 1.f, "ReleaseAttack");
	FVector Velocity = GetActorForwardVector()*1000.f;
	LaunchCharacter(Velocity, true, false);
}

//Detect if player hit another player
void AMooMooMadnessCharacter::CombatTrace(float Distance, FName Attack)
{
	UWorld* World = GetWorld();
	if (!World) { return; }

	//Initialize sphere trace variables
	FHitResult OutHit;
	FVector Direction = GetActorForwardVector();
	FVector Start = GetMesh()->GetBoneLocation("Neck3", EBoneSpaces::WorldSpace);
	FVector End = Start + Direction*Distance;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	
	//Call sphere trace and detect hit
	//World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams);
	FCollisionShape ColShape = FCollisionShape::MakeSphere(50.f);
	World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity,ECC_Visibility, ColShape, CollisionParams);
	
	if (Attack == "Headbutt" && !GetMesh()->GetAnimInstance()->Montage_IsActive(HeadButtAnim) && GetCharacterMovement()->GetMaxSpeed() < 650.f)
	{
		GetWorldTimerManager().ClearTimer(LT_TimerHandle);
	}

	//Check if hit is valid
	AActor* HitActor = OutHit.GetActor();
	if (!HitActor) { return; }
	
	//Check if hit is another player to apply stun
	if (HitActor->IsA(AMooMooMadnessCharacter::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("This Bish was hit!"));
		//GetWorldTimerManager().ClearTimer(LT_TimerHandle);
		AMooMooMadnessCharacter* HitPlayer = Cast<AMooMooMadnessCharacter>(OutHit.GetActor());
		if (HitPlayer && !HitPlayer->Invincible)
		{
			UpdateScore(10);
			HitPlayer->Stun(GetActorForwardVector());
			HitPlayer->UpdateScore(-10);
		}
	}
	else if (HitActor->IsA(ADestroyable::StaticClass()))
	{
		ADestroyable* HitDestroyable = Cast<ADestroyable>(OutHit.GetActor());
		if (HitDestroyable)
		{
			UpdateScore(HitDestroyable->GetPointValue());
			HitDestroyable->DestroySelf();
		}
	}
}




