#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include "game.h"
#include "timer.h"

using namespace std;

void printConfig(const vector<vector<char>> &config) {
    for (int row = 7; row >= 0; --row) {
        cout << row + 1 << "  ";
        for (int col = 0; col < 8; ++col) {
            cout << config[row][col] << " ";
        }
        cout << endl;
    }
    cout << "\n   a b c d e f g h\n" << endl;
}

// Throws away the remainder of the current input line, so that extra words are
// not picked up as the next command
static void discardRestOfLine() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static string toLower(string text) {
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c){ return tolower(c); });
    return text;
}

// Accepts -bonus, -enablebonus and -enableBonus
static bool isBonusFlag(const string &arg) {
    string flag = toLower(arg);
    return (flag == "-bonus" || flag == "-enablebonus");
}

static bool isValidPlayerName(const string &name) {
    return (name == "human" || name == "computer1" || name == "computer2" ||
            name == "computer3" || name == "computer4");
}

// Prints the session score and the overall winner, used whenever the user quits
static void printSessionScore(Game &game) {
    cout << "Final score:" << endl;
    cout << "White: " << game.getWhiteWins() << endl;
    cout << "Black: " << game.getBlackWins() << endl;
    if (game.getWhiteWins() > game.getBlackWins()) {
        cout << "White wins the session!" << endl;
    } else if (game.getWhiteWins() < game.getBlackWins()) {
        cout << "Black wins the session!" << endl;
    } else {
        cout << "Session is a draw!" << endl;
    }
}

// Runs the checks that follow a completed move. Returns true when the game is over.
// The timer pointer is null unless the bonus features are enabled.
static bool finishTurn(Game &game, Timer *timer) {
    if (timer) {
        timer->switchTurn();
        timer->printTime();
    }

    if (game.getBoard()->isCheckmate()) {
        if (timer) timer->stop();
        if (game.getCurrentTurn()->getColour() == Colour::WHITE) {
            cout << "Checkmate! Black wins!" << endl;
            cout << "Better luck next time: white! Don't worry, every grandmaster was once a beginner!" << endl;
            game.incrementBlackWins(1);
        } else {
            cout << "Checkmate! White wins!" << endl;
            cout << "Better luck next time: black! Don't worry, every grandmaster was once a beginner!" << endl;
            game.incrementWhiteWins(1);
        }
        return true;
    }

    if (game.getBoard()->isStalemate()) {
        if (timer) timer->stop();
        cout << "Stalemate!" << endl;
        cout << "It's a draw!" << endl;
        game.incrementWhiteWins(0.5);
        game.incrementBlackWins(0.5);
        return true;
    }

    if (game.getBoard()->isCheck()) {
        if (game.getCurrentTurn()->getColour() == Colour::WHITE) {
            cout << "Check! White is in check!" << endl;
        } else {
            cout << "Check! Black is in check!" << endl;
        }
    }

    // A player who runs out of time loses the game
    if (timer && timer->expired()) {
        timer->stop();
        if (timer->whiteOutOfTime()) {
            cout << "White ran out of time. Black wins!" << endl;
            game.incrementBlackWins(1);
        } else {
            cout << "Black ran out of time. White wins!" << endl;
            game.incrementWhiteWins(1);
        }
        return true;
    }

    return false;
}

