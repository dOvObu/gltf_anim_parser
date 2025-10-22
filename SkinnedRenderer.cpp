#include "SkinnedRenderer.h"

inline bool IsInbetween(double x, double A, double B)
{
    return A <= x && x <= B;
}

void SkinnedRenderer::SampleClip(float time)
{
    if (_clips.empty())
    {
        return;
    }

    Clip& clip = _clips[_currentClip];
    time = fmod(time, clip.end - clip.start);
    bool isSampled = false;

    for (Clip::Channel& channel : clip.channels)
    {
        Clip::Sampler& sampler = clip.samplers[channel.samplerId];
        if (sampler.IsInvalid())
        {
            continue;
        }

        for (size_t i = 0, sz = sampler.inputs.size() - 1; i < sz; ++i)
        {
            if (!IsInbetween(time, sampler.inputs[i], sampler.inputs[i + 1]))
            {
                continue;
            }
            float interpTime = std::fmaxf(0.f, time - sampler.inputs[i]);
            if (interpTime > 1.f)
            {
                continue;
            }
            switch (channel.type)
            {
                case Clip::Channel::PathType::Translation:
                    _nodes[channel.nodeId].transform->SetLPositionWithoutMatrixRecalc(glm::vec3(sampler.outputs[i]));
                    break;
                case Clip::Channel::PathType::Rotation:
                    _nodes[channel.nodeId].transform->SetLRotationWithoutMatrixRecalc(glm::quat(glm::normalize(sampler.outputs[i])));
                    break;
                case Clip::Channel::PathType::Scale:
                    _nodes[channel.nodeId].transform->SetLScaleWithoutMatrixRecalc(glm::vec3(sampler.outputs[i]));
                    break;
                default:
                    break;
            }
            isSampled = true;
        }
    }

    if (!isSampled)
    {
        return;
    }

    UpdateNode(0);
}

void SkinnedRenderer::Render(const Shader &shader)
{
}

constexpr unsigned int MAX_NUM_JOINTS = 128U;

glm::mat4 SkinnedRenderer::GetNodeMatrix(int node_id)
{
    Node& node = _nodes[node_id];
    glm::mat4 matrix = node.transform->GetLocalMatrix();

    for(auto id = node.parentId; id != -1; id = _nodes[id].parentId)
    {
        matrix = matrix * _nodes[id].transform->GetLocalMatrix();
    }
    return matrix;
}

void SkinnedRenderer::UpdateNode(int nodeId)
{
    Node& node = _nodes[nodeId];

    if (node.meshId > -1)
    {
        MeshRenderer& mesh = _meshes[node.meshId];
        mesh.SetTransform(node.transform);

        if (node.skinId > -1)
        {
            Skin& skin = _skins[node.skinId];
            glm::mat4 inverseTr = glm::inverse(node.transform->GetMatrix());

            unsigned int jointsCount = std::min((unsigned int)skin.joints.size(), MAX_NUM_JOINTS);
            for (unsigned int i = 0; i < jointsCount; ++i)
            {
                /* ref : https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_020_Skins.md */
                glm::mat4 jointMat = skin.inverse_bind_mats[i] * GetNodeMatrix(skin.joints[i]) * inverseTr;
                mesh.jointMats[i] = jointMat;
            }
        }
    }

    for (int child : node.childIds)
    {
        UpdateNode(child);
    }
}

SkinnedRenderer SkinnedRenderer::LoadGLTF(std::string path)
{
    using json = nlohmann::json;
    using byte = unsigned char;

    std::replace(std::begin(path), std::end(path), '\\', '/');
    std::string text = AssetsUtility::LoadAllTextFromFile(path);

    json root = json::parse(text);
    json nodes = root["nodes"];
    json meshes = root["meshes"];
    SkinnedRenderer self;

    return self;
}
