#include "FenParser.h"     // Includes the FENParser class, which has methods to parse and handle chess FEN strings.
#include "ChessAIHandler.h" // Includes the ChessAIHandler class for managing AI logic and interacting with Stockfish.
#include <cctype>          // Includes functions like std::isdigit to check if a character is a digit.
#include <iostream>        // Includes standard input/output functions, like std::cout for debugging.
#include <sstream>         // Includes std::istringstream for splitting strings, such as the FEN string.
#include <string>          // Includes the std::string class for handling text strings.
#include <vector>          // Includes std::vector for dynamic arrays, useful for storing data like board rows or legal moves.

using namespace std;

const std::string COLUMN_LETTERS = "abcdefgh"; // String representing column letters used in chess notation.

//----------------------------------------- Simple Parser ----------------------------------------------------

// Forward declaration of helper function
std::vector<bool> getCastlingAvailability(const std::string& castlingRights);

//Read the input string and split it into parts based on the delimiter
vector<string> splitStringIntoParts(const std::string& input, const char delimiter) {
    istringstream stringStream(input); // Create a stream from the input string for easy parsing.
    vector<string> parts; // This will store the resulting parts of the string.
    string part; // Temporary variable to hold each part of the string.

    // Read each part of the string up to the delimiter and store it in 'part'.
    while (getline(stringStream, part, delimiter))
    {
        parts.push_back(part); // Add the current part to the vector of parts.
    }

    return parts; // Return the list of parts.
}

// Parses a FEN string and extracts chess board state and other details.
void FENParser::parseFEN(const std::string& fen, std::vector<std::string>& board, bool& whitesTurn,
    std::vector<bool>& castlingRights, std::string& enPassantTarget,
    int& halfMoveClock, int& fullMoveNumber) {
    std::string fenString = fen; // Copy of the FEN string
    std::istringstream fenStream(fenString); // Stream to read parts of the FEN string
    std::vector<std::string> parts; // Stores the parts of the FEN string
    std::string part;

    // Split the FEN string into 6 parts using space as the delimiter.
    parts = splitStringIntoParts(fen, ' ');

    // Check if the FEN string has exactly 6 parts (board, turn, castling rights, en passant, half-move clock, full-move number).
    if (parts.size() != 6) {
        // If the input FEN is invalid, use a default FEN for the starting chess position.
        fenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        parts = splitStringIntoParts(fenString, ' ');
    }

    // The first part of the FEN represents the board, with rows separated by '/'.
    vector<string> rows = splitStringIntoParts(parts[0], '/');

    // Convert FEN rows to board representation
    for (const string& row : rows)
    {
        for (char currentFENChar : row)
        {
            if (isdigit(currentFENChar))
            {
                // If the character is a digit, it represents empty squares
                int emptySquares = currentFENChar - '0'; // Convert character to number
                for (int j = 0; j < emptySquares; ++j)
                {
                    board.push_back("."); // Add empty square to the board
                }
            }
            else
            {
                board.push_back(string(1, currentFENChar)); // Add piece to the board
            }
        }
    }

    // Extract other FEN details
    whitesTurn = (parts[1] == "w"); // Determine whose turn it is
    castlingRights = getCastlingAvailability(parts[2]); // Get castling rights
    enPassantTarget = parts[3]; // Get en passant target
    halfMoveClock = stoi(parts[4]); // Get half-move clock (number of half-moves since last capture or pawn move)
    fullMoveNumber = stoi(parts[5]); // Get full-move number (total number of moves)
}

// Function to get castling rights from a string
std::vector<bool> getCastlingAvailability(const std::string& castlingRights)
{
    std::vector<bool> castlingAvailability = { false, false, false, false }; // Initial castling rights (all false)

    // Check each castling right and update the vector
    castlingAvailability[0] = (castlingRights.find('K') != std::string::npos); // White Kingside
    castlingAvailability[1] = (castlingRights.find('Q') != std::string::npos); // White Queenside
    castlingAvailability[2] = (castlingRights.find('k') != std::string::npos); // Black Kingside
    castlingAvailability[3] = (castlingRights.find('q') != std::string::npos); // Black Queenside

    return castlingAvailability; // Return the vector of castling rights
}

