# ♟️ Chess Game

A terminal based chess game written in modern C++20. Play a friend, play the computer, set up any position you like, take back moves, and put a clock on the game. An X11 board window is available as an optional build. 🎉

---

## 🔧 Features

- ♜ Fully functional chess engine with move validation and king safety
- 🖨️ Text based board display, plus an optional graphical display (via X11)
- 📜 Standard chess rules: check, checkmate, stalemate, en passant, castling, and promotion
- 🛠️ Command-line setup mode for custom board configuration
- ↩️ Undo, to take back the last move
- ⏱️ Optional timer for each player, with any time control you like
- 🧑‍🤝‍🧑 Human vs Human, and 🤖 Human vs Computer with four difficulty levels
- 🧠 Uses smart pointers everywhere possible

---

## 🖥️ How to Build

### 📦 Requirements

- Any C++20 compiler (`g++` 11 or later, or `clang++`)
- Linux, macOS or WSL (Windows Subsystem for Linux)
- X11 development libraries, only for the optional graphical display

### 🔨 Build

| Command | What it does |
|---|---|
| `make` | 🏗️ Compiles the game and produces `./chess` |
| `make clean` | 🧹 Removes the compiled files |
| `make CXX=clang++` | 🔁 Builds with a different compiler |
| `make GRAPHICS=1` | 🪟 Also builds the X11 board window (needs the X11 libraries) |

---

### ♟️ Supported Chess Mechanics

Our chess engine fully implements the rules of standard chess, including:

- ⛳️ **Check and Checkmate Detection**  
  The game automatically checks for check and checkmate and prevents illegal moves that would leave a king in check.

- ⛳️ **Stalemate Handling**  
  If a player has no legal moves but is not in check, the game correctly declares a stalemate (draw).

- ⛳️ **Castling**  
  Both kingside and queenside castling are supported, provided all rules are satisfied:
  - Neither the king nor the rook involved has moved.
  - The squares between them are empty.
  - The king is not in check, and does not pass through or land on a square under attack.

- ⛳️ **En Passant**  
  En passant captures are supported immediately after a pawn moves two squares forward from its starting rank and lands beside an enemy pawn.

- ⛳️ **Pawn Promotion**  
  When a pawn reaches the opposite end of the board, it can be promoted to a queen, rook, bishop, or knight.

### 🕹️ Gameplay Commands

- `game white human black human` — Start a game between two human players  
- `game white human black computer[1-4]` — Start a game against AI of level 1–4  
- `move e2 e4` — Move a piece from e2 to e4  
- `resign` — Resign the game  
- `setup` — Enter setup mode to customize the board  
- `help` - Gives the player help on the commands available

### 🔧 Setup Mode Commands

- `+ K e1` — Add a white king to square e1  
- `- d7` — Remove a piece from square d7  
- `= white` — Set the current turn to white  
- `= black` — Set the current turn to black  
- `done` — Exit setup mode and start the game  


## 🧑‍💻 Authors

This project was built by Divy, Kshaman and Siddh.

