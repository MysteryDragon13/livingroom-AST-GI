// This file defines the chess piece class, which is a character that can move on the chessboard.

#pragma once

#include "CoreMinimal.h"                            // Core Unreal Engine functionality
#include "GameFramework/Character.h"                // Base class for all character actors (like chess pieces)
#include "ChessPiece.generated.h"                   // Auto-generated file setup for the chess piece class

// Class representing a chess piece, such as a pawn, rook, knight, etc.
UCLASS()
class LIVINGROOM_API AChessPiece : public ACharacter
{
	GENERATED_BODY()

public:
	// Constructor that sets default values for this chess piece's properties
	AChessPiece();

protected:
	// Function called when the game starts or when the chess piece is spawned into the game world
	virtual void BeginPlay() override;

public:
	// Function called every frame to update the chess piece (e.g., for animations or movements)
	virtual void Tick(float DeltaTime) override;

	// Function called to bind input controls (like movement or actions) to the chess piece
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Function to set the material (color) of the chess piece (e.g., light or dark color)
	void SetColorMaterial(UMaterialInstance* Color);
};
