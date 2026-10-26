#pragma once

#include "Position.hpp"
#include <string>
#include <vector>
#include <optional>
#include <utility>

/**
 * @brief High-level game status states recognized by the front-end UI.
 */
enum class GameStatus {
    Active,
    Check,
    Checkmate,
    Stalemate,
    Draw
};

/**
 * @brief Lightweight snapshot of a piece on a square.
 */
struct PieceInfo {
    Color color;
    PieceType type;
};

/**
 * @brief Record of a completed move for move history display and algebraic notation.
 */
struct MoveInfo {
    int moveNumber{1};
    Position from{-1, -1};
    Position to{-1, -1};
    Color pieceColor{Color::White};
    PieceType pieceType{PieceType::Pawn};
    std::optional<PieceType> capturedPiece{std::nullopt};
    std::optional<PieceType> promotionPiece{std::nullopt};
    bool isCheck{false};
    bool isCheckmate{false};
    std::string notation; // Standard algebraic notation (e.g. "e4", "Nxf3+", "e8=Q#")
};

/**
 * @brief Clean, decoupled C++ interface between the Desktop UI and the Chess Engine.
 * 
 * The UI layer queries this interface for board state, legal destinations, move history,
 * and game status. User actions (selecting squares, making moves, undo, restart)
 * are dispatched through these methods.
 * 
 * Engine developers must implement this interface or plug their engine into EngineAdapter.
 */
class IChessGame {
public:
    virtual ~IChessGame() = default;

    // --- Board Snapshot Queries ---

    /**
     * @brief Query whether a square contains a piece, and if so, its color and type.
     */
    virtual std::optional<PieceInfo> getPieceAt(Position pos) const = 0;

    /**
     * @brief Current side to move.
     */
    virtual Color getCurrentTurn() const = 0;

    /**
     * @brief Query all legal destination squares for the piece at the given square.
     * If the square is empty or the piece cannot move legally, returns an empty vector.
     */
    virtual std::vector<Position> getLegalDestinations(Position from) const = 0;

    /**
     * @brief Check whether moving from `from` to `to` is a legal move under current game rules.
     */
    virtual bool isLegalMove(Position from, Position to) const = 0;

    /**
     * @brief Check whether a move from `from` to `to` triggers pawn promotion.
     */
    virtual bool isPromotionMove(Position from, Position to) const = 0;

    // --- Action Dispatching ---

    /**
     * @brief Attempt to execute a move on the board.
     * @param from Origin square.
     * @param to Destination square.
     * @param promotion Chosen piece type if this is a pawn promotion (default Queen).
     * @return true if move was accepted and executed by the engine, false if rejected.
     */
    virtual bool makeMove(Position from, Position to, PieceType promotion = PieceType::Queen) = 0;

    /**
     * @brief Query whether an undo action is currently supported and available.
     */
    virtual bool canUndo() const = 0;

    /**
     * @brief Request to undo the most recent half-move or full-turn.
     * @return true if successfully undone, false if unavailable or rejected.
     */
    virtual bool undo() = 0;

    /**
     * @brief Reset the board and game state to standard initial position.
     */
    virtual void reset() = 0;

    // --- Game Status & History ---

    /**
     * @brief Current game status (Active, Check, Checkmate, Stalemate, Draw).
     */
    virtual GameStatus getGameStatus() const = 0;

    /**
     * @brief Winner of the game if game is concluded with a win, nullopt otherwise.
     */
    virtual std::optional<Color> getWinner() const = 0;

    /**
     * @brief Square of the King that is currently under check, or nullopt if no king is in check.
     */
    virtual std::optional<Position> getKingInCheckSquare() const = 0;

    /**
     * @brief Square coordinates of the most recent move (from, to) for visual board highlighting.
     */
    virtual std::optional<std::pair<Position, Position>> getLastMove() const = 0;

    /**
     * @brief Chronological move history.
     */
    virtual const std::vector<MoveInfo>& getMoveHistory() const = 0;

    /**
     * @brief List of pieces captured by or from the given color.
     */
    virtual std::vector<PieceType> getCapturedPieces(Color side) const = 0;

    /**
     * @brief Net material advantage score for the specified color (e.g. +3 if leading).
     */
    virtual int getMaterialAdvantage(Color side) const = 0;
};
