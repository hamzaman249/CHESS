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