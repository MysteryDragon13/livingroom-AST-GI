#include "ChessAI.h"        // Includes the UChessAI class declarations
#include "ChessAIHandler.h" // Includes the ChessAIHandler class for interacting with the chess AI
#include "FENParser.h"      // Includes the FENParser class for parsing FEN strings

// Global instances of the ChessAIHandler and FENParser classes
ChessAIHandler ChessAIHandlerInstance;
FENParser FENParserInstance;

// Converts an FString to a std::string
std::string ConvertToStdString(FString ToConvert) {
    return std::string(TCHAR_TO_UTF8(*ToConvert));
}

// Converts a std::string to an FString
FString ConvertToFString(std::string ToConvert) {
    return FString(ToConvert.c_str());
}

// Checks if the first character of the input FString is uppercase
bool UChessAI::IndexIsUpperCase(const FString& Input) {
    return FChar::IsUpper(Input[0]);
}

// Retrieves feedback from the chess AI based on the skill level and current FEN
void UChessAI::GetAIFeedback(const int SkillLevel, const FString CurrentFEN, FString& CorrectedFEN, FString& BestMove, TArray<FString>& LegalMoves, bool& IsCheckmate, bool& IsDrawOfferable) {
    std::string CurrentFENString = ConvertToStdString(CurrentFEN);

    // Get the response from the ChessAIHandler
    StockfishResponse Response = ChessAIHandlerInstance.getChessAIFeedback(SkillLevel, CurrentFENString);

    // Convert Stockfish response to FString and populate the output parameters
    CorrectedFEN = ConvertToFString(Response.fen);
    BestMove = ConvertToFString(Response.bestMove);
    IsCheckmate = Response.isCheckmate;

    // Convert legal moves to FString and populate the LegalMoves array
    for (const std::string& Move : Response.legalMoves) {
        LegalMoves.Add(ConvertToFString(Move));
    }

    // After correcting the FEN with the AI, extract further details like draw offerability
    // And adjust the Castling, EnPassant and Pawn Promotion strings to easily detect them
    ExtractFENDetails(CorrectedFEN, LegalMoves, IsDrawOfferable);
}

// Parses a FEN string and populates board details and game state information
void UChessAI::ParseFEN(const FString& FEN, TArray<FString>& Board, bool& WhitesTurn, TArray<bool>& CastlingRights, FString& EnPassantTarget, int& HalfMoveClock, int& FullMoveNumber) {
    std::string FENString = ConvertToStdString(FEN);
    std::vector<std::string> BoardStringVector;
    std::vector<bool> CastlingRightsVector;
    std::string EnPassantString = ConvertToStdString(EnPassantTarget);

    // Convert board and castling rights data
    for (const FString& Square : Board) {
        BoardStringVector.push_back(ConvertToStdString(Square));
    }

    for (bool CastlingRight : CastlingRights) {
        CastlingRightsVector.push_back(CastlingRight);
    }

    // Parse the FEN string
    FENParserInstance.parseFEN(FENString, BoardStringVector, WhitesTurn, CastlingRightsVector, EnPassantString, HalfMoveClock, FullMoveNumber);

    // Convert and populate board and castling rights data
    Board.Empty();
    for (const std::string& Square : BoardStringVector) {
        Board.Add(ConvertToFString(Square));
    }

    CastlingRights.Empty();
    for (bool CastlingRight : CastlingRightsVector) {
        CastlingRights.Add(CastlingRight);
    }

    EnPassantTarget = ConvertToFString(EnPassantString);
}

// Extracts legal moves and draw offerable status from a FEN string
void UChessAI::ExtractFENDetails(const FString& FEN, TArray<FString>& LegalMoves, bool& DrawOfferable) {
    std::string FENString = ConvertToStdString(FEN);
    std::vector<std::string> LegalMovesString;

    // Convert legal moves data
    for (const FString& Move : LegalMoves) {
        LegalMovesString.push_back(ConvertToStdString(Move));
    }

    // Extract details from the FEN string
    FENParserInstance.extractFENDetails(FENString, LegalMovesString, DrawOfferable);

    // Convert and populate legal moves data
    LegalMoves.Empty();
    for (const std::string& Move : LegalMovesString) {
        LegalMoves.Add(ConvertToFString(Move));
    }
}