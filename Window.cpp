//
// Created by Пермяков Михаил Павлович on 09.07.25.
//
#include "Window.h"


bool Window::Init(const char *title, int width, int height)
{
    _window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!_window)
    {
        std::cerr << "Can't create glfw window\n";
        glfwTerminate();
        _window = nullptr;
        _isOpened = false;
        return false;
    }
    glfwSetFramebufferSizeCallback(_window, OnResize);
    return true;
}

bool Window::IsInited() const
{
    return _window != nullptr;
}

bool Window::IsOpened() const
{
    return _isOpened && !glfwWindowShouldClose(_window);
}


bool Window::IsCursorHidden() const
{
    return _isCursorHidden;
}

void Window::SetDepthTest(bool enable) const
{
    EnableParameter(enable, GL_DEPTH_TEST);
}

glm::vec2 Window::GetSize() const
{
    int w, h;
    glfwGetFramebufferSize(_window, &w, &h);
    return glm::vec2(w, h);
}

void Window::Close()
{
    glfwSetWindowShouldClose(_window, GLFW_TRUE);
    _isOpened = false;
    onClose(*this);
}

void Window::Clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::Swap()
{
    glfwSwapBuffers(_window);
    glfwPollEvents();
}

void Window::SetCurrent()
{
    glfwMakeContextCurrent(_window);
}

bool Window::IsKeyPressed(int glfwKey)
{
    return glfwGetKey(_window, glfwKey) == GLFW_PRESS;
}

bool Window::IsCursorBtnPressed(int glfwButton)
{
    return glfwGetMouseButton(_window, glfwButton) == GLFW_PRESS;
}

bool Window::Equal(const Window &window) const
{
    return _window == window._window;
}

void Window::OnResize(GLFWwindow *w, int width, int height)
{
    glfwMakeContextCurrent(w);
    glViewport(0, 0, width, height);
}

void Window::EnableParameter(bool enable, unsigned int parameter)
{
    if (enable)
    {
        glEnable(parameter);
    }
    else
    {
        glDisable(parameter);
    }
}

void Window::HideCursor(bool hide)
{
    _isCursorHidden = hide;

    if (hide)
    {
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    else
    {
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Window::SetCursorPosition(glm::vec2 pos)
{
    glfwSetCursorPos(_window, pos.x, pos.y);
}

glm::vec2 Window::GetCursorPosition()
{
    double w, h;
    glfwGetCursorPos(_window, &w, &h);
    return glm::vec2(w, h);
}
