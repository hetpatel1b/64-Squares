#include "EngineAdapter.hpp"
#include "Queen.hpp"
#include "Rook.hpp"
#include "Bishop.hpp"
#include "Knight.hpp"

EngineAdapter::EngineAdapter()
    : engine() {}

EngineAdapter::EngineAdapter(ChessBoard initialBoard, Color startingTurn)
    : engine(std::move(initialBoard), startingTurn) {}

std::optional<PieceInfo> EngineAdapter::getPieceAt(Position pos) const {
    if (!isInside(pos)) return std::nullopt;
    const Piece* p = engine.getBoard().getPiece(pos);
    if (p == nullptr) return std::nullopt;
    return PieceInfo{ p->getColor(), p->getType() };
}

Color EngineAdapter::getCurrentTurn() const {
    return engine.getTurn();
}

std::vector<Position> EngineAdapter::getLegalDestinations(Position from) const {
    std::vector<Position> dests;
    if (!isInside(from)) return dests;

    const Piece* piece = engine.getBoard().getPiece(from);
    if (piece == nullptr || piece->getColor() != engine.getTurn()) {
        return dests;
    }

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Position to{ r, c };
            if (from != to && engine.isLegalMove(from, to)) {
                dests.push_back(to);
            }
        }
    }
    return dests;
}

bool EngineAdapter::isLegalMove(Position from, Position to) const {
    return engine.isLegalMove(from, to);
}

bool EngineAdapter::isPromotionMove(Position from, Position to) const {
    if (!isInside(from) || !isInside(to)) return false;
    const Piece* piece = engine.getBoard().getPiece(from);
    if (piece == nullptr || piece->getType() != PieceType::Pawn) {
        return false;
    }
    if (piece->getColor() == Color::White && to.row == 0) return true;
    if (piece->getColor() == Color::Black && to.row == 7) return true;
    return false;
}

void EngineAdapter::applyPromotion(Position to, PieceType promo, Color color) {
    auto& mutableBoard = const_cast<ChessBoard&>(engine.getBoard());
    std::unique_ptr<Piece> newPiece;
    switch (promo) {
        case PieceType::Queen:
            newPiece = std::make_unique<Queen>(color);
            break;
        case PieceType::Rook:
            newPiece = std::make_unique<Rook>(color);
            break;
        case PieceType::Bishop:
            newPiece = std::make_unique<Bishop>(color);
            break;
        case PieceType::Knight:
            newPiece = std::make_unique<Knight>(color);
            break;
        default:
            newPiece = std::make_unique<Queen>(color);
            break;
    }
    mutableBoard.setPiece(to, std::move(newPiece));
}

std::string EngineAdapter::formatAlgebraic(Position from, Position to, PieceType piece,
                                          bool isCapture, bool givesCheck, bool givesMate,
                                          std::optional<PieceType> promo) {
    char fromFile = static_cast<char>('a' + from.col);
    char toFile = static_cast<char>('a' + to.col);
    char toRank = static_cast<char>('8' - to.row);

    std::string notation;
    if (piece == PieceType::Pawn) {
        if (isCapture) {
            notation += fromFile;
            notation += 'x';
        }
        notation += toFile;
        notation += toRank;
    } else {
        char pieceChar = 'K';
        switch (piece) {
            case PieceType::Queen:  pieceChar = 'Q'; break;
            case PieceType::Rook:   pieceChar = 'R'; break;
            case PieceType::Bishop: pieceChar = 'B'; break;
            case PieceType::Knight: pieceChar = 'N'; break;
            case PieceType::King:   pieceChar = 'K'; break;
            default: break;
        }
        notation += pieceChar;
        if (isCapture) {
            notation += 'x';
        }
        notation += toFile;
        notation += toRank;
    }

    if (promo.has_value()) {
        notation += '=';
        switch (*promo) {
            case PieceType::Queen:  notation += 'Q'; break;
            case PieceType::Rook:   notation += 'R'; break;
            case PieceType::Bishop: notation += 'B'; break;
            case PieceType::Knight: notation += 'N'; break;
            default: notation += 'Q'; break;
        }
    }

    if (givesMate) {
        notation += '#';
    } else if (givesCheck) {
        notation += '+';
    }

    return notation;
}

