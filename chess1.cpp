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