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


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMooMooMadnessCharacter

AMooMooMadnessCharacter::AMooMooMadnessCharacter()
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
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 200.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
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
	}
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
		EnhancedInputComponent->BindAction(HeadButtAction, ETriggerEvent::Started, this, &AMooMooMadnessCharacter::ChargeHeadButt);
		EnhancedInputComponent->BindAction(HeadButtAction, ETriggerEvent::Completed, this, &AMooMooMadnessCharacter::ReleaseHeadButt);
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

void AMooMooMadnessCharacter::Sprint()
{
	if (Controller != nullptr)
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
}

void AMooMooMadnessCharacter::StopSprinting()
{
	if (Controller != nullptr)
	{
		GetCharacterMovement()->MaxWalkSpeed = 200.f;
	}
}

void AMooMooMadnessCharacter::Look(const FInputActionValue& Value)
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

void AMooMooMadnessCharacter::ReleaseHeadButt()
{
	if (Controller != nullptr && !HBOnCooldown)
	{
		StopCharge();
		UAnimInstance* CowMeshInstance = GetMesh()->GetAnimInstance();
		if (HeadButtAnim && CowMeshInstance->Montage_IsActive(HeadButtChargeAnim))
		{
			//PlayAnimMontage(HeadButtAnim, 1.f, "ReleaseAttack");
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
	
	float Delay = 0.1f;
	// Start the timer
	FTimerDelegate TimerDelegate;
	FName StartBone = "Head";
	FName EndBone = "Head_end";
	TimerDelegate.BindUObject(this, &AMooMooMadnessCharacter::CombatLineTrace, StartBone, EndBone);
	GetWorldTimerManager().SetTimer(LT_TimerHandle, TimerDelegate, Delay, true);
}

bool AMooMooMadnessCharacter::Multi_ReleaseHeadButt_Validate()
{
	return true;
}

void AMooMooMadnessCharacter::Multi_ReleaseHeadButt_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast Implementation."))

	StopCharge();
	PlayAnimMontage(HeadButtAnim, 1.f, "ReleaseAttack");
}

void AMooMooMadnessCharacter::CombatLineTrace(FName StartBone, FName EndBone)
{
	UWorld* World = GetWorld();
	if (!World) { return; }

	FHitResult OutHit;
	FVector Start = GetMesh()->GetBoneLocation(StartBone, EBoneSpaces::WorldSpace);
	FVector Direction = GetMesh()->GetBoneLocation(EndBone, EBoneSpaces::WorldSpace) - Start;
	Direction.Normalize();
	FVector End = Start + Direction*200;
	DrawDebugLine(World, Start, End, FColor::Purple,false, 5.f, 0, 3.f);

	World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Pawn);
	AMooMooMadnessCharacter* HitPlayer = Cast<AMooMooMadnessCharacter>(OutHit.GetActor());
	if (HitPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("This Bish was hit!"));
		GetWorldTimerManager().ClearTimer(LT_TimerHandle);
	}
	else if (!GetMesh()->GetAnimInstance()->Montage_IsActive(HeadButtAnim))
	{
		GetWorldTimerManager().ClearTimer(LT_TimerHandle);
	}
	
}