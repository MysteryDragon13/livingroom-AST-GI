#include "ChessAIHandler.h"  // Includes the ChessAIHandler class, which manages interactions with the chess AI and processes Stockfish responses.
#include "FENParser.h"       // Includes the FENParser class, used to parse and extract information from FEN (Forsyth-Edwards Notation) strings.
#include "Stockfish.h"       // Includes the Stockfish class, which handles communication with the Stockfish chess engine.
#include <string>            // Provides std::string for managing text strings, like moves and FEN strings.
#include <vector>            // Provides std::vector for dynamic arrays, such as lists of moves and board states.

// Color codes for terminal output to differentiate parts of the chess information.
const std::string GREEN = "\033[32m";   // Color code for green text.
const std::string YELLOW = "\033[33m";  // Color code for yellow text.
const std::string BLUE = "\033[34m";    // Color code for blue text.
const std::string WHITE = "\033[0m";    // Reset color code to default.

bool whitesTurn = false; // Bool to determine if white is the current playing color

// Creates an instance of the Stockfish class to interact with the chess engine.
Stockfish& stockfish = Stockfish::getInstance();

// Close all open stockfish connections and handles
void ChessAIHandler::closeStockfish() {
    stockfish.closeStockfish();
}

// Gets feedback from the chess AI based on the provided skill level and FEN string.
// Returns the response containing best move, legal moves, board state, and other details.
StockfishResponse ChessAIHandler::getChessAIFeedback(const int& skillLevel, const std::string& fen) {
    std::vector<std::string> response = stockfish.requestStockfish(skillLevel, fen); // Requests feedback from Stockfish.
    return extractResponse(response); // Extracts and returns the relevant information from the response.
}

// Extracts relevant details from the Stockfish response and updates the StockfishResponse object.
// Parses the response lines to find the best move, legal moves, board state, and FEN.
StockfishResponse ChessAIHandler::extractResponse(const std::vector<std::string>& response) {
    FENParser fenParser;  // Creates an instance of FENParser to handle FEN strings.
    StockfishResponse result;  // Holds the extracted information from the response.
    bool drawOfferable = false;  // Flag to indicate if a draw offer is possible.

    // Process each line of the response to extract information
    for (const std::string& line : response) {
        checkForBestMove(line, result.bestMove);  // Checks if the line contains the best move.
        checkForLegalMoves(line, result.legalMoves);  // Checks if the line contains legal moves.
        checkForBoardPart(line, result.board);  // Checks if the line represents a part of the board.
        checkForFen(line, result.fen);  // Checks if the line contains the FEN string.
    }

    result.isCheckmate = result.legalMoves.empty();  // Determine if it's checkmate based on the absence of legal moves.
    fenParser.extractFENDetails(result.fen, result.legalMoves, drawOfferable);  // Extract additional FEN details.

    //Remove \r (Carriage Return) if it is contained - seems like Stockfish was programmed on a Mac
    if (!result.fen.empty() && result.fen[result.fen.size() - 1] == '\r') {
        result.fen.erase(result.fen.size() - 1);
    }

    return result;  // Return the populated StockfishResponse object.
}

// Checks if a line contains the FEN string and updates the fen variable.
// Assumes the line starts with "Fen: " and extracts the FEN part.
void ChessAIHandler::checkForFen(const std::string& line, std::string& fen) {
    if (line.find("Fen: ") != std::string::npos) {
        fen = line.substr(5);  // Extracts the FEN string from the line.
        // Checks if white is the current color. Since the fen string is always located behind the moves
        // in the response, we have to switch the color logic here!
        whitesTurn = fen.find("w") != std::string::npos ? false : true;
    }
}

// Checks if a line from the response contains the best move and updates the bestMove variable.
// Assumes the format "bestmove <move>" and extracts the move part.
void ChessAIHandler::checkForBestMove(const std::string& line, std::string& bestMove) {
    if (line.find("bestmove") != std::string::npos) {
        if (line[13] == ' ') {
            bestMove = line.substr(9, 4);  // Extracts the normal move from the string.
        }
        else {
            bestMove = line.substr(9, 5);  // Extracts the promotion move from the string.
        }
    }
}

// Inline function to check if a line contains a pawn promotion move.
// Returns true if the line specifies a pawn promotion (e.g., "e7e8q").
static bool checkForPromotionPiece(const std::string& line) {
    if (line.length() >= 5) {
        std::vector<char> promotionPieces = { 'q', 'r', 'b', 'n' };  // List of valid promotion pieces.
        for (char promotionPiece : promotionPieces) {
            if (promotionPiece == line[4]) {
                return true;  // Promotion piece found.
            }
        }
    }
    return false;  // No promotion piece found.
}

// Checks if a line contains legal moves and updates the legalMoves vector.
// Also checks for pawn promotion and formats the move accordingly.
void ChessAIHandler::checkForLegalMoves(const std::string& line, std::vector<std::string>& legalMoves) {
    if (line.find(": 1") != std::string::npos && line.find("Node") == std::string::npos) {
        std::string move = line.substr(0, 4);  // Extracts the move from the line.

        if (checkForPromotionPiece(line)) {
            char promotionPiece = line[4];  // Gets the promotion piece.
            promotionPiece = whitesTurn ? _toupper(promotionPiece) : promotionPiece; // Set the promotionPiece based on current color
            legalMoves.push_back(move + "->" + promotionPiece);  // Adds promotion move to legal moves.
        }
        else if (move != "Key:" && move != "Fen:") {
            legalMoves.push_back(move);  // Adds the move to legal moves.
        }
    }
}

// Checks if a line contains part of the board representation and updates the board vector.
// Adds lines that represent the board state.
void ChessAIHandler::checkForBoardPart(const std::string& line, std::vector<std::string>& board) {
    if ((line.find(" +---+") != std::string::npos) || (line.find(" | ") != std::string::npos) ||
        (line.find("   a") != std::string::npos)) {
        board.push_back(line);  // Adds the line to the board representation.
    }
}

// Prints the Stockfish response to the console based on specified categories.
// Optionally prints the board, best move, legal moves, and FEN string.
void ChessAIHandler::printStockfishResponse(const StockfishResponse& response, const std::string& toPrint) {
    if ((toPrint.find("Board") != std::string::npos) || toPrint == "All") {
        std::cout << "Board Representation:\n";
        for (const std::string& line : response.board) {
            std::cout << line << "\n";  // Prints each line of the board representation.
        }
    }

    if ((toPrint.find("BestMove") != std::string::npos) || toPrint == "All") {
        std::cout << "Best Move: " << YELLOW << response.bestMove << WHITE << "\n";  // Prints the best move in yellow.
    }

    if ((toPrint.find("LegalMoves") != std::string::npos) || toPrint == "All") {
        std::string legalMoves;
        for (const std::string& move : response.legalMoves) {
            legalMoves.append(move + " ");  // Concatenates legal moves into a single string.
        }
        std::cout << "Legal Moves: " << GREEN << legalMoves << WHITE << "\n";  // Prints legal moves in green.
    }

    if ((toPrint.find("FEN") != std::string::npos) || toPrint == "All") {
        std::cout << "FEN: " << BLUE << response.fen << WHITE << "\n";  // Prints the FEN string in blue.
    }
}