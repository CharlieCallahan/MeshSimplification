#include "Application.hpp"

void ApplicationBase::run(){
    while(!this->shouldExit()){
        processInput();
        updateState();
    }
}