//----------------------------------------- Advanced Chess Rules ----------------------------------------------------

// Forward declarations of helper functions
bool isSquareUnderAttack(const std::vector<std::string>& negativeLegalMoves, int row, int col);
void removeStringFromVector(std::vector<std::string>& vec, const std::string& toRemove);
std::vector<std::string> buildNegativeLegalMoves(const std::string& fen, bool whitesTurn);

void FENParser::extractFENDetails(const std::string& fen, std::vector<std::string>& legalMoves, bool& drawOfferable)
{
    std::vector<std::string> board;
    bool whitesTurn;
    std::vector<bool> castlingRights;
    std::string enPassantTarget;
    int halfMoveClock;
    int fullMoveNumber;

    parseFEN(fen, board, whitesTurn, castlingRights, enPassantTarget, halfMoveClock, fullMoveNumber);
    detectPossibleCastlingMoves(fen, legalMoves, board, whitesTurn, castlingRights);
    detectEnPassant(legalMoves, board, enPassantTarget, whitesTurn);
    drawOfferable = (halfMoveClock >= 100); // Check if a draw can be offered based on half-move clock
}

// Removes a specific string from a vector
void removeStringFromVector(std::vector<std::string>& vec, const std::string& toRemove)
{
    // Move all instances of the string to the end of the vector
    auto newEnd = std::remove(vec.begin(), vec.end(), toRemove);
    vec.erase(newEnd, vec.end()); // Remove the moved elements
}

// Checks if a square is attacked by any move in the list
bool isSquareUnderAttack(const std::vector<std::string>& negativeLegalMoves, int row, int col)
{
    for (const std::string& move : negativeLegalMoves)
    {
        if (move.substr(2, 2) == (COLUMN_LETTERS[col] + std::to_string(row + 1)))
        {
            return true; // Square is under attack
        }
    }
    return false; // Square is not under attack
}

// Builds a list of moves that could attack a square if the opponent were to move
std::vector<std::string> buildNegativeLegalMoves(const std::string& fen, bool whitesTurn)
{
    ChessAIHandler chessAI;
    std::string negativeFEN = fen;

    // Switch the color in the FEN string to get the opponent's moves
    std::string currentColor = whitesTurn ? "w" : "b";
    int colorPosition = fen.find(currentColor);
    negativeFEN[colorPosition] = (currentColor == "w" ? 'b' : 'w');

    StockfishResponse response = chessAI.getChessAIFeedback(1, negativeFEN);
    return response.legalMoves; // Return the list of legal moves for the opponent
}

// Detects possible castling moves based on board state and castling rights
void FENParser::detectPossibleCastlingMoves(const std::string& fen, std::vector<std::string>& legalMoves,
    std::vector<std::string> board, bool whitesTurn,
    std::vector<bool> castlingRights)
{
    std::vector<std::string> negativeLegalMoves;

    // Check castling possibilities for the current side
    if (whitesTurn)
    {
        addCastlingOptionIfPossible(legalMoves, negativeLegalMoves, board, whitesTurn, castlingRights[0], true, "e1g1|h1f1"); // White Kingside
        addCastlingOptionIfPossible(legalMoves, negativeLegalMoves, board, whitesTurn, castlingRights[1], false, "e1c1|a1d1"); // White Queenside
    }
    else
    {
        addCastlingOptionIfPossible(legalMoves, negativeLegalMoves, board, whitesTurn, castlingRights[2], true, "e8g8|h8f8"); // Black Kingside
        addCastlingOptionIfPossible(legalMoves, negativeLegalMoves, board, whitesTurn, castlingRights[3], false, "e8c8|a8d8"); // Black Queenside
    }
}

