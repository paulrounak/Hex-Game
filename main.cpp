#include <bits/stdc++.h>

using namespace std;

enum class Player
{
    WHITE,
    BLACK
};

void clearScreen()
{
    cout << "\033[2J\033[1;1H";
}

class Board
{
private:
    static int direction[6][2];
    static char white;
    static char black;
    static char empty;
    int size{};
    string line;
    vector<vector<char>> board;

public:
    Board() = default;
    explicit Board(int size) : size(size), board(size, vector<char>(size, '+'))
    {
        line = "\\";
        for (int i = 1; i < size; i++)
            line += " / \\";
    }
    bool place(int x, int y, Player p);
    bool badMove(int x, int y);
    vector<pair<int, int>> getEmpty();
    bool win(int x, int y);
    Player winner();
    void printBoard();

private:
    [[nodiscard]] bool inBoard(int x, int y) const;
    void borders(int x, int y, vector<bool> &condition, char side) const;
    void bfsSearch(vector<pair<int, int>> &start, vector<bool> &condition);
};

class AI
{
public:
    static double getWins(Board &board, Player p);
    virtual pair<int, int> next(Board &board, Player p);
};

class Game
{
private:
    Player computer;
    Player player;
    AI robotAI;
    Board board;

public:
    Game() = default;
    explicit Game(AI &robotAI) : robotAI(robotAI) {}
    void play();

private:
    void setup();
    void choose();
    bool computerTurn();
    bool playerTurn();
};

int Board::direction[6][2] =
        {
                {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}};

char Board::white = 'W';
char Board::black = 'B';
char Board::empty = '+';

int main()
{
    AI gameAI;
    Game hexGame(gameAI);
    srand(time(nullptr));
    hexGame.play();
    return 0;
}

bool Board::inBoard(int x, int y) const
{
    return (x >= 0 && x < size && y >= 0 && y < size);
}

void Board::borders(int x, int y, vector<bool> &condition, char side) const
{
    if (side == black)
    {
        if (y == 0)
            condition[0] = true;
        if (y == size - 1)
            condition[1] = true;
    }
    else
    {
        if (x == 0)
            condition[0] = true;
        if (x == size - 1)
            condition[1] = true;
    }
}

// assigns cell block to B or W
bool Board::place(int x, int y, Player p)
{
    if (inBoard(x, y) && board[x][y] == empty)
    {
        if (p == Player::BLACK)
            board[x][y] = black;
        else
            board[x][y] = white;
        return true;
    }
    return false;
}

bool Board::badMove(int x, int y)
{
    if (inBoard(x, y))
    {
        board[x][y] = empty;
        return true;
    }
    return false;
}

// returns all the empty spots on the board, useful for monte carlo
vector<pair<int, int>> Board::getEmpty()
{
    vector<pair<int, int>> blankSpots;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
            if (board[i][j] == empty)
                blankSpots.emplace_back(i, j);
    }
    return blankSpots;
}
// Breadth-first search and checks borders() win conditionals
void Board::bfsSearch(vector<pair<int, int>> &start, vector<bool> &condition)
{
    if (!start.empty())
    {
        int x = start[0].first;
        int y = start[0].second;
        char side = board[x][y];

        vector<vector<bool>> visited(size, vector<bool>(size));
        queue<pair<int, int>> trace;

        for (const auto & itr : start)
        {
            trace.push(itr);
            visited[itr.first][itr.second] = true;
        }
        while (!(trace.empty()))
        {
            auto top = trace.front();
            borders(top.first, top.second, condition, side);
            trace.pop();

            for (auto & i : direction)
            {
                int xCursor = top.first + i[0];
                int yCursor = top.second + i[1];
                if (inBoard(xCursor, yCursor) && board[xCursor][yCursor] == side && !visited[xCursor][yCursor])
                {
                    visited[xCursor][yCursor] = true;
                    trace.emplace(xCursor, yCursor);
                }
            }
        }
    }
}

// BFS last registered move (x,y) returns true if path made
bool Board::win(int x, int y)
{
    if (inBoard(x, y) && board[x][y] != empty)
    {

        vector<bool> condition(2, false); // two opposite ends
        vector<pair<int, int>> start(1, make_pair(x, y));

        bfsSearch(start, condition);
        return condition[0] && condition[1];
    }
    return false;
}

// BFS to check if black made a path from left to right/has a path
Player Board::winner()
{
    vector<bool> condition(2, false);
    vector<pair<int, int>> start;
    for (int i = 0; i < size; i++)
        if (board[i][0] == black)
            start.push_back(make_pair(i, 0));

    bfsSearch(start, condition);
    return (condition[0] && condition[1]) ? Player::BLACK : Player::WHITE;
}

