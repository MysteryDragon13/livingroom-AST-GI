// This file manages the chess tile, which holds a chess piece and handles game logic on the tile.

#pragma once

#include "CoreMinimal.h"                            // Core Unreal Engine functionality
#include "GameFramework/Actor.h"                    // Base class for all in-game actors (like chess tiles)
#include "ChessPiece.h"                             // ChessPiece class, representing pieces that may be on the tile
#include "ChessTile.generated.h"                    // Auto-generated file setup for the chess tile class

// Class representing a single tile on the chessboard, where a chess piece can be placed
UCLASS()
class LIVINGROOM_API AChessTile : public AActor
{
	GENERATED_BODY()

public:
	// Constructor to set default values for this tile's properties
	AChessTile();

	// Holds the chess piece that is currently on this tile (nullptr if empty)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Chess")
	AChessPiece* PresentChessPiece;

protected:
	// Function called when the game starts or the tile is spawned into the world
	virtual void BeginPlay() override;

public:
	// Function called every frame to update the tile, if necessary
	virtual void Tick(float DeltaTime) override;
};
