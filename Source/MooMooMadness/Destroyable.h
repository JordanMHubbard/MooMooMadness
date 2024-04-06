// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Destroyable.generated.h"


UCLASS()
class MOOMOOMADNESS_API ADestroyable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADestroyable();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* HitBox;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score System", meta = (AllowPrivateAccess = "true"))
	int32 PointValue = 0;
	
	// Sound effect 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	class USoundWave* DestructionSound;

	UFUNCTION(BlueprintImplementableEvent)
	void PlaySound();
	
	/*UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, 
					  AActor* OtherActor, 
					  UPrimitiveComponent* OtherComp, 
					  int32 OtherBodyIndex, 
					  bool bFromSweep, 
					  const FHitResult &SweepResult );*/
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void DestroySelf();

	UFUNCTION (Server, Reliable, WithValidation)
	void Server_DestroySelf();
	bool Server_DestroySelf_Validate();
	void Server_DestroySelf_Implementation();
	
	UFUNCTION (NetMulticast, Reliable, WithValidation)
	void Multi_DestroySelf();
	bool Multi_DestroySelf_Validate();
	void Multi_DestroySelf_Implementation();

	int GetPointValue();

};