void Board::printBoard()
{
    if (size <= 0)
        return;

    cout << "  0";
    for (int i = 1; i < size; i++)
        cout << " w " << i;
    cout << endl;

    cout << "0 " << board[0][0];
    for (int i = 1; i < size; i++)
        cout << "---" << board[0][i];
    cout << endl;

    string indent;
    for (int i = 1; i < size; i++)
    {
        indent += ' ';
        cout << indent << "b " << line << endl;
        if (i < 10)
        {
            indent += ' ';
            cout << indent << i << ' ' << board[i][0];
        }
        else
        {
            cout << indent << i << ' ' << board[i][0];
            indent += ' ';
        }

        for (int j = 1; j < size; j++)
            cout << "---" << board[i][j];
        cout << endl;
    }
}

void Game::play()
{
    while (true)
    {
        setup();
        choose();
        char userIn;
        bool counter = false;

        int turn = (computer == Player::BLACK ? 0 : 1);
        while (!counter)
        {
            turn = !turn;
            if (turn)
                counter = computerTurn();
            else
                counter = playerTurn();
        }

        if (turn == 1)
        {
            cout << "I win" << endl;
            cout << "Want to play again? (y/n) ";
            cin >> userIn;
        }
        else
        {
            cout << "You win!" << endl;
            cout << "Want to play again? (y/n) ";
            cin >> userIn;
        }

        if (userIn != 'y' && userIn != 'Y')
            break;
        cin.clear();
    }
    cout << "Let's play again sometime!" << endl;
}

void Game::setup()
{
    string border(25, '+');

    int dimensions;
    cout << "Enter board size: ";
    cin >> dimensions;

    if (dimensions > 0)
    {
        cin.clear();
        board = Board(dimensions);
    }
    else
    {
        cout << "Invalid input, I'll pick 9.\n";
        board = Board(9);
    }
    board.printBoard();
}

void Game::choose()
{
    char side = 'w';

    cout << "Pick a side (b/w): ";
    cin >> side;
    if (side == 'b' || side == 'B')
    {
        player = Player::BLACK;
        computer = Player::WHITE;
    }
    else if (side == 'w' || side == 'W')
    {
        player = Player::WHITE;
        computer = Player::BLACK;
    }
    else
    {
        cout << "Invalid side\n";
        cout << "I'll be the black side" << endl
             << "Thinking..." << endl;
        player = Player::WHITE;
        computer = Player::BLACK;
    }
    cin.clear();
}

// returns true if AI won otherwise AI turn handler
bool Game::computerTurn()
{
    clearScreen();
    cout << "My turn! I move: ";
    auto move = robotAI.next(board, computer); // calculate optimal move
    board.place(move.first, move.second, computer);
    cout << move.first << " " << move.second << endl;
    board.printBoard();
    sleep(1);
    return board.win(move.first, move.second);
}

bool Game::playerTurn()
{
    clearScreen();
    int x, y;
    cout << "Your turn!" << endl;
    board.printBoard();
    while (true)
    {
        cout << "Where are you putting your piece? (y x = ) ";
        cin >> x >> y;
        if (board.place(x, y, player))
            break;
        cout << "You can't do that!" << endl;
    }
    clearScreen();
    cout<<"Thinking..."<<endl;
    board.printBoard();
    return board.win(x, y);
}

double AI::getWins(Board &board, Player player)
{
    auto blank = board.getEmpty();
    int winCount = 0;
    vector<int> perm(blank.size());
    for (int i = 0; i < perm.size(); i++)
        perm[i] = i;
    for (int n = 0; n < 1000; n++)
    {
        int turn = (player == Player::BLACK ? 0 : 1);
        for (int i = static_cast<int>(perm.size()); i > 1; i--)
        {
            int swap = rand() % i;
            int temp = perm[i - 1];
            perm[i - 1] = perm[swap];
            perm[swap] = temp; // rand the permutation
        }
        for (int i : perm)
        {
            turn = !turn; // bool turn tracking
            int x = blank[i].first;
            int y = blank[i].second;
            if (turn)
            {
                board.place(x, y, Player::WHITE);
            }
            else
            {
                board.place(x, y, Player::BLACK);
            }
        }
        if (board.winner() == player)
            winCount++;

        for (auto & itr : blank)
            board.badMove(itr.first, itr.second); // take back rand moves
    }
    return static_cast<double>(winCount) / 1000;
}

//Monte-Carlo Simulation
pair<int, int> AI::next(Board &board, Player p)
{
    auto blank = board.getEmpty();
    double bestMove = 0;
    pair<int, int> move = blank[0];

    for (auto & i : blank)
    {
        int x = i.first;
        int y = i.second;
        board.place(x, y, p);

        double moveValue = getWins(board, p);
        if (moveValue > bestMove)
        {
            move = i;
            bestMove = moveValue;
        }

        board.badMove(x, y);
    }
    return move;
}
