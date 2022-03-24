#include "Knob.hpp"
#include "main.h"
#include <algorithm>
#include <map>

std::map<volatile uint8_t*, int> paramMax { {&octave, 8}, {&volume, 7}, {&sound, 4}, {&reverb, 7} };

Knob::Knob() {
    target = nullptr;
}

// target refers to the "global" parameter that the knob would be tuning
Knob::Knob(volatile uint8_t* _target) {
    target = _target;
}

void Knob::setTarget(volatile uint8_t* _target) {
    target = _target;
    return;
}

void Knob::update(uint8_t oldState, uint8_t newState) {
    // change according to state
    int8_t delta = change(oldState, newState);
    // if no change OR knob not map to a target, no need to udpate
    if (!delta || target==nullptr) {
        return;
    }

    // update parameters
    int param = int(__atomic_load_n(target,__ATOMIC_RELAXED)) + delta;
    param = std::min(std::max(param, 0), (paramMax[target]));
    __atomic_store_n(target,param,__ATOMIC_RELAXED);

    // only for when target==octave (better way to handle this?)
    if (target != &octave) return;

    //Reset notes played (This is to make sure that no note in the notesPlayed array will never
    // turn off if we change the octave while a note is being played)
    if(delta != 0){
        for(int i=0; i<9*12; i++){
             __atomic_store_n(&playedNotes[i],0,__ATOMIC_RELAXED);
        }
    }
} 

// Check whether any of the keys have been turned
// We only care about transitions from states 00 or 11, to states 01 or 10.
// 0 unchange
int8_t Knob::change(uint8_t oldState, uint8_t newState) {
    switch (oldState) {
        case 0:
                if (newState == 1) {
                    return -1;
                } else if (newState == 2) {
                    return 1;
                }
                break;
        case 3:
                if (newState == 1) {
                    return 1;
                } else if (newState == 2) {
                    return -1;
                }
                break;
        default:
                return 0;
                break; 
    }
    return 0;
}