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

// =================================================
//              BOARD SETUP & DISPLAY
// =================================================

void Board::setupBoard() {
    // Black back rank
    squares[0][0] = new Rook('B', 0, 0);
    squares[0][1] = new Knight('B', 0, 1);
    squares[0][2] = new Bishop('B', 0, 2);
    squares[0][3] = new Queen('B', 0, 3);
    squares[0][4] = new King('B', 0, 4);
    squares[0][5] = new Bishop('B', 0, 5);
    squares[0][6] = new Knight('B', 0, 6);
    squares[0][7] = new Rook('B', 0, 7);

    // Black pawns
    for (int i = 0; i < 8; i++)
        squares[1][i] = new Pawn('B', 1, i);

    // White pawns
    for (int i = 0; i < 8; i++)
        squares[6][i] = new Pawn('W', 6, i);

    // White back rank
    squares[7][0] = new Rook('W', 7, 0);
    squares[7][1] = new Knight('W', 7, 1);
    squares[7][2] = new Bishop('W', 7, 2);
    squares[7][3] = new Queen('W', 7, 3);
    squares[7][4] = new King('W', 7, 4);
    squares[7][5] = new Bishop('W', 7, 5);
    squares[7][6] = new Knight('W', 7, 6);
    squares[7][7] = new Rook('W', 7, 7);
}


void Board::display() const {
    // ANSI color codes
    const string RESET = "\033[0m";
    const string WHITE_BG = "\033[47m";  // White background
    const string BLACK_BG = "\033[100m"; // Gray background (for black squares)
    const string BLACK_TEXT = "\033[30m"; // Black text
    const string WHITE_TEXT = "\033[97m"; // Bright white text

    cout << "\n    a   b   c   d   e   f   g   h\n";
    cout << "  +---+---+---+---+---+---+---+---+\n";

    for (int i = 0; i < 8; i++) {
        cout << 8 - i << " |";
        for (int j = 0; j < 8; j++) {
            // Determine square color (checkerboard pattern)
            bool isWhiteSquare = (i + j) % 2 == 0;

            if (isWhiteSquare) {
                cout << WHITE_BG << BLACK_TEXT;
            }
            else {
                cout << BLACK_BG << WHITE_TEXT;
            }

            if (squares[i][j])
                cout << " " << squares[i][j]->getSymbol() << " ";
            else
                cout << "   ";

            cout << RESET << "|";
        }
        cout << " " << 8 - i << "\n";
        cout << "  +---+---+---+---+---+---+---+---+\n";
    }

    cout << "    a   b   c   d   e   f   g   h\n\n";
}

// =================================================
//                      GAME
// =================================================

class Game {
private:
    Board board;
    char currentPlayer;
    bool gameOver;

public:
    Game() :currentPlayer('W'), gameOver(false) {
        board.setupBoard();
    }

    void play();

private:
    Position parsePosition(string s);

    bool makeMove(string from, string to);
    bool executeMove(Piece* piece, Position from, Position to);
    bool executeCastling(King* king, Position from, Position to);

    void promotePawn(Position pos);
    void checkGameStatus();
};


// =================================================
//                  MOVE PARSING
// =================================================

Position Game::parsePosition(string pos) {
    if (pos.length() != 2)
        return Position(-1, -1);

    char file = toLowerCase(pos[0]);
    int col = file - 'a';
    int row = 8 - (pos[1] - '0');

    return Position(row, col);
}

// =================================================
//                      GAME LOOP
// =================================================

void Game::play() {

    cout << "\n============== CHESS GAME ==============\n";
    cout << "White = Uppercase | Black = Lowercase\n";
    cout << "Enter moves: e2 e4\n";
    cout << "Castling: e1 g1 or e1 c1\n";
    cout << "En passant: automatic\n";
    cout << "Promotion: you choose piece\n";
    cout << "Type 'quit' to exit\n";
    cout << "========================================\n";

    while (!gameOver) {

        board.display();

        // Check game status before showing turn
        checkGameStatus();
        if (gameOver) break;

        cout << (currentPlayer == 'W' ? "White" : "Black") << "'s turn.\n";

        // Display check status
        if (board.isInCheck(currentPlayer)) {
            cout << "*** YOU ARE IN CHECK! ***\n";
        }

        cout << "Enter move (from to): ";

        string from, to;
        cin >> from;

        if (from == "quit" || from == "QUIT") {
            cout << "\nGame ended. Goodbye!\n";
            return;
        }

        cin >> to;

        if (makeMove(from, to)) {
            currentPlayer = (currentPlayer == 'W' ? 'B' : 'W');
        }
        else {
            cout << "\n*** Invalid move! Try again. ***\n\n";
        }
    }

    board.display();
    cout << "\n=========== GAME OVER ===========\n";
}


// =================================================
//              CHECK GAME STATUS
// =================================================