// Adds castling moves to the list if allowed
void FENParser::addCastlingOptionIfPossible(std::vector<std::string>& legalMoves,
    std::vector<std::string>& negativeLegalMoves,
    const std::vector<std::string>& board, bool whitesTurn,
    bool castlingAllowed, bool firstCastlingCheck,
    const std::string& castlingMove)
{
    //Checks if the castling string is included in the legalMoves
    bool castlingStringIncluded = (std::find(legalMoves.begin(), legalMoves.end(), castlingMove.substr(0, 4)) != legalMoves.end());

    if (castlingAllowed && castlingStringIncluded)
    {
        const int kingRow = whitesTurn ? 7 : 0; // Determine king's row based on color
        int minColumn = firstCastlingCheck ? 5 : 1; // Columns to check for castling
        int maxColumn = firstCastlingCheck ? 7 : 4;

        // Check if the king is in check
        if (isSquareUnderAttack(negativeLegalMoves, kingRow, 4))
        {
            castlingAllowed = false; // Castling is not allowed if king is in check
        }

        if (castlingAllowed)
        {
            // Check if squares between king and rook are empty and not attacked
            for (int currentColumn = minColumn; currentColumn < maxColumn; currentColumn++)
            {
                if ((board[kingRow * 8 + currentColumn] != ".") || isSquareUnderAttack(negativeLegalMoves, kingRow, currentColumn))
                {
                    castlingAllowed = false; // Castling is not allowed if any square is occupied or attacked
                    break;
                }
            }

            if (castlingAllowed)
            {
                removeStringFromVector(legalMoves, castlingMove.substr(0, 4)); // Remove any existing castling move
                legalMoves.push_back(castlingMove); // Add the new castling move
            }
        }
    }
}

// Detects en passant moves based on board state and en passant target
void FENParser::detectEnPassant(std::vector<std::string>& legalMoves, const std::vector<std::string>& board,
    const std::string& enPassantTarget, bool whitesTurn)
{
    bool enPassantPossible = (enPassantTarget != "-" && enPassantTarget.size() == 2);

    if (enPassantPossible)
    {
        int targetColumn = COLUMN_LETTERS.find(enPassantTarget[0]); // Column of en passant target
        int targetRow = 8 - std::stoi(enPassantTarget.substr(1, 1)); // Row of en passant target

        // Check if the target square is empty
        if (board[targetRow * 8 + targetColumn] != ".")
        {
            enPassantPossible = false; // En passant is not possible if target square is occupied
        }

        if (enPassantPossible)
        {
            int pawnRowDirection = whitesTurn ? 1 : -1; // Determine direction based on current turn
            int adjacentRow = targetRow + pawnRowDirection;
            std::string pawn = whitesTurn ? "P" : "p";

            // Create en passant moves
            std::string enPassantMoveAddon = enPassantTarget + "-" + enPassantTarget.substr(0, 1) + std::to_string(8 - adjacentRow);

            // Adjust row and column indices for the board representation
            if ((targetColumn > 0) && (board[(adjacentRow * 8) + (targetColumn - 1)] == pawn))
            {
                std::string startingField = COLUMN_LETTERS[targetColumn - 1] + std::to_string(8 - adjacentRow);
                removeStringFromVector(legalMoves, startingField + enPassantTarget); // Remove old en passant move if it exists
                legalMoves.push_back(startingField + enPassantMoveAddon); // Add new en passant move
            }
            else if ((targetColumn < 7) && (board[(adjacentRow * 8) + (targetColumn + 1)] == pawn))
            {
                std::string startingField = COLUMN_LETTERS[targetColumn + 1] + std::to_string(8 - adjacentRow);
                removeStringFromVector(legalMoves, startingField + enPassantTarget); // Remove old en passant move if it exists
                legalMoves.push_back(startingField + enPassantMoveAddon); // Add new en passant move
            }
        }
    }
}