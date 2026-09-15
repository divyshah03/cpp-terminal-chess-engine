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

## ♟️ Supported Chess Mechanics

Our chess engine implements the rules of standard chess, including:

- ⛳️ **Check and Checkmate Detection**  
  The game detects check and checkmate, and refuses any move that would leave your own king in check. Pinned pieces cannot move out of the pin.

- 🤝 **Stalemate Handling**  
  If a player has no legal moves but is not in check, the game correctly declares a stalemate (draw).

- 🏰 **Castling**  
  Both kingside and queenside castling are supported, provided all rules are satisfied:
  - Neither the king nor the rook involved has moved.
  - The squares between them are empty.
  - The king is not in check, and does not pass through or land on a square under attack.

- 👻 **En Passant**  
  En passant captures are supported immediately after a pawn moves two squares forward from its starting rank and lands beside an enemy pawn.

- 👑 **Pawn Promotion**  
  When a pawn reaches the opposite end of the board, it is promoted to a queen, rook, bishop, or knight.

### ✅ Verified Move Generation

The move generator is checked against the published move counts (perft) for the
standard test positions:

| Position | Depth 1 | Depth 2 | Depth 3 | Depth 4 |
|---|---|---|---|---|
| 🏁 Starting position | 20 | 400 | 8,902 | 197,281 |
| 🧨 "Kiwipete" (castling and en passant stress test) | 48 | 2,039 | 97,862 | — |

- `+ K e1` — Add a white king to square e1  
- `- d7` — Remove a piece from square d7  
- `= white` — Set the current turn to white  
- `= black` — Set the current turn to black  
- `done` — Exit setup mode and start the game  


## 🧑‍💻 Authors

This project was built by Divy, Kshaman and Siddh.

