#include <iostream>
#include <string>
#include <vector>
using namespace std;

// max depth for the minimax algorithm
const int MINIMAX_DEPTH = 8;

struct Pos {
    int row, col;
    Pos& operator=(const Pos& p) {
        row = p.row;
        col = p.col;
        return *this;
    }
};

struct State {
    char board[4][4];
    char ai, human, turn;
    Pos x_pos, o_pos;
    State& operator=(State& s) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                board[r][c] = s.board[r][c];
            }
        }
        ai = s.ai;
        human = s.human;
        turn = s.turn;
        x_pos = s.x_pos;
        o_pos = s.o_pos;
        return *this;
    }
};

bool make_move(State& state, char player, Pos move);
char winner(State& state);
int evaluate(State& state);
int max(int a, int b);
int min(int a, int b);
int minimax(State state, int depth, bool maximizing, Pos& move);
Pos ai_move(State state);
Pos start_minimax(State state);
vector<Pos> moves(State& state, char player);
void game_loop(State& state);
void init_board(State& state);
void play(State& state);
void print_board(State& state);

// checks whether a given move is legal for a specified player to make
bool is_legal(State& state, char player, Pos move) {
    bool same_space;
    bool out_of_bounds = move.row < 0 || move.row > 3 ||
                         move.col < 0 || move.col > 3;
    int r, c;
    Pos player_pos;
    if (player == 'x') {
        player_pos = state.x_pos;
    } else {
        player_pos = state.o_pos;
    }
    r = player_pos.row;
    c = player_pos.col;
    same_space = r == move.row && c == move.col;
    // can't go out of bounds or stay still
    if (out_of_bounds || same_space) {
        return false;
    }
    if (r != move.row && c != move.col) {
        // diagonal moves
        // check to make sure move is really on the diagonal
        if (abs(r - move.row) != abs(c - move.col)) {
            return false;
        }
        // check path from current pos to target pos for blocks
        while (r != move.row && c != move.col) {
            if (r > move.row) {
                r--;
            } else if (r < move.row) {
                r++;
            }
            if (c > move.col) {
                c--;
            } else if (c < move.col) {
                c++;
            }
            if (state.board[r][c] != ' ') {
                return false;
            }
        }
    } else if (r != move.row) {
        // vertical moves
        while (r != move.row) {
            if (r > move.row) {
                r--;
            } else if (r < move.row) {
                r++;
            }
            if (state.board[r][c] != ' ') {
                return false;
            }
        }
    } else if (c != move.col) {
        // horizontal moves
        // check path for blocks
        while (c != move.col) {
            if (c > move.col) {
                c--;
            }
            else if (c < move.col) {
                c++;
            }
            if (state.board[r][c] != ' ') {
                return false;
            }
        }
    }
    return true;
}

// executes a move for specified player on given state
bool make_move(State& state, char player, Pos move) {
    Pos player_pos;
    // check legality first
    if (!is_legal(state, player, move)) {
        return false;
    }
    // block current spot and move player to new location
    if (player == 'x') {
        player_pos = state.x_pos;
        state.board[move.row][move.col] = 'x';
        state.board[player_pos.row][player_pos.col] = '.';
        state.x_pos = move;
    } else {
        player_pos = state.o_pos;
        state.board[move.row][move.col] = 'o';
        state.board[player_pos.row][player_pos.col] = '.';
        state.o_pos = move;
    }
    return true;
}

// return winner of game ('n' if game is not over)
char winner(State& state) {
    int x_moves = moves(state, 'x').size();
    int o_moves = moves(state, 'o').size();
    if (x_moves == 0 && state.turn == 'x') {
        return 'o';
    } else if (o_moves == 0 && state.turn == 'o') {
        return 'x';
    } else {
        return 'n';
    }
}

// heuristic function for game state: difference in number of AI moves and human moves
int evaluate(State& state) {
    int ai_moves = moves(state, state.ai).size();
    int human_moves = moves(state, state.human).size();
    return ai_moves - human_moves;
}

// max of two nums
int max(int a, int b) {
    return (a > b) ? a : b;
}

// min of two nums
int min(int a, int b) {
    return (a < b) ? a : b;
}