bool EngineAdapter::makeMove(Position from, Position to, PieceType promotion) {
    if (!engine.isLegalMove(from, to)) {
        return false;
    }

    const Piece* moving = engine.getBoard().getPiece(from);
    if (moving == nullptr) return false;
    PieceType pieceType = moving->getType();
    Color movingColor = moving->getColor();

    const Piece* destPiece = engine.getBoard().getPiece(to);
    std::optional<PieceType> captured = std::nullopt;
    if (destPiece != nullptr) {
        captured = destPiece->getType();
    }

    bool isPromo = isPromotionMove(from, to);

    // Execute engine move
    if (!engine.tryMove(from, to)) {
        return false;
    }

    // Apply promotion piece substitution if triggered
    if (isPromo) {
        applyPromotion(to, promotion, movingColor);
    }

    // Record captured pieces
    if (captured.has_value()) {
        if (movingColor == Color::White) {
            whiteCaptured.push_back(*captured);
        } else {
            blackCaptured.push_back(*captured);
        }
    }

    bool givesCheck = engine.isInCheck(engine.getTurn());
    bool givesMate = engine.isCheckmate(engine.getTurn());

    std::string san = formatAlgebraic(from, to, pieceType, captured.has_value(),
                                      givesCheck, givesMate,
                                      isPromo ? std::make_optional(promotion) : std::nullopt);

    MoveInfo info;
    info.moveNumber = static_cast<int>(moveHistory.size() / 2) + 1;
    info.from = from;
    info.to = to;
    info.pieceColor = movingColor;
    info.pieceType = pieceType;
    info.capturedPiece = captured;
    info.promotionPiece = isPromo ? std::make_optional(promotion) : std::nullopt;
    info.isCheck = givesCheck;
    info.isCheckmate = givesMate;
    info.notation = san;

    moveHistory.push_back(info);
    lastMove = std::make_pair(from, to);

    return true;
}

bool EngineAdapter::canUndo() const {
    return !moveHistory.empty();
}

bool EngineAdapter::undo() {
    if (moveHistory.empty()) {
        return false;
    }

    size_t targetCount = moveHistory.size() - 1;
    std::vector<MoveInfo> historyCopy = moveHistory;

    reset();

    for (size_t i = 0; i < targetCount; ++i) {
        makeMove(historyCopy[i].from, historyCopy[i].to,
                 historyCopy[i].promotionPiece.value_or(PieceType::Queen));
    }

    return true;
}

void EngineAdapter::reset() {
    engine.reset();
    moveHistory.clear();
    whiteCaptured.clear();
    blackCaptured.clear();
    lastMove.reset();
}

GameStatus EngineAdapter::getGameStatus() const {
    if (engine.isCheckmate()) return GameStatus::Checkmate;
    if (engine.isStalemate()) return GameStatus::Stalemate;
    if (engine.isInCheck(engine.getTurn())) return GameStatus::Check;
    return GameStatus::Active;
}

std::optional<Color> EngineAdapter::getWinner() const {
    return engine.getWinner();
}

std::optional<Position> EngineAdapter::getKingInCheckSquare() const {
    if (engine.isInCheck(engine.getTurn())) {
        return engine.findKing(engine.getTurn());
    }
    return std::nullopt;
}

std::optional<std::pair<Position, Position>> EngineAdapter::getLastMove() const {
    return lastMove;
}

const std::vector<MoveInfo>& EngineAdapter::getMoveHistory() const {
    return moveHistory;
}

std::vector<PieceType> EngineAdapter::getCapturedPieces(Color side) const {
    return (side == Color::White) ? whiteCaptured : blackCaptured;
}

int EngineAdapter::getMaterialAdvantage(Color side) const {
    auto pieceValue = [](PieceType t) -> int {
        switch (t) {
            case PieceType::Pawn:   return 1;
            case PieceType::Knight: return 3;
            case PieceType::Bishop: return 3;
            case PieceType::Rook:   return 5;
            case PieceType::Queen:  return 9;
            default: return 0;
        }
    };

    int whiteScore = 0;
    int blackScore = 0;
    for (auto p : whiteCaptured) whiteScore += pieceValue(p);
    for (auto p : blackCaptured) blackScore += pieceValue(p);

    return (side == Color::White) ? (whiteScore - blackScore) : (blackScore - whiteScore);
}
