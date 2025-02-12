#pragma once  // Ensures the header is included only once during compilation

#include "CoreMinimal.h"                // Includes essential core definitions, macros, and types for Unreal Engine
#include "Kismet/BlueprintFunctionLibrary.h"  // Includes functionality for creating Blueprint function libraries in Unreal Engine
#include "ChessAI.generated.h"          // Includes the generated header file for UChessAI, required for Unreal's build tools

// The UCLASS() macro marks this class as a UObject-derived class
// It makes the class available for use in Unreal Engine and enables features such as reflection
UCLASS()
class LIVINGROOM_API UChessAI : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()  // Generates necessary boilerplate code for Unreal's reflection system and other features

public:
    // This function retrieves feedback from the AI based on a specified skill level
    // It provides the updated FEN string, the best move, a list of legal moves, and whether it's checkmate
    UFUNCTION(BlueprintCallable, Category = "Chess")
    static void GetAIFeedback(const int SkillLevel, const FString CurrentFEN, FString& CorrectedFEN, FString& BestMove, TArray<FString>& LegalMoves, bool& IsCheckmate, bool& IsDrawOfferable);

    // This function parses a FEN string and populates various game state details
    // It updates the board layout, whose turn it is, castling rights, en passant target, half-move clock, and full move number
    UFUNCTION(BlueprintCallable, Category = "Chess")
    static void ParseFEN(const FString& fen, TArray<FString>& board, bool& whitesTurn, TArray<bool>& castlingRights, FString& enPassantTarget, int& halfMoveClock, int& fullMoveNumber);

    // This function extracts details from a FEN string, including legal moves and whether a draw offer is possible
    UFUNCTION(BlueprintCallable, Category = "Chess")
    static void ExtractFENDetails(const FString& fen, TArray<FString>& legalMoves, bool& drawOfferable);

    // This function checks if the first character of the provided string is uppercase
    UFUNCTION(BlueprintCallable, Category = "Chess")
    static bool IndexIsUpperCase(const FString& Input);
};