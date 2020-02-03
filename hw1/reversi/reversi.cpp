#include <vector>
#include <stdexcept>
#include <sstream>

#include "reversi.h"

using namespace std;




void Square::flip()
{
    switch (value_)
    {
    case WHITE:
        value_ = BLACK;
        break;
    case BLACK:
        value_ = WHITE;
        break;
    default:
        break;
    }
}

Square &Square::operator=(Square::SquareValue value) {
    this->value_ = value;
    return *this;
}

bool Square::operator==(Square::SquareValue value) const {
    return this->value_ == value;
}

bool Square::operator!=(Square::SquareValue value) const {
    return this->value_ != value;
}

Square::SquareValue opposite_color(Square::SquareValue value)
{
    switch (value)
    {
    case Square::WHITE:
        return Square::BLACK;
    case Square::BLACK:
        return Square::WHITE;
    default:
        throw invalid_argument("Illegal square value");
    }
}







Square& Board::operator()(char row, size_t column)
{
    if (!is_valid_location(row, column))
    {
        throw out_of_range("Bad row index");
    }
    size_t row_index = row_to_index(row);
    return squares_[row_index][column - 1];
}

Square const& Board::operator()(char row, size_t column) const
{
    if (!is_valid_location(row, column))
    {
        throw out_of_range("Bad row index");
    }
    size_t row_index = row_to_index(row);
    return squares_[row_index][column - 1];
}

bool Board::is_legal_and_opposite_color(
    char row, size_t column, Square::SquareValue turn) const
{
    if (is_valid_location(row, column))
    {
        size_t row_index = row_to_index(row);
        return squares_[row_index][column - 1] != Square::FREE && squares_[row_index][column - 1] != turn;
    }
    return false;
}

bool Board::is_legal_and_same_color(
    char row, size_t column, Square::SquareValue turn) const
{
    if (is_valid_location(row, column))
    {
        size_t row_index = row_to_index(row);
        return squares_[row_index][column - 1] == turn;
    }
    return false;
}

bool Board::is_valid_location(char row, size_t column) const
{
    size_t row_index = row_to_index(row);
    return row_index < dimension_ && column - 1 < dimension_;
}

Checkpoint::Checkpoint(const Board& b, Square::SquareValue turn) :
    board_(b),
    turn_(turn)
{

}

ostream& Board::print(ostream& out) const
{
    out << "  ";
    for (size_t i = 1; i <= dimension_; i++)
    {
        if (i < 10)
        {
            out << " ";
        }
        else
        {
            out << (i / 10);
        }
    }
    out << endl;
    out << "  ";
    for (size_t i = 1; i <= dimension_; i++)
    {
        out << (i % 10);
    }
    out << endl;
    for (size_t i = 0; i < dimension_; i++)
    {
        out << (char)('a' + i) << ':';
        for (size_t k = 0; k < dimension_; k++)
        {
            out << squares_[i][k];
        }
        out << endl;
    }
    return out;
}

Board::Board(size_t s) {
    dimension_ = s;
    squares_ = new Square*[s];
    for(size_t i = 0 ; i < s;i++) {
        squares_[i] = new Square[s]();
    }
}

std::ostream &operator<<(std::ostream &out, const Board &board) {
    return board.print(out);
}

Board::~Board() {
    for(int i = 0; i < dimension_; i++) {
        delete[] this->squares_[i];
    }
    delete[] squares_;
}

Board::Board(const Board &b) {
    dimension_ = b.dimension_;
    squares_ = new Square*[dimension_];
    for(int i = 0; i < dimension_; i++) {
        squares_[i] = new Square[dimension_]();
        for(int j = 0; j < dimension_; j++) {
            squares_[i][j] = b.squares_[i][j];
        }
    }
}

Board &Board::operator=(const Board &b) {
    if(this == &b) return *this;
    for(int i = 0; i < dimension_; i++) {
        delete[] this->squares_[i];
    }
    delete[] squares_;
    dimension_ = b.dimension_;
    squares_ = new Square*[dimension_];
    for(int i = 0; i < dimension_; i++) {
        squares_[i] = new Square[dimension_]();
        for(int j = 0; j < dimension_; j++) {
            squares_[i][j] = b.squares_[i][j];
        }
    }
    return *this;
}


void Reversi::prompt() const
{
    cout << board_ << endl;
    cout << (turn_ == Square::BLACK ? "B" : "W");
    cout << " - Enter 'p r/c', 'q', 'c', 'u':" << endl;
}

void Reversi::win_loss_tie_message(size_t white_count, size_t black_count)
{
    cout << board_ << endl;
    if (white_count > black_count)
    {
        cout << "W wins" << endl;
    }
    else if (white_count < black_count)
    {
        cout << "B wins" << endl;
    }
    else
    {
        cout << "Tie" << endl;
    }
    cout << "W=" << white_count << "/B=" << black_count << endl;
}

