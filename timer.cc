#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include "timer.h"

using namespace std;


Timer::Timer(int seconds_per_player) :
    player1_time(seconds_per_player), player2_time(seconds_per_player),
    player1Turn(true), running(false) {}

Timer::~Timer() {
    stop();
}

void Timer::start() {
    if (running) return;
    running = true;
    timer_thread = std::thread([this]() {
        while (running) {
            this_thread::sleep_for(std::chrono::seconds(1));
            if (!running) break;

            if (player1Turn && player1_time > 0) --player1_time;
            else if (!player1Turn && player2_time > 0) --player2_time;

            printTime();

            if (player1_time == 0 || player2_time == 0) {
                running = false;
                cout << "\nTime's up!\n" << flush;
            }
        }
    });
}


void Timer::stop() {
    running = false;
    if (timer_thread.joinable()) timer_thread.join();
}


void Timer::switchTurn() {
    player1Turn = !player1Turn;
}

bool Timer::expired() const {
    return (player1_time == 0 || player2_time == 0);
}

bool Timer::whiteOutOfTime() const {
    return (player1_time == 0);
}

bool Timer::blackOutOfTime() const {
    return (player2_time == 0);
}


void Timer::printTime() {
    if (!waitingForInput) return;

    int p1 = player1_time;
    int p2 = player2_time;

    // Save cursor position
    cout << "\033[s";

    // Move cursor to row 2, column 1
    cout << "\033[2;1H";

    // Clear the entire line before printing
    cout << "\033[2K";

    // Print timer
    cout << "Timer -> Player 1: " << player1_time / 60 << ":"
              << (player1_time % 60 < 10 ? "0" : "") << player1_time % 60
              << " | Player 2: " << player2_time / 60 << ":"
              << (player2_time % 60 < 10 ? "0" : "") << player2_time % 60
              << " | " << (player1Turn ? "Player 1's turn" : "Player 2's turn")
              << "     " << flush;

    // Restore cursor
    cout << "\033[u" << flush;
}
