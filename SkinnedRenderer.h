#ifndef OPENGL_TUTORIALS_SKINNEDRENDERER_H
#define OPENGL_TUTORIALS_SKINNEDRENDERER_H
#include "Tr.h"
#include "MeshRenderer.h"
#include "Tex.h"
#include "Shader.h"
#include <vector>
#include <map>

struct SkinnedRenderer {
    static SkinnedRenderer LoadGLTF(std::string path);

    struct Skin {
        std::vector<int> joints;
        std::vector<glm::mat4> inverse_bind_mats;
        std::string name;
        int skeleton_root_id{ -1 };
    };
    struct Clip {
        struct Sampler {
            enum class InterpolationType {
                Unknown,
                Linear,
                Step,
                CubicSpline
            };
            bool IsInvalid() const { return inputs.size() > outputs.size(); }
            std::vector<float> inputs;
            std::vector<glm::vec4> outputs;
            InterpolationType type;
        };
        struct Channel {
            enum class PathType {
                Unknown,
                Translation,
                Rotation,
                Scale,
                Weights
            };
            int nodeId;
            int samplerId;
            PathType type;
        };
        std::vector<Sampler> samplers;
        std::vector<Channel> channels;
        float start;
        float end;
        std::string name;
    };
    struct Node {
        Tr* transform;
        int nodeId{ -1 };
        int parentId{ -1 };
        int meshId{ -1 };
        int skinId{ -1 };
        std::vector<int> childIds;
    };

    void SampleClip(float time);
    void Render(Shader const& shader);

private:
    Tr* _transform;
    int _currentClip { 0 };
    std::map<int, MeshRenderer> _meshes;
    std::map<int, Node> _nodes;
    std::map<int, Skin> _skins;
    std::map<int, Clip> _clips;
    std::map<int, Tex> _textures;

    void UpdateNode(int nodeId);
    glm::mat4 GetNodeMatrix(int node_id);
};

#endif //OPENGL_TUTORIALS_SKINNEDRENDERER_H
