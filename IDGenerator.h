

#ifndef EX2_IDGENERATOR_H
#define EX2_IDGENERATOR_H

#include <iostream>
#include <set>

class IDGenerator {
public:
    IDGenerator() : nextID_(1) {}

    int generateID() {
        int id;
        if (availableIDs_.empty()) {
            id = nextID_++;
        } else {
            id = *availableIDs_.begin();
            availableIDs_.erase(id);
        }
        return id;
    }

    void releaseID(int id) {
        if (id == nextID_ - 1) {
            nextID_--;
        } else {
            availableIDs_.insert(id);
        }
    }

private:
    int nextID_;
    std::set<int> availableIDs_;
};


#endif //EX2_IDGENERATOR_H
