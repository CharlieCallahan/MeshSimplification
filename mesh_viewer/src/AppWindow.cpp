#include "AppWindow.hpp"
#include <iostream>

AppWindow::AppWindow(int width, int height, std::string windowName){
    glfwInit();
    #ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    window = glfwCreateWindow(width, height, windowName.c_str(), NULL, NULL);
    // glfwSetWindowUserPointer(window->window,window); //this allows us to access the OpenGLWindow object from the framebuffer callback
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	// glfwSetFramebufferSizeCallback(window->window, frameBuffCallback);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

AppWindow::~AppWindow(){
    glfwTerminate();
    glfwDestroyWindow(window);
}

bool AppWindow::shouldExit(){
    return glfwWindowShouldClose(this->window);
}

void AppWindow::updateState(){
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

//process user input here
void AppWindow::processInput(){

}