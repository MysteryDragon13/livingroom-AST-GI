#pragma once  // Ensures this header file is included only once during compilation.

// Includes the CoreMinimal.h header file, which is a central part of the Unreal Engine framework.
// This header file includes essential core definitions, macros, and types used throughout Unreal Engine.
// It simplifies includes by aggregating commonly used minimal core components, such as fundamental types,
// utilities, and standard libraries, reducing the need for multiple individual header file includes.
#include "CoreMinimal.h"

#include <string>   // Provides std::string for handling text strings.
#include <vector>   // Provides std::vector for dynamic arrays, like lists of board rows or moves.

class LIVINGROOM_API FENParser {
public:
    // Parses a FEN (Forsyth-Edwards Notation) string to set up the board and game details.
    // Updates the board, turn indicator, castling rights, en passant target, half-move clock, and full-move number.
    void parseFEN(const std::string& fen, std::vector<std::string>& board, bool& whitesTurn,
        std::vector<bool>& castlingRights, std::string& enPassantTarget,
        int& halfMoveClock, int& fullMoveNumber);

    // Extracts legal moves and checks if a draw can be offered based on the FEN string.
    // Fills the legal moves and draw offer status based on the current FEN.
    void extractFENDetails(const std::string& fen, std::vector<std::string>& legalMoves,
        bool& drawOfferable);

private:
    // Determines which castling moves are possible from the given FEN and updates the list of legal moves.
    // Checks the board state and whether castling is allowed.
    void detectPossibleCastlingMoves(const std::string& fen, std::vector<std::string>& legalMoves,
        std::vector<std::string> board, bool whitesTurn, std::vector<bool> castlingRights);

    // Adds castling moves to the list of legal moves if they are allowed.
    // Checks if the castling move is valid and if the king is not under attack.
    void addCastlingOptionIfPossible(std::vector<std::string>& legalMoves,
        std::vector<std::string>& negativeLegalMoves, const std::vector<std::string>& board,
        bool whitesTurn, bool castlingAllowed, bool firstCastlingCheck, const std::string& castlingMove);

    // Detects and adds en passant moves to the list of legal moves if applicable.
    // Checks if the en passant move is possible and updates the list of legal moves accordingly.
    void detectEnPassant(std::vector<std::string>& legalMoves,
        const std::vector<std::string>& board,
        const std::string& enPassantTarget, bool whitesTurn);
};
