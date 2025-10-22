#ifndef OPENGL_TUTORIALS_CAMERA_H
#define OPENGL_TUTORIALS_CAMERA_H

#include "Tr.h"
#include "Window.h"

struct Camera {
    static Camera* Create()
    {
        Camera* self = new Camera();
        self->_transform = Tr::Create();
        self->_transform->SetLPosition(glm::vec3(0,0,2));
        self->SetPerspective(true);
        return self;
    }

    static void Destroy(Camera* c)
    {
        Tr::Destroy(c->_transform);
        delete c;
    }

    static Camera* GetMain()
    {
        static Camera* main = Create();
        return main;
    }

    Tr *GetTransform() { return _transform; }

    glm::vec2 GetSize() const { return _size; }
    void SetSize(glm::vec2 size)
    {
        _size = size;
        SetPerspective(_isPerspective);
    }

    bool IsPerspective () const { return _isPerspective; }
    void SetPerspective(bool isPerspective)
    {
        _isPerspective = isPerspective;
        _proj = isPerspective
                ? glm::perspective(_fow, _size.x / _size.y, _depthClipNear, _depthClipFar)
                : glm::ortho(0.0f, _size.x, 0.0f, _size.y, _depthClipNear, _depthClipFar);
    }

    void SetupFor(Shader const& shader)
    {
        if (shader.IsReadyToUse())
        {
            shader.Use();
        }
        _cameraMatUniformId = shader.GetUniformId("camera");
        glUniformMatrix4fv(_cameraMatUniformId, 1, GL_FALSE, glm::value_ptr(_cameraMat));
    }

    void SendToGPU(int cameraMatUniformId)
    {
        _cameraMatUniformId = cameraMatUniformId;
        glUniformMatrix4fv(cameraMatUniformId, 1, GL_FALSE, glm::value_ptr(_cameraMat));
    }

    void Update()
    {
        glm::quat r = _transform->GetWRotation();
        glm::vec3 p = _transform->GetWPosition();
        _view = glm::lookAt(-p, -p + r * _orientation, r * _up);
        _cameraMat = _proj * _view;
    }

    void WasdInput(Window& w, float deltaTime)
    {
        if (w.IsKeyPressed(GLFW_KEY_W))
        {
            _transform->SetLPosition(_transform->GetLPosition() - deltaTime * _speed * _orientation);
        }
        if (w.IsKeyPressed(GLFW_KEY_A))
        {
            _transform->SetLPosition(_transform->GetLPosition() + deltaTime * _speed * glm::normalize(glm::cross(_orientation, _up)));
        }
        if (w.IsKeyPressed(GLFW_KEY_S))
        {
            _transform->SetLPosition(_transform->GetLPosition() + deltaTime * _speed * _orientation);
        }
        if (w.IsKeyPressed(GLFW_KEY_D))
        {
            _transform->SetLPosition(_transform->GetLPosition() - deltaTime * _speed * glm::normalize(glm::cross(_orientation, _up)));
        }
        if (w.IsKeyPressed(GLFW_KEY_E))
        {
            _transform->SetLPosition(_transform->GetLPosition() + deltaTime * _speed * glm::cross(_orientation, glm::normalize(glm::cross(_orientation, _up))));
        }
        if (w.IsKeyPressed(GLFW_KEY_Q))
        {
            _transform->SetLPosition(_transform->GetLPosition() - deltaTime * _speed * glm::cross(_orientation, glm::normalize(glm::cross(_orientation, _up))));
        }
    }

    void MouseUnityFPSInput(Window& w)
    {
        if (w.IsCursorBtnPressed(GLFW_MOUSE_BUTTON_RIGHT))
        {
            bool wasHidden = w.IsCursorHidden();
            w.HideCursor(true);

            glm::vec2 size = w.GetSize();

            if (!wasHidden)
            {
                w.SetCursorPosition(0.5f * size);
            }

            glm::vec2 mousePos = w.GetCursorPosition();

            float rotX = _sensitivity * (float)(mousePos.y - (size.y / 2)) / size.y;
            float rotY = _sensitivity * (float)(mousePos.x - (size.x / 2)) / size.x;

            glm::vec3 newOrientation = glm::angleAxis(glm::radians(-rotX), glm::normalize(glm::cross(_orientation, _up))) * _orientation;

            // Decides whether or not the next vertical Orientation is legal or not

            if (abs(glm::acos(glm::dot(newOrientation, _up)) - glm::radians(90.0f)) <= glm::radians(85.0f))
            {
                _orientation = newOrientation;
            }

            // Rotates the Orientation left and right
            _orientation = glm::angleAxis(glm::radians(-rotY), _up) * _orientation;
            w.SetCursorPosition(0.5f * size);
        }
        else
        {
            if (w.IsCursorHidden())
            {
                w.SetCursorPosition(0.5f * w.GetSize());
            }
            w.HideCursor(false);
        }
    }

    Camera(Camera const&) = default;
    ~Camera() = default;
private:
    Tr *_transform;
    glm::vec2 _size{800.f, 800.f};
    float _depthClipNear{0.1f};
    float _depthClipFar{100.f};
    float _fow{glm::radians(45.f)};
    float _speed{1.f};
    float _sensitivity{100.f};
    bool _isPerspective;
    glm::mat4 _view{1.f};
    glm::mat4 _proj{1.f};
    unsigned int _cameraMatUniformId{0};
    glm::mat4 _cameraMat;
    const glm::vec3 _up{0.f, 1.f, 0.f};
    glm::vec3 _orientation{0.f, 0.f, -1.f};
    Camera() = default;
};

#endif //OPENGL_TUTORIALS_CAMERA_H
