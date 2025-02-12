// Fill out your copyright notice in the Description page of Project Settings.


#include "ChessPiece.h"

// Sets default values
AChessPiece::AChessPiece()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChessPiece::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AChessPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AChessPiece::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// Function to set the material (color) of a chess piece
void AChessPiece::SetColorMaterial(UMaterialInstance* Color) {
    // Attempt to find the static mesh component by its name ("StaticMesh") on the chess piece
    UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("StaticMesh")));

    // If the static mesh component is successfully found
    if (MeshComponent) {
        // Set the first material (index 0) of the mesh component to the specified color
        MeshComponent->SetMaterial(0, Color);
    }
}

