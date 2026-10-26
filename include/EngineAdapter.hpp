#pragma once

#include "IChessGame.hpp"
#include "ChessEngine.hpp"
#include <vector>
#include <optional>
#include <memory>

/**
 * @brief Production adapter connecting the modular desktop UI to the core ChessEngine.
 * 
 * Implements IChessGame completely by delegating rules and move execution to ChessEngine.
 * Handles move notation generation, move history tracking, captured piece counting,
 * material advantage calculations, last move highlighting, and reversible move undo.
 */
class EngineAdapter : public IChessGame {
private:
    ChessEngine engine;
    std::vector<MoveInfo> moveHistory;
    std::vector<PieceType> whiteCaptured; // Pieces captured by White (Black pieces lost)
    std::vector<PieceType> blackCaptured; // Pieces captured by Black (White pieces lost)
    std::optional<std::pair<Position, Position>> lastMove;

    std::string formatAlgebraic(Position from, Position to, PieceType piece,
                                bool isCapture, bool givesCheck, bool givesMate,
                                std::optional<PieceType> promo);

    void applyPromotion(Position to, PieceType promo, Color color);

public:
    EngineAdapter();
    explicit EngineAdapter(ChessBoard initialBoard, Color startingTurn = Color::White);
    ~EngineAdapter() override = default;

    // --- IChessGame Implementation ---
    std::optional<PieceInfo> getPieceAt(Position pos) const override;
    Color getCurrentTurn() const override;
    std::vector<Position> getLegalDestinations(Position from) const override;
    bool isLegalMove(Position from, Position to) const override;
    bool isPromotionMove(Position from, Position to) const override;

    bool makeMove(Position from, Position to, PieceType promotion = PieceType::Queen) override;
    bool canUndo() const override;
    bool undo() override;
    void reset() override;

    GameStatus getGameStatus() const override;
    std::optional<Color> getWinner() const override;
    std::optional<Position> getKingInCheckSquare() const override;
    std::optional<std::pair<Position, Position>> getLastMove() const override;
    const std::vector<MoveInfo>& getMoveHistory() const override;
    std::vector<PieceType> getCapturedPieces(Color side) const override;
    int getMaterialAdvantage(Color side) const override;

    // Direct access to underlying engine if needed by future extensions
    const ChessEngine& getEngine() const { return engine; }
    ChessEngine& getEngine() { return engine; }
};
