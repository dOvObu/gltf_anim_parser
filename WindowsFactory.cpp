#include "WindowsFactory.h"

bool WindowsFactory::Init(int major, int minor) {
    if(!glfwInit())
    {
        std::cerr << "Can't init glfw\n";
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_OSMESA_CONTEXT_API);
#endif
    return true;
}

Window WindowsFactory::CreateWindow(const char *title, int width, int height) {
    Window window;
    if (!window.Init(title, width, height))
    {
        std::cerr << "Can't init window\n";
        return {};
    }
    window.SetCurrent();
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr<<"Can't init glad\n";
        return {};
    }
    WindowsFactory* self = this;
    window.onClose = [self](Window w)
    {
        self->OnClose(w);
    };
    _windows.push_back(window);
    return window;
}

void WindowsFactory::OnClose(Window &window) {
    window.SetCurrent();
    size_t i = 0;
    size_t const sz = _windows.size();
    for(; i < sz; ++i)
    {
        if (_windows[i].Equal(window))
        {
            _windows.erase(std::begin(_windows) + i);
            break;
        }
    }
    glfwTerminate();
    window = {};
}
