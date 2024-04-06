// Fill out your copyright notice in the Description page of Project Settings.


#include "Destroyable.h"

#include "MooMooMadnessCharacter.h"
#include "Components/BoxComponent.h"
#include "DynamicMesh/ColliderMesh.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADestroyable::ADestroyable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Setup Mesh and HitBox attachment
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;

	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	HitBox->SetupAttachment(StaticMesh);

	//Setup Overlap event for HitBox
	//HitBox->OnComponentBeginOverlap.AddDynamic( this, &ADestroyable::BeginOverlap);

}

// Called when the game starts or when spawned
void ADestroyable::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADestroyable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/*void ADestroyable::BeginOverlap(UPrimitiveComponent* OverlappedComponent,
								AActor* OtherActor,
								UPrimitiveComponent* OtherComp,
								int32 OtherBodyIndex,
								bool bFromSweep,
								const FHitResult& SweepResult)
{
	if (Cast<AMooMooMadnessCharacter>(OtherActor))
	{
		DestroySelf();
	}
}*/

void ADestroyable::DestroySelf()
{
	Server_DestroySelf();
}

bool ADestroyable::Server_DestroySelf_Validate()
{
	return true;
}

void ADestroyable::Server_DestroySelf_Implementation()
{
	Multi_DestroySelf();
}

bool ADestroyable::Multi_DestroySelf_Validate()
{
	return true;
}

void ADestroyable::Multi_DestroySelf_Implementation()
{
	PlaySound();
	Destroy();
}

int ADestroyable::GetPointValue()
{
	return PointValue;
}

