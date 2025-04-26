#pragma once
#ifndef DEBOUNCER_HPP
#define DEBOUNCER_HPP

#include <opencv2/opencv.hpp>
#include <chrono>

class Debouncer {
    private:
        steady_clock::time_point lastTriggerTime;
        int debounceDelay; // em milissegundos
        bool lastState;
        
    public:
        Debouncer(int delayMs) : debounceDelay(delayMs), lastState(false) {
            lastTriggerTime = steady_clock::now();
        }
        
        bool debounce(bool currentState) {
            auto now = steady_clock::now();
            auto elapsed = duration_cast<milliseconds>(now - lastTriggerTime).count();
            
            if (currentState && (!lastState || elapsed >= debounceDelay)) {
                lastTriggerTime = now;
                lastState = true;
                return true;
            }
            
            if (!currentState) {
                lastState = false;
            }
            
            return false;
        }
    };

#endif