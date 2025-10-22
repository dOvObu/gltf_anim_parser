#ifndef OPENGL_TUTORIALS_TR_H
#define OPENGL_TUTORIALS_TR_H

#include "Shader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "json.hpp"
#include <set>
#include <vector>
#include <map>
#include "tiny_gltf.hpp"

struct Tr
{
    static Tr* Create()
    {
        return new Tr();
    }

    static void Destroy(Tr*& tr)
    {
        delete tr;
        tr = nullptr;
    }

    static Tr* LoadGLTF(nlohmann::json const& root, int nodeId, std::map<int, Tr*>& nodes, std::map<Tr*, int> cameras, std::map<Tr*, int> meshes, std::map<Tr*, int> skins)
    {
        using json = nlohmann::json;
        Tr* tr = Create();
        nodes[nodeId] = tr;
        json node = root["nodes"][nodeId];
        tr->_name = node["name"];

        json translation = node.value("translation", json::array());
        if (translation.size() == 3)
        {
            tr->_locPos = glm::vec3
            {
                translation[0],
                translation[1],
                translation[2],
            };
        }

        json rotation = node.value("rotation", json::array());
        if (rotation.size() == 4)
        {
            tr->_locRot = glm::quat
            {
                rotation[0],
                rotation[1],
                rotation[2],
                rotation[3],
            };
        }

        json scale = node.value("scale", json::array());
        if (scale.size() == 3)
        {
            tr->_locScale = glm::vec3
            {
                scale[0],
                scale[1],
                scale[2],
            };
        }

        int mesh = node.value("mesh", -1);
        int skin = node.value("skin", -1);
        int camera = node.value("camera", -1);

        json children = node.value("children", json::array());

        for (json child : children.items())
        {
            //LoadGLTF(root, (int)child)->SetParentKeepingLocalTransform(tr);
        }
        tr->RecalcMat();
        return tr;
    }

private:
    Tr() = default;
    Tr(const Tr&)            = delete;
    Tr& operator=(const Tr&) = delete;
public:
    ~Tr()
    {
        for (Tr* ch : _children)
        {
            delete ch;
        }
        _children.clear();
    }

    void ApplyTo(Shader const& shader, std::string const& parameterId);
    void SendToGPU(int uniformId);

    Tr*       SetWPosition(glm::vec3 p);
    glm::vec3 GetWPosition()const;

    Tr*       SetLPosition(glm::vec3 p);
    glm::vec3 GetLPosition()const;

    Tr*       SetWEuler(glm::vec3 r);
    glm::vec3 GetWEuler() const;

    Tr*       SetLEuler(glm::vec3 r);
    glm::vec3 GetLEuler()const;

    Tr*       SetWRotation(glm::quat r);
    glm::quat GetWRotation()const;

    Tr*       SetLRotation(glm::quat r);
    glm::quat GetLRotation()const;

    Tr* SetWScale(glm::vec3 s);
    glm::vec3 GetWScale() const;

    Tr*       SetLScale(glm::vec3 s);
    glm::vec3 GetLScale()const;

    void SetParent(Tr* transform, bool recalcMatRecursively = true);
    void SetParentKeepingLocalTransform(Tr* transform);


    Tr* Q(std::string const& name);
    std::vector<Tr*> Query(std::string const& name);
    Tr* Q(std::function<bool(Tr*)> p);
    std::vector<Tr*> Query(std::function<bool(Tr*)> p);



    void SetLPositionWithoutMatrixRecalc(glm::vec3 p);
    void SetLRotationWithoutMatrixRecalc(glm::quat r);
    void SetLScaleWithoutMatrixRecalc(glm::vec3 s);

    glm::mat4 GetMatrix() const { return _mat; }
    glm::mat4 GetLocalMatrix() const
    {
        return /*_mat */ glm::scale(glm::mat4{1.f}, _locScale) * glm::mat4(_locRot) * glm::translate(glm::mat4{1.f}, _locPos);
    }

    void RecalcMat(bool recursively = true);
    
private:
    std::string _name{ "" };
    glm::mat4 _mat{ 1.0f };
    glm::vec3 _locPos{ 0.0f };
    glm::quat _locRot{ glm::identity<glm::quat>() };
    glm::vec3 _locScale{ 1.0f };

    Tr* _parent{nullptr };
    std::vector<Tr*> _children;


};

#endif //OPENGL_TUTORIALS_TR_H
