#ifndef OPENGL_TUTORIALS_WINDOWSFACTORY_H
#define OPENGL_TUTORIALS_WINDOWSFACTORY_H

#include "Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

struct WindowsFactory
{
    bool Init(int major = 4, int minor = 2);
    Window CreateWindow(char const* title, int width = 640, int height = 480);

private:
    std::vector<Window> _windows;
    void OnClose(Window& window);
};

#endif //OPENGL_TUTORIALS_WINDOWSFACTORY_H
