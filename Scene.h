#ifndef OPENGL_TUTORIALS_SCENE_H
#define OPENGL_TUTORIALS_SCENE_H
#include "json.hpp"
#include "Tr.h"
#include "Camera.h"
#include "PrimitiveMesh.h"
#include <string>
#include <map>

struct Theater {
    struct Scene {
        std::string name;
        std::map<int, Tr*> nodes;
    };

    static Theater LoadGLTF(nlohmann::json root)
    {
        using json = nlohmann::json;
        json scenes = root["scenes"];
        Theater result;
        result._scenes.reserve(scenes.size());

        for (json scene : scenes.items())
        {
            result._scenes.push_back(new Scene {
                std::string(scene["name"])
            });
            Scene* s = result._scenes.back();

            json nodes = scene["nodes"];
            for (json node : nodes.items())
            {
                Tr::LoadGLTF(root, (int)node, result._nodes);
            }
        }

        return result;
    }

private:
    std::vector<Scene*> _scenes;
    std::map<int, Tr*> _nodes;
};

#endif //OPENGL_TUTORIALS_SCENE_H
