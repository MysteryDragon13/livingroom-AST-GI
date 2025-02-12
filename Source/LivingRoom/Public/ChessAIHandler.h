#pragma once  // Ensures this header file is included only once during compilation.

// Includes the CoreMinimal.h header file, which is a central part of the Unreal Engine framework.
// This header file includes essential core definitions, macros, and types used throughout Unreal Engine.
// It simplifies includes by aggregating commonly used minimal core components, such as fundamental types,
// utilities, and standard libraries, reducing the need for multiple individual header file includes.
#include "CoreMinimal.h"

#include <string>  // Provides std::string for managing text strings.
#include <vector>  // Provides std::vector for handling dynamic arrays.

// A structure to store the response from Stockfish.
// Contains details about the best move, legal moves, board state, FEN, and game status.
struct StockfishResponse {
    std::string bestMove;  // The best move recommended by Stockfish.
    std::vector<std::string> legalMoves;  // List of all legal moves according to Stockfish.
    std::vector<std::string> board;  // Board representation as a list of strings.
    std::string fen;  // FEN string representing the current board state.
    bool isCheckmate = false;  // Indicates if the position is checkmate.
    bool drawOfferable = false;  // Indicates if a draw can be offered.
};

// Class to handle interactions with the chess AI and process Stockfish responses.
class LIVINGROOM_API ChessAIHandler {
public:
    // Retrieves feedback from the chess AI based on skill level and FEN string.
    // Returns a StockfishResponse object containing details about the best move, legal moves, board state, and more.
    StockfishResponse getChessAIFeedback(const int& skill_level, const std::string& fen);

    // Prints the information contained in a StockfishResponse object.
    // The 'toPrint' parameter specifies what information to print (e.g., "Board", "BestMove", "LegalMoves", "FEN", or "All").
    void printStockfishResponse(const StockfishResponse& response, const std::string& toPrint = "All");

    // Close all open stockfish connections and handles
    void closeStockfish();
private:
    // Extracts detailed information from the raw Stockfish response.
    // Parses a vector of response lines to populate a StockfishResponse object.
    StockfishResponse extractResponse(const std::vector<std::string>& response);

    // Checks if a response line contains the best move and updates the bestMove variable.
    void checkForBestMove(const std::string& line, std::string& bestMove);

    // Checks if a response line contains legal moves and updates the legalMoves vector.
    void checkForLegalMoves(const std::string& line, std::vector<std::string>& legalMoves);

    // Checks if a response line contains a part of the board representation and updates the board vector.
    void checkForBoardPart(const std::string& line, std::vector<std::string>& board);

    // Checks if a response line contains the FEN string and updates the fen variable.
    void checkForFen(const std::string& line, std::string& fen);
};