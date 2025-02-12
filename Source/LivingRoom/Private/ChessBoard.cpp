// Fill out your copyright notice in the Description page of Project Settings.


#include "ChessBoard.h"

// Sets default values
AChessBoard::AChessBoard()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AChessBoard::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AChessBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Function to calculate the location of a tile on the chessboard based on its file (letter) and rank (number)
FVector AChessBoard::CalculateFieldLocation(int LetterIndex, int NumberIndex) {
	const FVector Offset = { -450.0f, -350.0f, 0.0f };  // Starting offset for the first tile on the board
	const float Stepsize = 100.0f;  // Distance between adjacent tiles

	// Calculate the X and Y coordinates for the tile, applying the offset and step size, then scale them
	float FieldLocationX = (Offset.X + (NumberIndex * Stepsize)) * TileScale.X;
	float FieldLocationY = (Offset.Y + (LetterIndex * Stepsize)) * TileScale.Y;

	// Return the final 3D location for the tile, including the Z offset to slightly lower the tile
	return { FieldLocationX, FieldLocationY, 0.0f };
}

// Function to construct all fields (tiles) on the chessboard, placing them in LightTiles and DarkTiles meshes
void AChessBoard::ConstructCheckboardPattern(UHierarchicalInstancedStaticMeshComponent* LightTiles, UHierarchicalInstancedStaticMeshComponent* DarkTiles) {
	FieldLocations.Reset();  // Clear any previous tile locations

	// Loop through the chessboard files (A to H)
	for (int FieldLetterIndex = 0; FieldLetterIndex < 8; FieldLetterIndex++) {
		FString FieldLetter = FieldLetters[FieldLetterIndex];  // Get the current file letter (e.g., "A", "B", etc.)

		// Loop through the ranks (1 to 8, reversed since we typically place rows from 8 to 1)
		for (int Number = 8; Number > 0; Number--) {
			// Create a unique name for each tile (e.g., "A8", "B7")
			FString FieldName = FieldLetter + FString::FromInt(Number);

			// Calculate the 3D location for the current tile
			FVector FieldLocation = CalculateFieldLocation(FieldLetterIndex, Number);

			// Add this field's name and location to the map of field locations
			FieldLocations.Add(FieldName, FieldLocation);

			// Determine the location for the tile, adjusting it slightly along the Z axis
			FVector TileLocation = { FieldLocation.X, FieldLocation.Y, FieldLocation.Z };
			FRotator DummyRotator = { 0.0f, 0.0f, 0.0f };  // No rotation for the tiles

			// Create a transform object for placing the tile, including position and scale
			FTransform TileTransform = { DummyRotator, TileLocation, TileScale };

			// Alternating between light and dark tiles:
			// If the calculated value is even, add a light tile; otherwise, add a dark tile
			if ((((FieldLetterIndex * 7) + Number) % 2) == 0) {
				LightTiles->AddInstance(TileTransform);  // Add to light-colored tiles mesh
			}
			else {
				DarkTiles->AddInstance(TileTransform);  // Add to dark-colored tiles mesh
			}
		}
	}
}

// Converts the board index (0-63) into a field name (e.g., "A1", "B2", etc.)
FString AChessBoard::ConvertIndexToFieldName(int Index) {
    FString FieldLetter = FieldLetters[Index % 8];  // Get letter based on modulus of 8 (columns)
    int FieldNumber = ((63 - Index) / 8) + 1;  // Convert index into row number (1-8)
    return FieldLetter + FString::FromInt(FieldNumber);  // Combine letter and number to form field name
}