bool Reversi::is_legal_choice(char row, size_t column, Square::SquareValue turn) const
{
    // Vectors for each cardinal direction
    const size_t direction_count = 8;
    const int direction_row[] =    {-1, -1,  0, +1, +1, +1,  0, -1};
    const int direction_column[] = { 0, -1, -1, -1,  0, +1, +1, +1};

    // Make sure location is free
    if (board_(row, column) != Square::FREE)
    {
        return false;
    }

    // Now check in each directions
    for (size_t d = 0; d < direction_count; d++)
    {
        // Where we're checking
        char cursor_row = row + direction_row[d];
        size_t cursor_column = column + direction_column[d];

        // Move towards the direction while we're on the opposite color
        bool found_opposite = false;
        while (board_.is_legal_and_opposite_color(cursor_row, cursor_column, turn_))
        {
            found_opposite = true;
            cursor_row += direction_row[d];
            cursor_column += direction_column[d];
        }

        // Check if the next thing after is our color
        bool found_same = board_.is_legal_and_same_color(cursor_row, cursor_column, turn_);

        // If this direction is valid, the move is valid, so short circuit and return
        if (found_opposite && found_same) {
            return true;
        }
    }
    return false;
}

bool Reversi::is_game_over() const
{
    for (unsigned char row = 'a'; row < 'a' + board_.dimension(); row++)
    {
        for (size_t column = 1; column <= board_.dimension(); column++)
        {
            if (is_legal_choice(row, column, turn_))
            {
                return false;
            }
        }
    }
    return true;
}

Reversi::Reversi(size_t size) : board_(size),turn_(Square::SquareValue::BLACK) {
    char r = size/2 + 'a';
    int c = size/2 + 1;
    board_(r,c) = Square::SquareValue::BLACK;
    board_((char)(r-1),c-1) = Square::SquareValue::BLACK;
    board_((char)(r-1),c) = Square::SquareValue::WHITE;
    board_(r,c-1) = Square::SquareValue::WHITE;
}

void Reversi::play() {
    string input, temp;
    while(!is_game_over()) {
        prompt();
        std::getline(cin,input);
        if(input == "q") {
            size_t white = 0, black = 0;
            for(size_t i = 0; i < board_.dimension(); i++) {
                for(size_t j = 1; j <= board_.dimension(); j++) {
                    if(board_((char)(i+'a'),j) == Square::WHITE) white++;
                    else if(board_((char)(i+'a'),j) == Square::BLACK) black++;
                }
            }
            win_loss_tie_message(white,black);
            return;
        } else if(input == "c") {
            save_checkpoint();
        } else if(input == "u") {
            undo();
        } else {
            stringstream ss(input);
            ss >> temp;
            if(temp == "p") {
                ss >> temp;
                if (temp.size() == 2 || temp.size() == 3) {
                    char row = temp[0];
                    int col = 0;
                    if (temp.size() == 2) {
                        col = temp[1] - '0';
                    } else {
                        col = (temp[1] - '0') * 10 + (temp[2] - '0');
                    }
                    if (is_legal_choice(row, col, turn_)) {
                        board_(row, col) = turn_;
                        reverse(row,col,turn_);
                        turn_ = opposite_color(turn_);
                    }
                }
            }
        }
    }

}

void Reversi::save_checkpoint() {
    history_.emplace_back(board_,turn_);
}

void Reversi::undo() {
    if(history_.empty()) return;
    board_ = history_.back().board_;
    turn_ = history_.back().turn_;
    history_.pop_back();
}

void Reversi::reverse(char row, size_t col, Square::SquareValue turn) {
// Vectors for each cardinal direction
    const size_t direction_count = 8;
    const int direction_row[] =    {-1, -1,  0, +1, +1, +1,  0, -1};
    const int direction_column[] = { 0, -1, -1, -1,  0, +1, +1, +1};

    // Now check in each directions
    for (size_t d = 0; d < direction_count; d++)
    {
        // Where we're checking
        char cursor_row = row + direction_row[d];
        size_t cursor_column = col + direction_column[d];

        // Move towards the direction while we're on the opposite color
        bool found_opposite = false;
        while (board_.is_legal_and_opposite_color(cursor_row, cursor_column, turn_))
        {
            found_opposite = true;
            cursor_row += direction_row[d];
            cursor_column += direction_column[d];
        }

        // Check if the next thing after is our color
        bool found_same = board_.is_legal_and_same_color(cursor_row, cursor_column, turn_);

        // If this direction is valid, the move is valid, so short circuit and return
        if (found_opposite && found_same) {
            cursor_row = row + direction_row[d];
            cursor_column = col + direction_column[d];
            while (board_.is_legal_and_opposite_color(cursor_row, cursor_column, turn_))
            {
                board_(cursor_row,cursor_column) = turn_;
                cursor_row += direction_row[d];
                cursor_column += direction_column[d];
            }
        }
    }
}


std::ostream& operator<<(std::ostream& out, const Square& square) {
    if(square == Square::WHITE) out << "W";
    else if(square == Square::BLACK) out << "B";
    else out << "-";
    return out;
}