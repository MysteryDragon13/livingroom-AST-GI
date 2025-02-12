// This file manages the chessboard logic, including tile construction and chess piece spawning.

#pragma once

#include "CoreMinimal.h"                                                // Core Unreal Engine functionality
#include "GameFramework/Actor.h"                                         // Base class for all in-game actors (like the chessboard)
#include "Components/HierarchicalInstancedStaticMeshComponent.h"         // Efficient handling of multiple static mesh instances (for chess tiles)
#include "ChessTile.h"                                                   // ChessTile class, representing individual tiles on the chessboard
#include "ChessPiece.h"                                                  // ChessPiece class, representing individual chess pieces
#include "ChessAI.h"                                                     // ChessAI class, handling chess-related logic and AI
#include "ChessBoard.generated.h"                                        // Auto-generated file setup for the chessboard class

// Class that represents the chessboard and handles chess tile/piece management
UCLASS()
class LIVINGROOM_API AChessBoard : public AActor
{
	GENERATED_BODY()

public:
	// Constructor that sets default values for this actor's properties
	AChessBoard();

	// Map to store the chess piece types (like King, Pawn) mapped to FEN characters (e.g., "k" for King)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Chess")
	TMap<FString, TSubclassOf<AChessPiece>> ChessPieceClasses;

	// Map that associates field names (like "a1") to chess tiles
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Chess")
	TMap<FString, AChessTile*> Fields;

	// Map to store the locations of fields (tiles) on the board
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Chess")
	TMap<FString, FVector> FieldLocations;

	// Material for light-colored chess pieces
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Chess")
	UMaterialInstance* LightColor;

	// Material for dark-colored chess pieces
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Chess")
	UMaterialInstance* DarkColor;

	// Z-axis offset for tiles, used to adjust their height
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Chess")
	float ZOffset = -20.0f;

	// Scaling factor for each chess tile
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Chess")
	FVector TileScale = { 1.05f, 1.05f, 1.05f };

	// Array of field letters (like "a", "b", etc.) for easier board navigation
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Chess")
	TArray<FString> FieldLetters = { "a", "b", "c", "d", "e", "f", "g", "h" };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Calculates the position of each chess tile based on its letter and number index
	FVector CalculateFieldLocation(int LetterIndex, int NumberIndex);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Function to construct the checkboard pattern with light and dark tiles
	UFUNCTION(BlueprintCallable, Category = "Chess")
	void ConstructCheckboardPattern(UHierarchicalInstancedStaticMeshComponent* LightTiles, UHierarchicalInstancedStaticMeshComponent* DarkTiles);

	// Converts a board position index (0-63) into a standard chess field name (like "a1", "h8")
	UFUNCTION(BlueprintCallable, Category = "Chess")
	FString ConvertIndexToFieldName(int BoardPosition);

	// Spawns a chess piece based on its FEN character (like 'K' for King) at the corresponding field location
	UFUNCTION(BlueprintCallable, Category = "Chess")
	AChessPiece* SpawnChessPieceBasedOnFENChar(const FString FENChar, FVector FieldLocation);

	// Spawns chess pieces on the board according to the provided FEN string
	UFUNCTION(BlueprintCallable, Category = "Chess")
	AChessPiece* SpawnChessPieceOnBoard(const FString FENChar, const int Index);
};