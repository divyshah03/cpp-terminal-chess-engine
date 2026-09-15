#ifndef SUBJECT_H
#define SUBJECT_H
#include "observer.h"
#include "info.h"
#include "state.h"
#include <vector>

class Subject {
    std::vector<Observer*> observers;
    State state;

    public:
    virtual ~Subject() = default;
    void setState(State newS);
    void attach(Observer *ob);
    void notifyObservers();
    virtual Info getInfo() const = 0;
    State getState();

    void detachAll();
};

#endif
