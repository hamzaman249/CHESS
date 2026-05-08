#include <iostream>
#include <string>
using namespace std;

// ---------------- Utility helpers ----------------
char toLowerCase(char c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

int absValue(int x) { return (x < 0 ? -x : x); }
int maxValue(int a, int b) { return (a > b ? a : b); }
int minValue(int a, int b) { return (a < b ? a : b); }

// =================================================
//                    POSITION
// =================================================
class Position {
private:
    int row, col;

public:
    Position(int r = 0, int c = 0) :row(r), col(c) {}
    int getRow() const { return row; }
    int getCol() const { return col; }
    void setRow(int r) { row = r; }
    void setCol(int c) { col = c; }

    bool isValid() const {
        return (row >= 0 && row < 8 && col >= 0 && col < 8);
    }

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

// Forward declare Piece
class Piece;

// =================================================
//                      BOARD
// =================================================
class Board {
private:
    Piece* squares[8][8];
    Position lastPawnDoubleMove;
    bool lastMoveWasPawnDouble;

public:
    Board();
    ~Board();

    Piece* getPiece(int r, int c) const;
    Piece* getPiece(Position p) const;

    void setPiece(int r, int c, Piece* p);
    void setPiece(Position pos, Piece* p);

    void removePiece(int r, int c);
    void removePiece(Position p);

    Position getLastPawnDoubleMove() const { return lastPawnDoubleMove; }
    bool wasLastMovePawnDouble() const { return lastMoveWasPawnDouble; }
    void setLastPawnDoubleMove(Position p);
    void clearLastPawnDoubleMove();

    void setupBoard();
    void display() const;

    // Check/Checkmate detection functions
    Position findKing(char color) const;
    bool isSquareUnderAttack(Position pos, char byColor) const;
    bool isInCheck(char kingColor) const;
    bool hasLegalMove(char playerColor);
    bool isCheckmate(char kingColor);
    bool isStalemate(char playerColor);
};

// =================================================
//                    PIECE BASE
// =================================================
class Piece {
protected:
    char color;
    Position position;
    char symbol;
    bool hasMoved;

public:
    Piece(char c, int r, int col, char sym)
        :color(c), position(r, col), symbol(sym), hasMoved(false) {
    }

    virtual ~Piece() {}

    virtual bool isValidMove(Position dest, Board& board) = 0;
    virtual string getPieceName() const = 0;

    char getColor() const { return color; }
    char getSymbol() const { return symbol; }
    Position getPosition() const { return position; }

    void setPosition(Position p) {
        position = p;
        hasMoved = true;
    }

    bool getHasMoved() const { return hasMoved; }
    void setHasMoved(bool m) { hasMoved = m; }

    bool isPathClear(Position dest, Board& board);
};

// =================================================
//              BOARD IMPLEMENTATION
// =================================================
Board::Board() :lastPawnDoubleMove(-1, -1), lastMoveWasPawnDouble(false) {
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            squares[i][j] = nullptr;
}

Board::~Board() {
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (squares[i][j] != nullptr)
                delete squares[i][j];
}

Piece* Board::getPiece(int r, int c) const {
    if (r < 0 || r>7 || c < 0 || c>7) return nullptr;
    return squares[r][c];
}

Piece* Board::getPiece(Position p) const {
    return getPiece(p.getRow(), p.getCol());
}

void Board::setPiece(int r, int c, Piece* p) {
    squares[r][c] = p;
}

void Board::setPiece(Position pos, Piece* p) {
    setPiece(pos.getRow(), pos.getCol(), p);
}

void Board::removePiece(int r, int c) {
    if (r >= 0 && r < 8 && c >= 0 && c < 8) {
        if (squares[r][c] != nullptr) {
            delete squares[r][c];
            squares[r][c] = nullptr;
        }
    }
}

void Board::removePiece(Position p) {
    removePiece(p.getRow(), p.getCol());
}

void Board::setLastPawnDoubleMove(Position p) {
    lastPawnDoubleMove = p;
    lastMoveWasPawnDouble = true;
}

void Board::clearLastPawnDoubleMove() {
    lastPawnDoubleMove = Position(-1, -1);
    lastMoveWasPawnDouble = false;
}

// Find the King of given color
Position Board::findKing(char color) const {
    char kingSymbol = (color == 'W' ? 'K' : 'k');
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (squares[i][j] != nullptr && squares[i][j]->getSymbol() == kingSymbol) {
                return Position(i, j);
            }
        }
    }
    return Position(-1, -1);
}