void Game::checkGameStatus() {
    if (board.isCheckmate(currentPlayer)) {
        cout << "\n************ CHECKMATE! ************\n";
        cout << (currentPlayer == 'W' ? "Black" : "White") << " wins!\n";
        gameOver = true;
    }
    else if (board.isStalemate(currentPlayer)) {
        cout << "\n************ STALEMATE! ************\n";
        cout << "The game is a draw!\n";
        gameOver = true;
    }
}


// =================================================
//                MOVE VALIDATION
// =============================================

// =================================================
//                MOVE EXECUTION
// =================================================

bool Game::executeCastling(King* king, Position from, Position to) {
    int r = from.getRow();
    int direction = (to.getCol() > from.getCol()) ? 1 : -1;
    int rookFromCol = (direction == 1) ? 7 : 0;
    int rookToCol = (direction == 1) ? 5 : 3;

    Piece* rook = board.getPiece(r, rookFromCol);

    board.setPiece(to, king);
    board.setPiece(r, from.getCol(), nullptr);
    king->setPosition(to);

    board.setPiece(r, rookToCol, rook);
    board.setPiece(r, rookFromCol, nullptr);
    rook->setPosition(Position(r, rookToCol));

    board.clearLastPawnDoubleMove();
    return true;
}

void Game::promotePawn(Position pos) {
    cout << "Pawn promotion! Choose piece (Q/R/B/N): ";
    char choice;
    cin >> choice;
    choice = toupper(choice);

    Piece* newPiece = nullptr;
    char c = board.getPiece(pos)->getColor();

    board.removePiece(pos);

    switch (choice) {
    case 'R': newPiece = new Rook(c, pos.getRow(), pos.getCol());   break;
    case 'B': newPiece = new Bishop(c, pos.getRow(), pos.getCol()); break;
    case 'N': newPiece = new Knight(c, pos.getRow(), pos.getCol()); break;
    default:  newPiece = new Queen(c, pos.getRow(), pos.getCol());  break;
    }

    board.setPiece(pos, newPiece);
}

bool Game::executeMove(Piece* piece, Position from, Position to) {
    // En passant capture
    if (piece->getPieceName() == "Pawn") {
        int colDiff = to.getCol() - from.getCol();
        if (colDiff != 0 && board.getPiece(to) == nullptr) {
            // Captured pawn sits on same row as 'from', same col as 'to'
            board.removePiece(Position(from.getRow(), to.getCol()));
        }
    }

    // Capture destination piece if present
    Piece* target = board.getPiece(to);
    if (target != nullptr) {
        board.removePiece(to);
    }

    // Move piece
    board.setPiece(to, piece);
    board.setPiece(from, nullptr);
    piece->setPosition(to);

    // Track pawn double move for en passant
    if (piece->getPieceName() == "Pawn" &&
        abs(to.getRow() - from.getRow()) == 2) {
        board.setLastPawnDoubleMove(to);
    }
    else {
        board.clearLastPawnDoubleMove();
    }

    // Pawn promotion
    if (piece->getPieceName() == "Pawn") {
        if ((piece->getColor() == 'W' && to.getRow() == 0) ||
            (piece->getColor() == 'B' && to.getRow() == 7)) {
            promotePawn(to);
        }
    }

    return true;
}

bool Game::makeMove(string fromStr, string toStr) {
    Position from = parsePosition(fromStr);
    Position to = parsePosition(toStr);

    if (!from.isValid() || !to.isValid()) {
        cout << "Invalid square.\n";
        return false;
    }

    Piece* piece = board.getPiece(from);
    if (piece == nullptr) {
        cout << "No piece at " << fromStr << ".\n";
        return false;
    }

    if (piece->getColor() != currentPlayer) {
        cout << "That is not your piece.\n";
        return false;
    }

    if (!piece->isValidMove(to, board)) {
        return false;
    }

    // --- Simulate move to check for self-check ---
    int sr = from.getRow(), sc = from.getCol();
    int dr = to.getRow(), dc = to.getCol();
    Piece* captured = board.getPiece(dr, dc);

    board.setPiece(dr, dc, piece);
    board.setPiece(sr, sc, nullptr);
    Position oldPos = piece->getPosition();
    piece->setPosition(to);

    bool inCheck = board.isInCheck(currentPlayer);

    // Undo simulation
    board.setPiece(sr, sc, piece);
    board.setPiece(dr, dc, captured);
    piece->setPosition(oldPos);

    if (inCheck) {
        cout << "Move leaves king in check!\n";
        return false;
    }

    // --- Execute the real move ---

    // Castling
    if (piece->getPieceName() == "King" &&
        abs(to.getCol() - from.getCol()) == 2) {
        return executeCastling(static_cast<King*>(piece), from, to);
    }

    return executeMove(piece, from, to);
}

// =================================================
//                      MAIN
// =================================================

int main() {
    Game game;
    game.play();
    return 0;
}