// Spawns a chess piece on the chess tile based on the FEN character (e.g., "P" for white pawn, "p" for black pawn).
AChessPiece* AChessBoard::SpawnChessPieceBasedOnFENChar(const FString FENChar, FVector FieldLocation) {
    UMaterialInstance* Color; // Material color for the chess piece (white or black).
    FRotator LookDirection = { 0.0f, 0.0f, 0.0f }; // Default rotation for white pieces.
    FVector Offset = { 0.0f, 0.0f, 200.0f }; // Offset to ensure the piece spawns slightly above the tile.

    // Attempt to find the chess piece class corresponding to the FEN character (e.g., 'P' -> Pawn).
    TSubclassOf<AChessPiece>* ChessPieceClassPointer = ChessPieceClasses.Find(FENChar);

    // If a valid chess piece class was found for this FEN character.
    if (ChessPieceClassPointer && *ChessPieceClassPointer) {
        TSubclassOf<AChessPiece> ChessPieceClass = *ChessPieceClassPointer;  // Dereference the pointer to get the actual class.
        //UE_LOG(LogTemp, Warning, TEXT("Found chess piece class for FENChar: %s"), *FENChar);

        // Determine if the FEN character represents a white or black piece.
        if (UChessAI::IndexIsUpperCase(FENChar)) {
            Color = LightColor; // White pieces use light material color.
            //UE_LOG(LogTemp, Warning, TEXT("FENChar: %s is upper case. Assigning light color."), *FENChar);
        }
        else {
            LookDirection = { 0.0f, 180.0f, 0.0f }; // Black pieces should face the opposite direction.
            Color = DarkColor; // Black pieces use dark material color.
            //UE_LOG(LogTemp, Warning, TEXT("FENChar: %s is lower case. Assigning dark color."), *FENChar);
        }

        // Calculate the spawn location relative to the chess tile's position.
        FVector SpawnLocation = FieldLocation + Offset;
        //UE_LOG(LogTemp, Warning, TEXT("Spawn location for FENChar: %s is: %s"), *FENChar, *SpawnLocation.ToString());

        // Create the transform for spawning the chess piece, including rotation, position, and scale.
        FTransform SpawnTransform(LookDirection, SpawnLocation, FVector(1.0f));

        FActorSpawnParameters SpawnParameter;
        SpawnParameter.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // Spawn the chess piece in the world using the specified class and transform.
        AChessPiece* SpawnedChessPiece = GetWorld()->SpawnActor<AChessPiece>(ChessPieceClass, SpawnTransform, SpawnParameter);
        //UE_LOG(LogTemp, Warning, TEXT("Spawned chess piece: %s for FENChar: %s"), SpawnedChessPiece ? *SpawnedChessPiece->GetName() : TEXT("None"), *FENChar);

        // If the chess piece was successfully spawned, set its color and link it to the tile.
        if (SpawnedChessPiece) {
            SpawnedChessPiece->SetColorMaterial(Color); // Set the material (color) of the chess piece.
            //UE_LOG(LogTemp, Warning, TEXT("Assigned color to chess piece and linked it to the tile."));
        }

        // Return the spawned chess piece to the caller.
        return SpawnedChessPiece;
    }

    // Log a warning if no chess piece class was found.
    //UE_LOG(LogTemp, Error, TEXT("No valid chess piece class found for FENChar: %s"), *FENChar);
    return NULL;
}

// Spawns chess pieces on the board according to the provided FEN or similar board representation
AChessPiece* AChessBoard::SpawnChessPieceOnBoard(const FString FENChar, const int Index) {
    // Convert the board position index into a chessboard field name (like "A1", "B2", etc.).
    FString FieldName = ConvertIndexToFieldName(Index);
    //UE_LOG(LogTemp, Warning, TEXT("Converted index %d to field name: %s"), Index, *FieldName);

    // Check if the field exists in the FieldLocations map.
    FVector* FieldLocationPointer = FieldLocations.Find(FieldName);
    if (FieldLocationPointer) {  // Ensure we have a valid location.
        FVector FieldLocation = *FieldLocationPointer;
        //UE_LOG(LogTemp, Warning, TEXT("Found location: %s for field name: %s"), *FieldLocation.ToString(), *FieldName);

        // Spawn the chess piece on the appropriate tile.
        AChessPiece* SpawnedChessPiece = SpawnChessPieceBasedOnFENChar(FENChar, FieldLocation);
        return SpawnedChessPiece;
    }
    else {
        // Log a warning if no chess tile was found for the field name.
        //UE_LOG(LogTemp, Error, TEXT("No location found for field name: %s"), *FieldName);
        return NULL;
    }
}