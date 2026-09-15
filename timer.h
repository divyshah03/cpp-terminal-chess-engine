#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>

class Timer {
    std::atomic<int> player1_time;
    std::atomic<int> player2_time;
    std::atomic<bool> player1Turn;
    std::atomic<bool> running;
    std::thread timer_thread;

public:
    std::atomic<bool> waitingForInput = true;
    Timer(int seconds_per_player); 
    ~Timer(); // Destructor

    void start();       // Starts the countdown and listens for input
    void stop();        // Stops the timer
    void printTime();   // Prints current time left for both players
    void switchTurn();  // Switches turn between players

    bool expired() const;          // has either clock run out?
    bool whiteOutOfTime() const;   // player 1 is white
    bool blackOutOfTime() const;
};

#endif
