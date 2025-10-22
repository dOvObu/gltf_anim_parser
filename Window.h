//
// Created by Пермяков Михаил Павлович on 09.07.25.
//

#ifndef OPENGL_TUTORIALS_WINDOW_H
#define OPENGL_TUTORIALS_WINDOW_H

#include "glm/glm.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

struct Window
{
    std::function<void(Window)> onClose; // event

    bool IsInited() const;
    bool IsOpened() const;
    bool IsCursorHidden() const;
    void SetDepthTest(bool enable) const;

    glm::vec2 GetSize() const;

    bool Init(char const* title, int width = 640, int height = 480);
    void SetCurrent();
    void Close();

    void Clear(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.f);
    void Swap();
    bool IsKeyPressed(int glfwKey);
    bool IsCursorBtnPressed(int glfwButton);
    bool Equal(Window const& window) const;

    void HideCursor(bool hide);
    void SetCursorPosition(glm::vec2 pos);
    glm::vec2 GetCursorPosition();

private:
    GLFWwindow* _window = nullptr;
    bool _isOpened = true;
    bool _isCursorHidden = false;
    static void OnResize(GLFWwindow* w, int width, int height);
    static void EnableParameter(bool enable, unsigned parameter);
};

#endif //OPENGL_TUTORIALS_WINDOW_H
