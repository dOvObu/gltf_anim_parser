#ifndef OPENGL_TUTORIALS_MESHRENDERER_H
#define OPENGL_TUTORIALS_MESHRENDERER_H

#include "PrimitiveMesh.h"
#include "Shader.h"
#include "Tex.h"
#include "Tr.h"
#include "Camera.h"

#include <map>
#include <string>

struct MeshRenderer {

    MeshRenderer& SetTransform(Tr* tr) { _transform = tr; return *this; }
    MeshRenderer& SetMesh(PrimitiveMesh const& mesh) { _mesh = mesh; return *this; }
    MeshRenderer& SetShader(Shader const& shader) { _shader = shader; return *this; }
    MeshRenderer& SetVec4(std::string const& name, glm::vec4 vec) { _vec4s[name] = vec; return *this; }
    MeshRenderer& SetVec3(std::string const& name, glm::vec3 vec) { _vec3s[name] = vec; return *this; }
    MeshRenderer& SetVal(std::string const& name, float value) { _values[name] = value; return *this; }
    MeshRenderer& AddTex(std::string name, Tex* tex) { _textures[name] = tex; return *this; }
    MeshRenderer& SetDrawType(PrimitiveMesh::DrawType drawType) { _drawType = drawType; return *this; }


    void DrawTo(Camera* camera)
    {
        camera->SetupFor(_shader);
        Draw();
    }

    void Draw()
    {
        if (!_mesh.IsOnGPU())
        {
            _mesh.SendToGPU(_drawType);
        }

        if (!_shader.IsInUsage())
        {
            _shader.Use();
        }

        _inShaderTransformId = _shader.GetUniformId("transform");

        for (auto [n,t] : _textures)
        {
            if (!t->IsOnGPU())
            {
                t->SendToGPU();
            }

            if (!t->IsInUse())
            {
                t->Use();
            }

            t->ApplyTo(_shader, n);
        }

        for (auto [n,v4] : _vec4s)
        {
            glUniform4fv(_shader.GetUniformId(n), 1, glm::value_ptr(v4));
        }

        for (auto [n,v3] : _vec3s)
        {
            glUniform3fv(_shader.GetUniformId(n), 1, glm::value_ptr(v3));
        }

        _transform->SendToGPU(_inShaderTransformId);

        _mesh.Draw();
    }

    std::map<int, glm::mat4> jointMats;
private:
    Tr* _transform;
    PrimitiveMesh _mesh;
    Shader _shader;
    std::map<std::string, Tex*> _textures;
    std::map<std::string, float> _values;
    std::map<std::string, glm::vec4> _vec4s;
    std::map<std::string, glm::vec3> _vec3s;
    int _inShaderTransformId;

    PrimitiveMesh::DrawType _drawType = PrimitiveMesh::DrawType::Static;
};


#endif //OPENGL_TUTORIALS_MESHRENDERER_H
