# ♟️ Chess Game

A terminal based chess game written in modern C++20. Play a friend, play the computer, set up any position you like, take back moves, and put a clock on the game. An X11 board window is available as an optional build. 🎉

---

## 🔧 Features

- ♜ Fully functional chess engine with move validation and king safety
- 🖨️ Text based board display, plus an optional graphical display (via X11)
- 📜 Standard chess rules: check, checkmate, stalemate, en passant, castling, and promotion
- 🛠️ Command-line setup mode for custom board configuration
- ↩️ Undo, to take back the last move
- ⏱️ Optional countdown timer for each player, started with `./chess -bonus`; a player who runs out of time loses
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

---

## 🕹️ Gameplay Commands

| Command | What it does |
|---|---|
| `game human human` | 🧑‍🤝‍🧑 Start a game between two human players |
| `game human computer2` | 🤖 Start a game against the computer (levels 1–4) |
| `move e2 e4` | ♟️ Move a piece from e2 to e4 |
| `move e7 e8 Q` | 👑 Move and choose a promotion piece (`Q`, `R`, `B` or `N`; queen if left out) |
| `undo` | ↩️ Take back the last move |
| `resign` | 🏳️ Resign the game |
| `setup` | 🛠️ Enter setup mode to customize the board |
| `help` | 🆘 Show the commands available |
| `Ctrl + D` | 📊 Print the session score and quit |

🤖 When you play against the computer it moves by itself, so you only enter your own
moves. In a game between two computers, each `move` command plays one turn.
Against the computer, `undo` takes back both its reply and your move, so the turn
comes back to you.

### 🎬 A session to get you started

```
game human computer2
move e2 e4
move d2 d4
undo
```

---

## 🤖 Computer Levels

The computer players use handwritten heuristics, no external engine:

- 🎲 `computer1` — plays a random legal move
- ⚔️ `computer2` — prefers captures of valuable pieces, promotions and checks
- 🛡️ `computer3` — also avoids moving onto squares the opponent attacks
- 🧠 `computer4` — also saves pieces that are already under attack and limits the opponent's replies

---

## 🛠️ Setup Mode Commands

| Command | What it does |
|---|---|
| `+ K e1` | ➕ Add a white king to e1 (uppercase is white, lowercase is black) |
| `- d7` | ➖ Remove the piece on d7 |
| `= white` | ⚪ Set the current turn to white |
| `= black` | ⚫ Set the current turn to black |
| `done` | ✔️ Leave setup mode |

⚠️ A setup is accepted only when there is exactly one king of each colour, no pawns
on the first or last row, and neither king is already under attack.

---

## 🗂️ Project Layout

| Area | Files |
|---|---|
| 🚪 Entry point and command loop | `main.cc` |
| 🎯 Match and session state | `game.h`, `game.cc` |
| 📐 Rules, move generation, board state | `board`, `cell`, `piece`, `move`, `position` |
| 🕹️ Players | `player`, `humanPlayer`, `computerPlayer` |
| 📡 Observer plumbing | `subject`, `observer`, `state`, `info` |
| 🖼️ Displays | `textDisplay`, `graphicsDisplay`, `window` |
| ⏱️ Timer | `timer` |

---

## 🧑‍💻 Authors

This project was built by Divy, Kshaman and Siddh. 🙌