int main(int argc, char* argv[]){
    bool enableBonus = false;

    for (int i = 1; i < argc; ++i) { // start at 1 to skip the program name
        if (isBonusFlag(argv[i])) {
            enableBonus = true;
        }
    }

    if (enableBonus) {
        std::cout << "Bonus features enabled.\n";
    }
    
    Game game;
    Colour colour = Colour::WHITE;
    unique_ptr<Timer> timer = nullptr;

    cout << endl;
    cout << "Welcome to the Chess Game!" << endl;
    cout << "Please enter a command to begin." << endl;
    cout << "You can start a new game or set up the board." << endl;
    cout << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "To start a game, type:" << endl;
    cout << "  game <white_player> <black_player> where <white_player> and <black_player> can be:" << endl;
    cout << "  - human" << endl;
    cout << "  - computer1 (Beginner)" << endl;
    cout << "  - computer2 (Intermediate)" << endl;
    cout << "  - computer3 (Advanced)" << endl;
    cout << "  - computer4 (Expert)" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "To enter setup mode, type:" << endl;
    cout << "  setup" << endl;
    cout << "To see the help menu, type:" << endl;
    cout << "  help" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "To see the current score: Ctrl + D" << endl;
    cout << "--------------------------------------------------" << endl;
  
    while(true){
        string cmd;
        cout << endl;
        cout << "Please enter a command." << endl;
        cin >> cmd;
        if (cin.eof()) {
            printSessionScore(game);
            if(enableBonus && timer) timer->stop();
            cin.clear();
            return 0;
        }

        // Command handling 
        if (cmd == "game") {
            string whitePlayer, blackPlayer;
            cin >> whitePlayer >> blackPlayer;

            if (cin.fail()) {
                cout << "Oops! Input failed" << endl;
                cin.clear();
                continue;
            }

            if (isValidPlayerName(whitePlayer) && isValidPlayerName(blackPlayer)) {
                cout << endl;
                game.start(whitePlayer, blackPlayer, colour);
            } else {
                cout << "Invalid Command, try naming the players correctly" << endl;
                continue;
            }

            if(enableBonus){
                cout << "--------------------------------------------------" << endl;
                cout << "How intense do you want this battle to be?" << endl;
                cout << "Set the time limit for each player (in seconds):" << endl;
                cout << "  (Tip: Blitz = 60, Rapid = 600, Classical = 1800+)" << endl;
                cout << "--------------------------------------------------" << endl;
                int time_limit = 0;
                cin >> time_limit;
                if (cin.fail() || time_limit <= 0) {
                    cin.clear();
                    string discard;
                    getline(cin, discard);
                    time_limit = 600;
                    cout << "Using the default of " << time_limit << " seconds per player." << endl;
                }
                timer = make_unique<Timer>(time_limit);
            }

            cout << "Game starts: Let's play!" << endl;
            cout << "--------------------------------------------------" << endl;
            cout << endl;

            if(colour == Colour::WHITE){
                cout << "Let's start with the white player!" << endl;
                cout << endl;
            } else {
                cout << "Let's start with the black player!" << endl;
                cout << endl;
            }

            // A custom setup can already be finished before anybody moves
            if (game.getBoard()->isCheckmate()) {
                cout << "This position is already checkmate!" << endl;
                if (game.getCurrentTurn()->getColour() == Colour::WHITE) {
                    game.incrementBlackWins(1);
                } else {
                    game.incrementWhiteWins(1);
                }
                continue;
            }
            if (game.getBoard()->isStalemate()) {
                cout << "This position is already a stalemate!" << endl;
                game.incrementWhiteWins(0.5);
                game.incrementBlackWins(0.5);
                continue;
            }

            // Start the timer
            if(enableBonus && timer) timer->start();

            Timer *timerPtr = enableBonus ? timer.get() : nullptr;

            // When a person is playing, the computer moves on its own. In a game
            // between two computers each move is still requested with "move".
            bool humanInGame = game.getWhitePlayer()->usesTerminalInput() ||
                               game.getBlackPlayer()->usesTerminalInput();

            // Game loop
            while(true){
                if (humanInGame && !game.getCurrentTurn()->usesTerminalInput()) {
                    cout << "The computer is thinking..." << endl;
                    if (!game.gameMove()) {
                        cout << "The computer has no legal move to play." << endl;
                        break;
                    }
                    if (finishTurn(game, timerPtr)) break;
                    continue;
                }

                cout << "It is " << (game.getCurrentTurn()->getColour() == Colour::WHITE ? "white" : "black")
                     << "'s turn." << endl;
                cout << "Enter a command: " << endl;
                cout << "Your choices are: " << endl;
                cout << "move <from> <to> [promotion piece]" << endl;
                cout << "undo" << endl;
                cout << "resign" << endl;
                if (!humanInGame) cout << "(type move to let the computer play its turn)" << endl;
                cout << "To see the current score: Ctrl + D" << endl;
                cout << "--------------------------------------------------" << endl;

                string game_cmd;
                if (enableBonus && timer) timer->waitingForInput = true;
                cin >> game_cmd;
                if (enableBonus && timer) timer->waitingForInput = false;

                if (cin.eof()) {
                    printSessionScore(game);
                    if(enableBonus && timer) timer->stop();
                    cin.clear();
                    return 0;
                }

                // Handling move command here.
                if (game_cmd == "move"){
                    // A computer picks its own move, so drop anything typed after
                    // the command instead of reading it as the next command
                    if (!game.getCurrentTurn()->usesTerminalInput()) discardRestOfLine();

                    if(game.gameMove()){
                        if (finishTurn(game, timerPtr)) break;
                    } else {
                        cout << "Invalid move, try again" << endl;
                        continue;
                    }  
                } else if (game_cmd == "undo"){
                    discardRestOfLine();
                    if(game.undoMove()){
                        if(timerPtr) timerPtr->switchTurn();

                        // Against the computer, take back its reply as well so that
                        // the move comes back to the person playing
                        if(humanInGame && !game.getCurrentTurn()->usesTerminalInput() && game.undoMove()){
                            if(timerPtr) timerPtr->switchTurn();
                        }
                        cout << "Last move taken back." << endl;
                    } else {
                        cout << "There is no move to undo." << endl;
                    }
                    continue;
                } else if (game_cmd == "resign"){
                    discardRestOfLine();
                    if(enableBonus && timer) timer->stop();
                    if(game.getCurrentTurn()->getColour() == Colour::WHITE){
                        cout << "Game over!" << endl;
                        cout << "Black wins!" << endl;
                        cout << "Better luck next time: white! Don't worry, every grandmaster was once a beginner!" << endl;
                        game.incrementBlackWins(1);
                        break;
                    } else {
                        cout << "Game over!" << endl;
                        cout << "White wins!" << endl;
                        cout << "Better luck next time: black! Don't worry, every grandmaster was once a beginner!" << endl;
                        game.incrementWhiteWins(1);
                        break;
                    }
                } else {
                    discardRestOfLine();
                    cout << "Invalid Command, try again" << endl;
                    continue;
                }
            } // while loop for game commands

            if(enableBonus && timer) timer->stop();
        } else if (cmd == "setup") {

            cout << "--------------------------------------------------" << endl;
            cout << "Please enter a setup command: " << endl; 
            cout << "Your choices are: " << endl;
            cout << "+ <piece> <position>" << endl;
            cout << "- <position>" << endl;
            cout << "= <colour>" << endl;
            cout << "done" << endl;
            cout << "To see the current score: Ctrl + D" << endl;
            cout << "--------------------------------------------------" << endl;

            while (true) {
                string setup_cmd;
                cin >> setup_cmd;

                if (cin.eof()) {
                    printSessionScore(game);
                    if(enableBonus && timer) timer->stop();
                    cin.clear();
                    return 0;
                }

                // Handling + setup command here
                if (setup_cmd == "+") {
                    char piece;
                    string position;
                    cin >> piece >> position;

                    if (cin.fail()) {
                        cout << "Oops! Input failed" << endl;
                        cout << "Please write a valid command" << endl;
                        cin.clear();
                        continue;
                    }

                    if (piece == 'P' || piece == 'R' || piece == 'N' || piece == 'B' || piece == 'Q' || piece == 'K' ||
                        piece == 'p' || piece == 'r' || piece == 'n' || piece == 'b' || piece == 'q' || piece == 'k') {
                        if (position.length() == 2 && position[0] >= 'a' && position[0] <= 'h' && position[1] >= '1' && position[1] <= '8') {
                            char colChar = position[0];
                            int rowInt = position[1] - '0';
                            Position pos(rowInt, colChar);

                            int rowIndex = pos.getRowVector();
                            int colIndex = pos.getColVector();

                            game.config[rowIndex][colIndex] = piece;
                            printConfig(game.config);
                            cout << "Piece " << piece << " placed at " << position << endl;
                        } else {
                            cout << "Invalid Command, position is not valid" << endl;
                        }
                    } else {
                        cout << "Invalid Command, piece is not valid" << endl;
                        continue;
                    }
                } else if (setup_cmd == "-"){
                    string position;
                    cin >> position;

                    if (cin.fail()) {
                        cout << "Oops! Input failed" << endl;
                        cout << "Please write a valid command" << endl;
                        cin.clear();
                        continue;
                    }

                    if(position.length() == 2 && position[0] >= 'a' && position[0] <= 'h' && position[1] >= '1' && position[1] <= '8'){
                        char colChar = position[0];
                        int rowInt = position[1] - '0';
                        Position pos(rowInt, colChar);

                        int rowIndex = pos.getRowVector();
                        int colIndex = pos.getColVector();

                        if((rowIndex + colIndex) % 2 == 0){
                            game.config[rowIndex][colIndex] = '_';
                        } else {
                            game.config[rowIndex][colIndex] = ' ';
                        }
                        printConfig(game.config);
                        cout << "Piece removed from " << position << endl;
                    } else {
                        cout << "Invalid Command, position is not valid" << endl;
                    }
                } else if (setup_cmd == "="){
                    string desired_colour;
                    cin >> desired_colour;
                    if (cin.fail()) {
                        cout << "Oops! Input failed" << endl;
                        cout << "Please write a valid command" << endl;
                        cin.clear();
                        continue;
                    }

                    if (desired_colour == "white" || desired_colour == "black"){
                        if(desired_colour == "white"){
                            colour = Colour::WHITE;
                        } else {
                            colour = Colour::BLACK;
                        }
                        cout << "Current turn set to " << desired_colour << endl;
                    } else {
                        cout << "Invalid Command, colour is not valid" << endl;
                    }
                } else if (setup_cmd == "done"){
                    if(game.isSetupValid()){
                        cout << "Setup complete!" << endl;
                        cout << "--------------------------------------------------" << endl;
                        cout << "You can now start the game, or enter setup mode again if you wish." << endl;
                        break;   
                    } else {
                        cout << "Invalid setup. Make sure you have exactly one white king, one black king, no pawns on first or last row and no king in check." << endl;
                        continue;
                    }
                } else {
                    cout << "Invalid Command, try again" << endl;
                    continue;
                }
            } // while loop for setup

            // help command
        } else if (cmd == "help") {
            cout << "--------------------------------------------------" << endl;
            cout << "Help - Chess Game Commands" << endl;
            cout << "--------------------------------------------------" << endl;
            cout << "To start a new game:" << endl;
            cout << "  game <white_player> <black_player>" << endl;
            cout << "    <white_player> and <black_player> can be:" << endl;
            cout << "      - human" << endl;
            cout << "      - computer1 (Beginner)" << endl;
            cout << "      - computer2 (Intermediate)" << endl;
            cout << "      - computer3 (Advanced)" << endl;
            cout << "      - computer4 (Expert)" << endl;
            cout << endl;
            cout << "To enter setup mode (customize the board):" << endl;
            cout << "  setup" << endl;
            cout << "  In setup mode, you can use:" << endl;
            cout << "    + <piece> <position>   (add a piece, e.g., + K e1)" << endl;
            cout << "    - <position>           (remove a piece, e.g., - e1)" << endl;
            cout << "    = <colour>             (set whose turn, e.g., = white)" << endl;
            cout << "    done                   (finish setup mode)" << endl;
            cout << endl;
            cout << "During a game, you can use:" << endl;
            cout << "  move <from> <to>         (move a piece, e.g., move e2 e4)" << endl;
            cout << "  move <from> <to> <piece> (promote a pawn, e.g., move e7 e8 Q)" << endl;
            cout << "  undo                     (take back the last move)" << endl;
            cout << "  resign                   (concede the game)" << endl;
            cout << endl;
            cout << "At any time, you can use:" << endl;
            cout << "  help                     (to get help)" << endl;
            cout << "--------------------------------------------------" << endl;
            continue;
        } else {
            cout << "Invalid Command, try again" << endl;
            continue;
        }
    } // while loop
    return 0;
} // main
