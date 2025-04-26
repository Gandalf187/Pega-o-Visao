#include <chrono>

class Debouncer {
private:
    std::chrono::steady_clock::time_point lastTriggerTime;
    std::chrono::milliseconds debounceDelay;
    bool lastState;
    
public:
    Debouncer(int delayMs) : 
        debounceDelay(std::chrono::milliseconds(delayMs)),
        lastState(false) 
    {
        lastTriggerTime = std::chrono::steady_clock::now();
    }
    
    bool debounce(bool currentState) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = now - lastTriggerTime;
        
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