// Check if a square is under attack by pieces of a given color
bool Board::isSquareUnderAttack(Position pos, char byColor) const {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Piece* piece = squares[i][j];
            if (piece != nullptr && piece->getColor() == byColor) {
                // Need to create a temporary non-const board for validation
                Board* tempBoard = const_cast<Board*>(this);
                if (piece->isValidMove(pos, *tempBoard)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Check if the king of given color is in check
bool Board::isInCheck(char kingColor) const {
    Position kingPos = findKing(kingColor);
    if (kingPos.getRow() == -1) return false;

    char opponentColor = (kingColor == 'W' ? 'B' : 'W');
    return isSquareUnderAttack(kingPos, opponentColor);
}

// Check if player has any legal moves
bool Board::hasLegalMove(char playerColor) {
    // Try every piece of the player
    for (int sr = 0; sr < 8; sr++) {
        for (int sc = 0; sc < 8; sc++) {
            Piece* piece = squares[sr][sc];
            if (piece == nullptr || piece->getColor() != playerColor)
                continue;

            // Try every possible destination
            for (int dr = 0; dr < 8; dr++) {
                for (int dc = 0; dc < 8; dc++) {
                    Position dest(dr, dc);

                    if (!piece->isValidMove(dest, *this))
                        continue;

                    // Simulate the move
                    Position from = piece->getPosition();
                    Piece* captured = squares[dr][dc];

                    squares[sr][sc] = nullptr;
                    squares[dr][dc] = piece;
                    Position oldPos = piece->getPosition();
                    piece->setPosition(dest);

                    // Check if king is still in check
                    bool stillInCheck = isInCheck(playerColor);

                    // Undo the move
                    squares[dr][dc] = captured;
                    squares[sr][sc] = piece;
                    piece->setPosition(oldPos);

                    if (!stillInCheck)
                        return true;
                }
            }
        }
    }
    return false;
}

// Check if it's checkmate
bool Board::isCheckmate(char kingColor) {
    if (!isInCheck(kingColor))
        return false;
    return !hasLegalMove(kingColor);
}

// Check if it's stalemate
bool Board::isStalemate(char playerColor) {
    if (isInCheck(playerColor))
        return false;
    return !hasLegalMove(playerColor);
}


// =================================================
//                DERIVED PIECES
// =================================================

class Pawn : public Piece {
public:
    Pawn(char c, int r, int col)
        :Piece(c, r, col, (c == 'W' ? 'P' : 'p')) {
    }
    string getPieceName() const { return "Pawn"; }
    bool isValidMove(Position dest, Board& board);
};

class Rook : public Piece {
public:
    Rook(char c, int r, int col)
        :Piece(c, r, col, (c == 'W' ? 'R' : 'r')) {
    }
    string getPieceName() const { return "Rook"; }
    bool isValidMove(Position dest, Board& board);
};

class Knight : public Piece {
public:
    Knight(char c, int r, int col)
        :Piece(c, r, col, (c == 'W' ? 'N' : 'n')) {
    }
    string getPieceName() const { return "Knight"; }
    bool isValidMove(Position dest, Board& board);
};

class Bishop : public Piece {
public:
    Bishop(char c, int r, int col)
        :Piece(c, r, col, (c == 'W' ? 'B' : 'b')) {
    }
    string getPieceName() const { return "Bishop"; }
    bool isValidMove(Position dest, Board& board);
};

class Queen : public Piece {
public:
    Queen(char c, int r, int col)
        :Piece(c, r, col, (c == 'W' ? 'Q' : 'q')) {
    }
    string getPieceName() const { return "Queen"; }
    bool isValidMove(Position dest, Board& board);
};

class King : public Piece {
public:
    King(char c, int r, int col)
        :Piece(c, r, col, (c == 'W' ? 'K' : 'k')) {
    }
    string getPieceName() const { return "King"; }
    bool isValidMove(Position dest, Board& board);
private:
    bool isCastlingValid(Position dest, Board& board);
};


// =================================================
//                  PATH CHECKER
// =================================================
bool Piece::isPathClear(Position dest, Board& board) {
    int rowStep = 0, colStep = 0;

    if (dest.getRow() > position.getRow()) rowStep = 1;
    else if (dest.getRow() < position.getRow()) rowStep = -1;

    if (dest.getCol() > position.getCol()) colStep = 1;
    else if (dest.getCol() < position.getCol()) colStep = -1;

    int r = position.getRow() + rowStep;
    int c = position.getCol() + colStep;

    while (r != dest.getRow() || c != dest.getCol()) {
        if (board.getPiece(r, c) != nullptr)
            return false;
        r += rowStep;
        c += colStep;
    }

    Piece* target = board.getPiece(dest);
    if (target == nullptr) return true;
    return target->getColor() != color;
};

// =================================================
//                PAWN
// =================================================
bool Pawn::isValidMove(Position dest, Board& board) {
    if (!dest.isValid()) return false;

    int sr = position.getRow();
    int sc = position.getCol();
    int dr = dest.getRow();
    int dc = dest.getCol();

    int rowDiff = dr - sr;
    int colDiff = dc - sc;
    int absCol = absValue(colDiff);

    int dir = (color == 'W' ? -1 : 1);

    // 1-step forward
    if (colDiff == 0 && rowDiff == dir) {
        if (board.getPiece(dr, dc) == nullptr) return true;
        return false;
    }

    // 2-step forward from starting row
    int startRow = (color == 'W' ? 6 : 1);
    if (sr == startRow && colDiff == 0 && rowDiff == 2 * dir) {
        if (board.getPiece(sr + dir, sc) == nullptr &&
            board.getPiece(dr, dc) == nullptr)
            return true;
        return false;
    }

    // Diagonal capture
    if (absCol == 1 && rowDiff == dir) {
        Piece* target = board.getPiece(dr, dc);
        if (target != nullptr) {
            return target->getColor() != color;
        }

        // En passant
        if (board.wasLastMovePawnDouble()) {
            Position last = board.getLastPawnDoubleMove();
            if (last.getRow() == sr && last.getCol() == dc) {
                Piece* cand = board.getPiece(last);
                if (cand && cand->getPieceName() == "Pawn" && cand->getColor() != color)
                    return true;
            }
        }
        return false;
    }

    return false;
}


// =================================================
//                ROOK
// =================================================
bool Rook::isValidMove(Position dest, Board& board) {
    if (position.getRow() != dest.getRow() &&
        position.getCol() != dest.getCol())
        return false;
    return isPathClear(dest, board);
}

// =================================================
//                BISHOP
// =================================================
bool Bishop::isValidMove(Position dest, Board& board) {
    int r = absValue(dest.getRow() - position.getRow());
    int c = absValue(dest.getCol() - position.getCol());
    if (r != c) return false;
    return isPathClear(dest, board);
}


// =================================================
//                QUEEN
// =================================================
bool Queen::isValidMove(Position dest, Board& board) {
    int r = absValue(dest.getRow() - position.getRow());
    int c = absValue(dest.getCol() - position.getCol());
    if (r == c ||
        position.getRow() == dest.getRow() ||
        position.getCol() == dest.getCol())
        return isPathClear(dest, board);
    return false;
}

// =================================================
//                KING
// =================================================
bool King::isCastlingValid(Position dest, Board& board) {
    int r = position.getRow();
    int direction = (dest.getCol() > position.getCol() ? 1 : -1);
    int rookCol = (direction == 1 ? 7 : 0);

    Piece* rook = board.getPiece(r, rookCol);
    if (!rook) return false;
    if (rook->getPieceName() != "Rook") return false;
    if (rook->getHasMoved()) return false;

    int cStart = position.getCol();
    for (int c = minValue(cStart, rookCol) + 1; c < maxValue(cStart, rookCol); c++) {
        if (board.getPiece(r, c) != nullptr) return false;
    }

    // Check if king passes through or ends in check
    char opponentColor = (color == 'W' ? 'B' : 'W');
    for (int c = cStart; c != dest.getCol() + direction; c += direction) {
        if (board.isSquareUnderAttack(Position(r, c), opponentColor))
            return false;
    }

    return true;
}

bool King::isValidMove(Position dest, Board& board) {
    int r = absValue(dest.getRow() - position.getRow());
    int c = absValue(dest.getCol() - position.getCol());

    // normal king move
    if (r <= 1 && c <= 1) {
        Piece* t = board.getPiece(dest);
        return (t == nullptr || t->getColor() != color);
    }

    // castling
    if (!hasMoved && r == 0 && c == 2) {
        return isCastlingValid(dest, board);
    }

    return false;
}
