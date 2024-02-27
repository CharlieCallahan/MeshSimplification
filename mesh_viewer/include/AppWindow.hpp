#include "Application.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

#ifndef APPWINDOW
#define APPWINDOW

//application composed of a window
class AppWindow : public ApplicationBase{
public:
    AppWindow(int width, int height, std::string windowName);

    ~AppWindow();

    bool shouldExit();

    virtual void updateState();

    //process user input here
    virtual void processInput();

    GLFWwindow *window;

};

#endif //APPWINDOW