// implements minimax algorithm to select best course of action, modifies move reference to hold best immediate move
int minimax(State state, int depth, bool maximizing, Pos& move) {
    int eval, maxEval, minEval;
    State temp;
    vector<Pos> movelist;
    // base case, return state evaluation (heuristic)
    if (depth == 0 || winner(state) != 'n') {
        return evaluate(state);
    }
    // AI is maximizing player
    if (maximizing) {
        // set maxEval to very low number (analogous to -infinity)
        maxEval = -1000;
        movelist = moves(state, state.ai);
        // for each child state, run minimax with maximizing = false and lower depth
        for (Pos m : movelist) {
            temp = state;
            make_move(temp, temp.ai, m);
            temp.turn = (temp.turn == 'x') ? 'o' : 'x';
            eval = minimax(temp, depth - 1, false, move);
            if (eval > maxEval) {
                maxEval = eval;
                // update the move reference if it is first level and move is better than best seen
                if (depth == MINIMAX_DEPTH) {
                    move = m;
                }
            }
        }
        return maxEval;
    } else {
        minEval = 1000;
        movelist = moves(state, state.human);
        // for each child state, run minimax with maximizing = true and lower depth
        for (Pos m : movelist) {
            temp = state;
            make_move(temp, temp.human, m);
            temp.turn = (temp.turn == 'x') ? 'o' : 'x';
            eval = minimax(temp, depth - 1, true, move);
            if (eval < minEval) {
                minEval = eval;
            }
        }
        return minEval;
    }
}

// use minimax to choose a move
Pos ai_move(State state) {
    Pos move = start_minimax(state);
    return move;
}

// intiate minimax
Pos start_minimax(State state) {
    Pos move;
    int m = minimax(state, MINIMAX_DEPTH, true, move);
    return move;
}

// list all possible moves for player in state
vector<Pos> moves(State& state, char player) {
    vector<Pos> list;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (is_legal(state, player, {r, c})) {
                list.push_back({r, c});
            }
        }
    }
    return list;
}

// play the game
void game_loop(State& state) {
    while (winner(state) == 'n') {
        Pos move;
        print_board(state);
        if (state.turn == state.human) {
            string target;
            do {
                cout << state.turn << " enter your move (eg c3): ";
                cin >> target;
                move.row = target[1] - '0' - 1;
                move.col = target[0] - 'a';
            } while (!make_move(state, state.human, move));
        } else {
            do {
                move = ai_move(state);
            } while (!make_move(state, state.ai, move));
            printf("AI moved to %c%d\n", move.col+'a', move.row+1);
        }
        state.turn = (state.turn == 'x') ? 'o' : 'x';
    }
    print_board(state);
    cout << "Game over: " << winner(state) << " wins!\n";
}

// initialize the board and players
void init_board(State& state) {
    char human_choice, first;
    cout << "Choose your side (enter 'x' or 'o'): ";
    cin >> human_choice;
    cout << "Which side will go first (enter 'x' or 'o'): ";
    cin >> first;
    // set player sides + first turn
    state.human = human_choice;
    state.ai = (human_choice == 'x') ? 'o' : 'x';
    state.turn = first;
    // fill board with spaces
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            state.board[r][c] = ' ';
        }
    }
    // place x and o
    state.board[0][0] = 'x';
    state.board[3][3] = 'o';
    state.x_pos = {0, 0};
    state.o_pos = {3, 3};
}

void play(State& state) {
    char again;
    do {
        init_board(state);
        game_loop(state);
        cout << "Would you like to play again? (y or n): ";
        cin >> again;
    } while (again == 'y');
}

// print the board
void print_board(State& state) {
    cout << "  ";
    for (int c = 0; c < 4; c++) {
        cout << (char)(c+'a');
        if (c < 3) {
            cout << " ";
        }
    }
    cout << endl;
    for (int r = 0; r < 4; r++) {
        cout << r+1 << " ";
        for (int c = 0; c < 4; c++) {
            if (state.board[r][c] == ' ') {
                cout << "_";
            } else {
                cout << state.board[r][c];
            }
            if (c < 3) {
                cout << " ";
            }
        }
        cout << endl;
    }
    cout << endl;
}