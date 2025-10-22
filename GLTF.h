//
// Created by Пермяков Михаил Павлович on 01.08.25.
//

#ifndef OPENGL_TUTORIALS_GLTF_H
#define OPENGL_TUTORIALS_GLTF_H
#include "AssetsUtility.h"
#include "GPUUtility.h"
#include "Tr.h"
#include "PrimitiveMesh.h"
#include "Camera.h"
#include "Tex.h"

#include "json.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_access.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/spline.hpp"
#include <glad/glad.h>

#include "base64.h" // https://github.com/ReneNyffenegger/cpp-base64

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <iostream>


struct GLTF {
    struct Buffer {
        const std::string INLINE_URI_START = "data:application/gltf-buffer;base64,";
        const std::string INLINE_OCTET_URI_START = "data:application/octet-stream;base64,";

        std::string URI;
        int ByteLength;

        std::vector<unsigned char> Bytes;

        void LoadBytes(std::string const& rootFolder)
        {
            if (Bytes.size() == ByteLength)
            {
                return;
            }

            std::string text = URI.starts_with(INLINE_URI_START)
                    ? base64_decode(URI.substr(INLINE_URI_START.length()))
                    : URI.starts_with(INLINE_OCTET_URI_START)
                        ? base64_decode(URI.substr(INLINE_OCTET_URI_START.length()))
                        : AssetsUtility::LoadAllTextFromFile(rootFolder + "/" + URI);
            Bytes = {std::begin(text), std::end(text) };
        }
    };
    struct BufferView {
        enum class TargetType {
            UNKNOWN = -1,
            ARRAY_BUFFER = 34962,
            ELEMENT_ARRAY_BUFFER = 34963
        };
        static int ToInt(TargetType type) { return (int)type; }
        static TargetType ToTargetType(int type) { return (TargetType)type; }

        int Buffer{ 0 };
        int ByteOffset{ 0 };
        int ByteLength{ 0 };
        int ByteStride{ -1 };
        TargetType Target{ TargetType::UNKNOWN };

        GLTF::Buffer& GetBuffer(GLTF* self) const { return self->_buffers[Buffer]; }
    };
    struct Accessor {
        enum class ComponentTypeType {
            Float         = 5126, // array of 4 unsigned char
            UnsignedInt   = 5125, // array of 4 unsigned char
            UnsignedShort = 5123, // array of 2 unsigned char
            Short         = 5122, // array of 2 unsigned char
            UnsignedByte  = 5121, // array of 1 unsigned char
            Byte          = 5120, // array of 1 unsigned char
        };
        static int ToInt(ComponentTypeType type) { return (int)type; }
        static ComponentTypeType ToComponentTypeType(int type) { return (ComponentTypeType)type; }
        int SizeOfComponentType()
        {
            switch (ComponentType)
            {
                case ComponentTypeType::Float:
                case ComponentTypeType::UnsignedInt:
                    return 4;
                case ComponentTypeType::UnsignedShort:
                case ComponentTypeType::Short:
                    return 2;
                case ComponentTypeType::UnsignedByte:
                case ComponentTypeType::Byte:
                    return 1;
            }
            return 2;
        }

        enum class TypeType {
            Scalar = 1,
            Vec2 = 2,
            Vec3 = 3,
            Vec4 = 4,
            Mat4 = 16
        };
        static std::string ToString(TypeType type)
        {
            switch (type)
            {
                case TypeType::Scalar: return "SCALAR";
                case TypeType::Vec2: return "VEC2";
                case TypeType::Vec3: return "VEC3";
                case TypeType::Vec4: return "VEC4";
                case TypeType::Mat4: return "MAT4";
                default: return "SCALAR";
            }
        }
        static TypeType ToTypeType(std::string const& str)
        {
            if (str == "SCALAR") return TypeType::Scalar;
            if (str == "VEC2") return TypeType::Vec2;
            if (str == "VEC3") return TypeType::Vec3;
            if (str == "VEC4") return TypeType::Vec4;
            if (str == "MAT4") return TypeType::Mat4;
            return TypeType::Scalar;
        }
        int NumberOfComponents() { return (int)Type; }

        int BufferView{ 0 };
        int ByteOffset{ 0 };
        ComponentTypeType ComponentType { ComponentTypeType::Float };
        int Count{ 0 };
        TypeType Type { TypeType::Scalar };

        GLTF::BufferView& GetBufferView(GLTF* self) const { return self->_bufferViews[BufferView]; }
    };
    struct Mesh {
        struct Primitive {
            enum class ModeType
            {
                POINTS         = 0,
                LINES          = 1,
                LINE_LOOP      = 2,
                LINE_STRIP     = 3,
                TRIANGLES      = 4,
                TRIANGLE_STRIP = 5,
                TRIANGLE_FAN   = 6
            };
            std::map<std::string, int> Attributes;
            std::vector<std::map<std::string, int>> Targets;
            int Indices;
            int Material{ -1 };
            GLTF::Mesh::Primitive::ModeType Mode{ GLTF::Mesh::Primitive::ModeType::TRIANGLES };

            GLTF::Accessor& GetIndices(GLTF* self) const
            {
                return self->_accessors[Indices];
            }

            void GetVerts(GLTF* self,
                          std::map<int, std::string> const& inputs,
                          std::vector<::PrimitiveMesh::AttributeType>& format,
                          std::vector<float>& res) const
            {
                std::map<std::string, GLTF::Accessor*> acs = GetAccessors(self);
                if (acs.empty())
                {
                    return;
                }

                std::vector<std::map<std::string, GLTF::Accessor*>> targets = GetTargets(self);

                size_t size = 0;

                for (auto&[idx, name] : inputs)
                {
                    GLTF::Accessor* acc = acs[name];
                    if (acc == nullptr)
                    {
                        format.push_back(GetCommonTypeOfAttribute(name));
                        continue;
                    }
                    format.push_back((::PrimitiveMesh::AttributeType)acc->Type);
                    GLTF::BufferView& bv = acc->GetBufferView(self);
                    GLTF::Buffer& b = bv.GetBuffer(self);
                    b.LoadBytes(self->_rootFolder);
                    for (auto& t : targets)
                    {
                        if (t.contains(name))
                        {
                            format.push_back((::PrimitiveMesh::AttributeType)t[name]->Type);
                        }
                    }

                    if (name.starts_with("POSITION"))
                    {
                        size = acc->Count;
                    }
                }

                for (size_t vert = 0; vert < size; ++vert)
                {
                    for (auto&[idx, name] : inputs)
                    {
                        GLTF::Accessor* acc = acs[name];
                        if (acc == nullptr)
                        {
                            int const type = (int)GetCommonTypeOfAttribute(name);
                            for (int j = 0; j < type; ++j)
                            {
                                res.push_back(0.f);
                            }
                            continue;
                        }
                        GLTF::CollectVertNumbersByAccessor(self, acc, vert, res);
                        for (auto& t : targets)
                        {
                            if (t.contains(name))
                            {
                                GLTF::CollectVertNumbersByAccessor(self, t[name], vert, res);
                            }
                        }
                    }
                }
            }

            void GetElements(GLTF* self, std::vector<int>& res) const
            {
                auto acc = GetIndices(self);
                auto& bv = acc.GetBufferView(self);
                auto& buffer = bv.GetBuffer(self);
                buffer.LoadBytes(self->_rootFolder);
                size_t const size = acc.Count;
                for (size_t vert = 0; vert < size; ++vert)
                {
                    GLTF::CollectVertNumbersByAccessor(self, &acc, vert, res);
                }
            }

            void UseMaterial(GLTF* self)
            {
                if (self->_shaders.contains(Material))
                {
                    Shader& shader = self->_shaders[Material];
                    if (shader.IsReadyToUse())
                    {
                        if (!shader.IsInUsage())
                        {
                            shader.Use();
                        }
                    }
                }
            }

        private:
            std::map<std::string, GLTF::Accessor*> GetAccessors(GLTF* self) const
            {
                std::map<std::string, GLTF::Accessor*> res;
                for (auto&[val, key] : Attributes)
                {
                    res[val] = &self->_accessors[key];
                }
                return res;
            }
            std::vector<std::map<std::string, GLTF::Accessor*>> GetTargets(GLTF* self) const
            {
                std::vector<std::map<std::string, GLTF::Accessor*>> res;
                for (auto& target : Targets)
                {
                    res.emplace_back();
                    auto& trg = res.back();
                    for (auto& [val, key] : target)
                    {
                        trg[val] = &self->_accessors[key];
                    }
                }
                return res;
            }
            static ::PrimitiveMesh::AttributeType GetCommonTypeOfAttribute(std::string const& name)
            {
                if (name.starts_with("TEXCOORD_")) return ::PrimitiveMesh::AttributeType::Vec2;
                if (name.starts_with("POSITION")) return ::PrimitiveMesh::AttributeType::Vec3;
                if (name.starts_with("NORMAL")) return ::PrimitiveMesh::AttributeType::Vec3;
                if (name.starts_with("JOINTS_")) return ::PrimitiveMesh::AttributeType::Vec4;
                if (name.starts_with("WEIGHTS_")) return ::PrimitiveMesh::AttributeType::Vec4;
                return ::PrimitiveMesh::AttributeType::Float;
            }
        };
        std::vector<Primitive> Primitives;
        std::vector<float> Weights;

        std::vector<::PrimitiveMesh> PrimitiveMeshes;


        void MakeSureItsOnGPU(GLTF* self)
        {
            for (size_t i = 0; i < PrimitiveMeshes.size(); ++i)
            {
                if (PrimitiveMeshes[i].IsOnGPU())
                {
                    continue;
                }

                Primitives[i].UseMaterial(self);
                PrimitiveMeshes[i].SendToGPU();
            }
        }

        void Draw(GLTF* self, Tr* transform)
        {
            for (size_t i = 0, sz = Primitives.size(); i < sz; ++i)
            {
                Primitives[i].UseMaterial(self);
                transform->SendToGPU(Shader::GetUniformIdOfCurrentShader("transform"));
                Camera::GetMain()->SendToGPU(Shader::GetUniformIdOfCurrentShader("camera"));
                PrimitiveMeshes[i].Draw();
            }
        }

        void Draw(GLTF* self, Tr* transform, Tex::TexBuffer& jointMats)
        {
            for (size_t i = 0, sz = Primitives.size(); i < sz; ++i)
            {
                Primitives[i].UseMaterial(self);
                transform->SendToGPU(Shader::GetUniformIdOfCurrentShader("transform"));
                Camera::GetMain()->SendToGPU(Shader::GetUniformIdOfCurrentShader("camera"));
                jointMats.SendToGPU(Shader::GetUniformIdOfCurrentShader("jointsMats"));

                for (auto const& v : self->vec3s)
                {
                    glUniform3f(Shader::GetUniformIdOfCurrentShader(v.first), v.second.x, v.second.y, v.second.z);
                }

                for (auto const& v : self->vec4s)
                {
                    glUniform4f(Shader::GetUniformIdOfCurrentShader(v.first), v.second.x, v.second.y, v.second.z, v.second.w);
                }

                PrimitiveMeshes[i].Draw();
            }
            
        }

        void Load(GLTF* self, Shader const& shader, std::vector<::PrimitiveMesh>& meshes)
        {
            std::map<int, std::string> const& inputs = shader.GetInputs();

            for (auto& primitive : Primitives)
            {
                meshes.emplace_back();
                ::PrimitiveMesh& mesh = meshes.back();

                std::vector<::PrimitiveMesh::AttributeType> format;
                std::vector<float> vertices;
                std::vector<int> indices;
                primitive.GetVerts(self, inputs, format, vertices);
                primitive.GetElements(self, indices);
                mesh.Init(format, vertices, indices);
            }
        }
    };
    struct Node {
        std::string Name;
        int Skin{ -1 };
        int Mesh{ -1 };
        std::vector<int> Children;
        glm::vec3 Translation{ 0.f, 0.f, 0.f };
        glm::quat Rotation{ 0.f, 0.f, 0.f, 1.f };
        glm::vec3 Scale{ 1.f, 1.f, 1.f };
        Tr* Transform{ nullptr };


        auto& GetSkin(GLTF* self) const { return self->_skins[Skin]; }
        GLTF::Mesh& GetMesh(GLTF* self) const { return self->_meshes[Mesh]; }
        bool HasSkin() const { return Skin != -1; }
        bool HasMesh() const { return Mesh != -1; }
        std::vector<Node*> GetChildren(GLTF* self) const
        {
            std::vector<Node*> res;
            for (int n : Children)
            {
                res.push_back(&self->_nodes[n]);
            }
            return res;
        }

        void PreWarm(GLTF* self, Shader const& shader)
        {
            if (HasMesh())
            {
                auto& mesh = GetMesh(self);

                if (mesh.PrimitiveMeshes.empty())
                {
                    mesh.Load(self, shader, mesh.PrimitiveMeshes);
                }
            }

            auto children = GetChildren(self);
            for (auto node : children)
            {
                node->PreWarm(self, shader);
            }
        }

        void Draw(GLTF* self)
        {
            if (HasMesh())
            {
                auto& mesh = GetMesh(self);
                mesh.MakeSureItsOnGPU(self);

                if (HasSkin())
                {
                    //Transform->RecalcMat(false);

                    auto& skin = GetSkin(self);
                    skin.CalculateJointMats(self);
                    mesh.Draw(self, Transform, skin.TextureBuffer);
                }
                else
                {
                    mesh.Draw(self, Transform);
                }
            }

            auto children = GetChildren(self);
            for (auto node : children)
            {
                node->Draw(self);
            }
        }

        void InitTransform(GLTF* self)
        {
            if (Transform != nullptr)
            {
                Tr::Destroy(Transform);
            }
            Transform = Tr::Create();
            Transform->SetLPosition(Translation);
            Transform->SetLRotation(Rotation);
            Transform->SetLScale(Scale);
            auto children = GetChildren(self);
            for (auto child : children)
            {
                child->InitTransform(self);
                child->Transform->SetParentKeepingLocalTransform(Transform);
                child->Transform->RecalcMat(false);
            }
        }

        ~Node()
        {
            if (Transform != nullptr)
            {
                Tr::Destroy(Transform);
            }
        }
    };
    struct Skin {
        int InverseBindMatrices;
        std::vector<int> Joints;

        // МАТРИЦЫ ДЛЯ КАЖДОГО СУСТАВА — ПЕРЕВОДЯТ ВЕРШИНУ В ЛОКАЛЬНЫЕ КООРДИНАТЫ СУСТАВА
        std::vector<glm::mat4> InvBindMats;
        Tex::TexBuffer TextureBuffer;

        // НОДЫ-СУСТАВЫ
        std::vector<GLTF::Node*> GetJoints(GLTF* self)
        {
            std::vector<GLTF::Node*> res;
            for (int joint : Joints)
            {
                res.push_back(self->_nodes.data() + joint);
            }
            return res;
        }

        void PreWarmInvBindMats(GLTF* self)
        {
            if (InvBindMats.size() == Joints.size())
            {
                return;
            }
            LoadInverseBindMatrices(self, InvBindMats);
            std::vector<glm::mat4> invBindMats;
            if (!TextureBuffer.IsOnGPU())
            {
                TextureBuffer = Tex::TexBuffer::LoadMat4FromRAMToGPU(InvBindMats, 5);
            }
        }

        void CalculateJointMats(GLTF* self)
        {
            if (InvBindMats.size() != Joints.size())
            {
                return;
            }

            std::vector<GLTF::Node*> joints = GetJoints(self);
            std::vector<glm::mat4> jointMats;
            jointMats.reserve(joints.size());

            for (size_t i = 0; i < joints.size(); ++i)
            {
                jointMats.emplace_back(joints[i]->Transform->GetMatrix() * InvBindMats[i]);
            }

            TextureBuffer.SetDataAsMat4s(jointMats);
        }

    private:
        GLTF::Accessor& GetInverseBindMatrices(GLTF* self) const { return self->_accessors[InverseBindMatrices]; }

        void LoadInverseBindMatrices(GLTF* self, std::vector<glm::mat4>& res)
        {
            auto& acc = GetInverseBindMatrices(self);
            for (size_t vert = 0; vert < Joints.size(); ++vert)
            {
                std::vector<float> floats;
                GLTF::CollectVertNumbersByAccessor(self, &acc, vert, floats);
                res.push_back(glm::make_mat4(floats.data()));
            }
        }
    };
    struct Scene {
        std::string Name;
        std::vector<int> Nodes;

        std::vector<Node*> GetNodes(GLTF* self)
        {
            std::vector<Node*> res;
            for (int n : Nodes)
            {
                res.push_back(&self->_nodes[n]);
            }
            return res;
        }
    };
    struct Anim {
        struct Sampler {
            enum class InterpolationType {
                Step,
                Linear,
                CubicSpline
            };
            static std::string ToString(InterpolationType type)
            {
                switch (type)
                {
                    case InterpolationType::Step: return "STEP";
                    case InterpolationType::Linear: return "LINEAR";
                    case InterpolationType::CubicSpline: return "CUBICSPLINE";
                    default: return "LINEAR";
                }
            }
            static InterpolationType ToInterpolationType(std::string const& str)
            {
                if (str == "STEP") return InterpolationType::Step;
                if (str == "LINEAR") return InterpolationType::Linear;
                if (str == "CUBICSPLINE") return InterpolationType::CubicSpline;
                return InterpolationType::Linear;
            }

            int Input{-1};
            InterpolationType Interpolation{ InterpolationType::Linear };
            int Output{-1};

            std::vector<float> Time;
            std::vector<glm::vec3> Vec3;
            std::vector<glm::quat> Quat;
            std::vector<float> Weights;

            glm::vec3 InterpolateVec3(float time)
            {
                if (Time.empty())
                {
                    return glm::vec3(0.f, 0.f, 0.f);
                }

                if (time <= Time[0])
                {
                    return Vec3[0];
                }

                if (Time.back() <= time)
                {
                    return Vec3.back();
                }

                size_t prevKey = 0;

                for (size_t i = 0, sz = Time.size() - 1; i < sz; ++i)
                {
                    if (Time[i] <= time && time < Time[i + 1])
                    {
                        prevKey = i;
                        break;
                    }
                }

                if (Interpolation == InterpolationType::Step)
                {
                    return Vec3[prevKey];
                }

                size_t const nextKey = prevKey + 1;

                if (Interpolation == InterpolationType::Linear)
                {
                    float const t = (time - Time[prevKey]) / (Time[nextKey] - Time[prevKey]);
                    return (1.f - t) * Vec3[prevKey] + t * Vec3[nextKey];
                }

                if (Interpolation == InterpolationType::CubicSpline)
                {
                    float const deltaT = Time[nextKey] - Time[prevKey];
                    float const t = (time - Time[prevKey]) / deltaT;

                    glm::vec3 const previousTangent  = deltaT * Vec3[3 * prevKey + 2];
                    glm::vec3 const previousPoint = Vec3[3 * prevKey + 1];
                    glm::vec3 const nextPoint = Vec3[3 * nextKey + 1];
                    glm::vec3 const nextTangent  = deltaT * Vec3[3 * nextKey];

                    return glm::hermite(previousPoint, previousTangent, nextPoint, nextTangent, t);
                }
                return Vec3[prevKey];
            }

            glm::quat InterpolateQuat(float time)
            {
                if (Time.empty())
                {
                    return glm::quat(0.f, 0.f, 0.f, 1.f);
                }

                if (time <= Time[0])
                {
                    return Quat[0];
                }

                if (Time.back() <= time)
                {
                    return Quat.back();
                }

                size_t prevKey = 0;

                for (size_t i = 0, sz = Time.size() - 1; i < sz; ++i)
                {
                    if (Time[i] <= time && time < Time[i + 1])
                    {
                        prevKey = i;
                        break;
                    }
                }

                if (Interpolation == InterpolationType::Step)
                {
                    return Quat[prevKey];
                }

                size_t const nextKey = prevKey + 1;

                if (Interpolation == InterpolationType::Linear)
                {
                    float const t = (time - Time[prevKey]) / (Time[nextKey] - Time[prevKey]);
                    return glm::slerp(Quat[prevKey], Quat[nextKey], t);
                }

                // https://github.com/KhronosGroup/glTF/issues/1386
                if (Interpolation == InterpolationType::CubicSpline)
                {
                    float const deltaT = Time[nextKey] - Time[prevKey];
                    float const t = (time - Time[prevKey]) / deltaT;

                    glm::quat const previousTangent  = deltaT * Quat[3 * prevKey + 2];
                    glm::quat const previousPoint = Quat[3 * prevKey + 1];
                    glm::quat const nextPoint = Quat[3 * nextKey + 1];
                    glm::quat const nextTangent  = deltaT * Quat[3 * nextKey];

                    return glm::hermite(previousPoint, previousTangent, nextPoint, nextTangent, t);
                }
                return Quat[prevKey];
            }

            float InterpolateWeight(float time)
            {
                // TODO find how weights are stored in memory, restore them and interpolate them
                return 0.f;
            }

            void PreWarmTime(GLTF* self)
            {
                if (!Time.empty())
                {
                    return;
                }
                GetTime(self, Time);
            }

            void PreWarmVec3(GLTF* self)
            {
                if (!Vec3.empty())
                {
                    return;
                }
                GetVec3s(self, Vec3);
            }

            void PreWarmQuat(GLTF* self)
            {
                if (!Quat.empty())
                {
                    return;
                }
                GetQuaternions(self, Quat);
            }

            void PreWarmWeights(GLTF* self)
            {
                /* TODO fix it and load weights properly */
                if (!Weights.empty())
                {
                    return;
                }
                GetFloats(self, Weights);
            }

        private:

            void GetTime(GLTF* self, std::vector<float>& res)
            {
                auto* acc = &self->_accessors[Input];
                for (size_t i = 0; i < acc->Count; ++i)
                {
                    GLTF::CollectVertNumbersByAccessor(self, acc, i, res);
                }
            }
            void GetVec3s(GLTF* self, std::vector<glm::vec3>& res)
            {
                auto* acc = &self->_accessors[Output];
                for (size_t i = 0; i < acc->Count; ++i)
                {
                    std::vector<float> floats;
                    GLTF::CollectVertNumbersByAccessor(self, acc, i, floats);
                    res.push_back(glm::make_vec3(floats.data()));
                }
            }
            void GetQuaternions(GLTF* self, std::vector<glm::quat>& res)
            {
                auto* acc = &self->_accessors[Output];
                for (size_t i = 0; i < acc->Count; ++i)
                {
                    std::vector<float> f;
                    GLTF::CollectVertNumbersByAccessor(self, acc, i, f);
                    glm::quat q(f[3], f[0], f[1], f[2]);
                    res.push_back(q);
                }
            }
            void GetFloats(GLTF* self, std::vector<float>& res)
            {
                auto* acc = &self->_accessors[Output];
                for (size_t i = 0; i < acc->Count; ++i)
                {
                    GLTF::CollectVertNumbersByAccessor(self, acc, i, res);
                }
            }
        };
        struct Channel {
            struct Target {
                enum class PathType
                {
                    Translation,
                    Rotation,
                    Scale,
                    Weights
                };
                int Node;
                PathType Path;

                GLTF::Node& GetNode(GLTF* self) const
                {
                    return self->_nodes[Node];
                }
            };
            int Sampler;
            Target Target;
            GLTF::Anim::Sampler& GetSampler(GLTF::Anim* self) const
            {
                return self->Samplers[Sampler];
            }
        };
        std::string Name = "";
        std::vector<Channel> Channels {};
        std::vector<Sampler> Samplers {};
        float Length { 0.f };

        void LoadFromHDDToRAM(GLTF* self)
        {
            for (auto& channel : Channels)
            {
                auto& sampler = channel.GetSampler(this);
                auto& target = channel.Target;
                sampler.PreWarmTime(self);
                if (sampler.Time.empty())
                {
                    continue;
                }

                if (Length < sampler.Time.back())
                {
                    Length = sampler.Time.back();
                }

                switch (target.Path)
                {
                    case GLTF::Anim::Channel::Target::PathType::Translation:
                        sampler.PreWarmVec3(self);
                        break;
                    case GLTF::Anim::Channel::Target::PathType::Rotation:
                        sampler.PreWarmQuat(self);
                        break;
                    case GLTF::Anim::Channel::Target::PathType::Scale:
                        sampler.PreWarmVec3(self);
                        break;
                    case GLTF::Anim::Channel::Target::PathType::Weights:
                        // TODO understand how floats r exposed
                        break;
                }
            }
        }

        void Eval(GLTF* self, float time)
        {
            for (auto& channel : Channels)
            {
                auto& sampler = channel.GetSampler(this);
                auto& target = channel.Target;
                auto& node = target.GetNode(self);
                sampler.PreWarmTime(self);
                switch (target.Path)
                {
                    case GLTF::Anim::Channel::Target::PathType::Translation:
                        node.Transform->SetLPosition(sampler.InterpolateVec3(time));
                        break;
                    case GLTF::Anim::Channel::Target::PathType::Rotation:
                        node.Transform->SetLRotation(sampler.InterpolateQuat(time));
                        break;
                    case GLTF::Anim::Channel::Target::PathType::Scale:
                        node.Transform->SetLScale(sampler.InterpolateVec3(time));
                        break;
                    case GLTF::Anim::Channel::Target::PathType::Weights:
                        // TODO understand how floats r exposed and sample them also
                        break;
                }
            }
        }
    };

    void SampleClip(int currentClip, float time)
    {
        if (currentClip < 0 || _animations.size() <= currentClip)
        {
            return;
        }
        auto& clip = _animations[currentClip];
        //std::cout << "sampling clip : \"" << clip.Name << "\" at time: " << time << std::endl;
        clip.Eval(this, time);
    }

    float GetClipLength(int currentClip)
    {
        if (currentClip < 0 || _animations.size() <= currentClip)
        {
            return 0.f;
        }
        return _animations[currentClip].Length;
    }

    void PreWarmFor(Shader const& shader)
    {
        for (auto& mesh : _meshes)
        {
            for (auto& pm : mesh.PrimitiveMeshes)
            {
                pm.Clear();
            }
            mesh.PrimitiveMeshes.clear();
        }

        for (auto& scene : _scenes)
        {
            auto roots = scene.GetNodes(this);
            for (auto node : roots)
            {
                node->PreWarm(this, shader);
                //node->Transform->SetLScale(glm::vec3(0.1f));
            }
        }

        SetShader(0, shader);
        SetShader(-1, shader);
    }

    void Draw()
    {
        for (auto& scene : _scenes)
        {
            auto roots = scene.GetNodes(this);
            for (auto node : roots)
            {
                node->Draw(this);
            }
        }
    }

    void LoadFromHDDToRAM(std::string path)
    {
        using json = nlohmann::json;

        std::replace(std::begin(path), std::end(path), '\\', '/');
        _rootFolder = path.substr(0, path.find_last_of('/'));

        std::string allText = AssetsUtility::LoadAllTextFromFile(path);
        json root = json::parse(allText);

        // _scenes;
        json scenes = root["scenes"];
        for (json const& item : scenes)
        {
            _scenes.emplace_back();
            auto& s = _scenes.back();
            s.Name = item.value("name", "");
            s.Nodes = item["nodes"].get<std::vector<int>>();
        }

        // _nodes;
        json nodes = root["nodes"];
        for (json const& item : nodes)
        {
            _nodes.emplace_back();
            auto& n = _nodes.back();
            n.Name = item.value("name", "");
            n.Skin = item.value("skin", -1);
            n.Mesh = item.value("mesh", -1);
            n.Children = item.value("children", std::vector<int>());
            if (item.contains("matrix"))
            {
                glm::mat4 M = glm::make_mat4(item["matrix"].get<std::vector<float>>().data());

                glm::vec3 skew;
                glm::vec4 persp;
                glm::vec3 scale, translation;
                glm::quat rotation;

                glm::decompose(M, scale, rotation, translation, skew, persp);
                rotation = glm::conjugate(rotation);

                n.Translation = translation;
                n.Rotation    = rotation;
                n.Scale       = scale;
                continue;
            }
            if (item.contains("translation"))
            {
                n.Translation = glm::make_vec3(item["translation"].get<std::vector<float>>().data());
            }
            if (item.contains("rotation"))
            {
                n.Rotation = glm::make_quat(item["rotation"].get<std::vector<float>>().data());
            }
            if (item.contains("scale"))
            {
                n.Scale = glm::make_vec3(item["scale"].get<std::vector<float>>().data());
            }
        }

        for (auto& scene : _scenes)
        {
            auto nodes = scene.GetNodes(this);
            for (auto node : nodes)
            {
                node->InitTransform(this);
            }
        }

        // _meshes;
        json meshes = root["meshes"];
        for (json const& item : meshes)
        {
            _meshes.emplace_back();
            GLTF::Mesh& mesh = _meshes.back();
            json primitives = item["primitives"];
            for (json const& pr : primitives)
            {
                mesh.Primitives.emplace_back();
                GLTF::Mesh::Primitive& p = mesh.Primitives.back();
                json attrs = pr["attributes"];
                for (auto const& attr : attrs.items())
                {
                    if (!p.Attributes.contains(attr.key()))
                    {
                        p.Attributes.emplace(attr.key(), attr.value());
                    }
                }
                p.Indices = pr.value("indices", 0);
                if (pr.contains("targets"))
                {
                    json targets = pr["targets"];
                    for (json const& targObj : targets)
                    {
                        p.Targets.emplace_back();
                        auto& target = p.Targets.back();
                        for (auto const& el : targObj.items())
                        {
                            target[el.key()] = el.value();
                        }
                    }
                }
                p.Material = pr.value("material", -1);
            }
            if (item.contains("weights"))
            {
                mesh.Weights = item["weights"].get<std::vector<float>>();
            }
        }
        // _buffers;
        json buffers = root["buffers"];
        for (json const& item : buffers)
        {
            _buffers.emplace_back();
            GLTF::Buffer& buffer = _buffers.back();
            buffer.URI = item["uri"];
            buffer.ByteLength = item["byteLength"];
        }
        // _bufferViews;
        json bufferViews = root["bufferViews"];
        for (json const& item : bufferViews)
        {
            _bufferViews.emplace_back();
            GLTF::BufferView& b = _bufferViews.back();
            b.Buffer = item.value("buffer", 0);
            b.ByteOffset = item.value("byteOffset", 0);
            b.ByteStride = item.value("byteStride", -1);
            b.ByteLength = item.value("byteLength", 0);
            b.Target = (GLTF::BufferView::TargetType)item.value("target", (int)GLTF::BufferView::TargetType::UNKNOWN);
        }
        // _accessors;
        json accessors = root["accessors"];
        for (json const& item : accessors)
        {
            _accessors.emplace_back();
            GLTF::Accessor& acc = _accessors.back();
            acc.BufferView = item.value("bufferView", 0);
            acc.ComponentType = (GLTF::Accessor::ComponentTypeType)item.value("componentType", (int)GLTF::Accessor::ComponentTypeType::Float);
            acc.Count = item.value("count", 0);
            acc.Type = GLTF::Accessor::ToTypeType(item.value("type", "SCALAR"));
            acc.ByteOffset = item.value("byteOffset", 0);
        }
        // _skins;
        json skins = root["skins"];
        for (json const& item : skins)
        {
            _skins.emplace_back();
            GLTF::Skin& skin = _skins.back();
            skin.InverseBindMatrices = item.value("inverseBindMatrices", -1);
            if (item.contains("joints"))
            {
                skin.Joints = item["joints"].get<std::vector<int>>();
                std::cout << skin.Joints.size() << std::endl;
            }
        }
        for (GLTF::Skin& skin : _skins)
        {
            skin.PreWarmInvBindMats(this);
        }
        // _animations
        json animations = root["animations"];
        for (json const& item : animations)
        {
            _animations.emplace_back();
            GLTF::Anim& animation = _animations.back();
            animation.Name = item.value("name", "");

            for (auto const& ch : item["channels"])
            {
                animation.Channels.emplace_back();
                auto& c = animation.Channels.back();
                c.Sampler = ch["sampler"];
                c.Target.Node = ch["target"]["node"];
                std::string path = ch["target"]["path"];
                if (path == "translation") c.Target.Path = Anim::Channel::Target::PathType::Translation;
                else if (path == "rotation") c.Target.Path = Anim::Channel::Target::PathType::Rotation;
                else if (path == "scale") c.Target.Path = Anim::Channel::Target::PathType::Scale;
                else c.Target.Path = Anim::Channel::Target::PathType::Weights;
            }

            for (auto const& smp : item["samplers"])
            {
                animation.Samplers.emplace_back();
                auto& s = animation.Samplers.back();
                s.Input = smp["input"];
                s.Output = smp["output"];
                s.Interpolation = Anim::Sampler::ToInterpolationType(smp.value("interpolation","LINEAR"));
            }

            animation.LoadFromHDDToRAM(this);
        }
    }

    void SetShader(int material, Shader shader)
    {
        _shaders[material] = shader;
    }

    void SetVec3(std::string const& paramName, glm::vec3 v)
    {
        vec3s[paramName] = v;
    }

    void SetVec4(std::string const& paramName, glm::vec4 v)
    {
        vec4s[paramName] = v;
    }

    std::map<std::string, glm::vec3> vec3s;
    std::map<std::string, glm::vec4> vec4s;

private:
    std::vector<GLTF::Scene> _scenes;
    std::vector<GLTF::Node> _nodes;
    std::vector<GLTF::Mesh> _meshes;
    std::vector<GLTF::Skin> _skins;
    std::vector<GLTF::Buffer> _buffers;
    std::vector<GLTF::BufferView> _bufferViews;
    std::vector<GLTF::Accessor> _accessors;
    std::vector<GLTF::Anim> _animations;

    std::map<int, Shader> _shaders;
    std::string _rootFolder;

    template<typename T>
    static void CollectVertNumbersByAccessor(GLTF *self, GLTF::Accessor *acc, size_t vert, std::vector<T> &res)
    {
        auto& bv = acc->GetBufferView(self);
        auto const stride = bv.ByteStride == -1
                ? acc->SizeOfComponentType() * acc->NumberOfComponents()
                : bv.ByteStride;

        auto const offset = bv.ByteOffset + acc->ByteOffset + stride * vert;

        GLTF::Buffer& buffer = bv.GetBuffer(self);
        if (buffer.Bytes.empty())
        {
            buffer.LoadBytes(self->_rootFolder);
        }

        if (acc->ComponentType == Accessor::ComponentTypeType::Float)
        {
            auto* b = reinterpret_cast<float*>(buffer.Bytes.data() + offset);
            size_t sz = acc->NumberOfComponents();
            for (size_t i = 0; i < sz; ++i)
            {
                res.push_back(*(b+i));
            }
        }
        else if (acc->ComponentType == Accessor::ComponentTypeType::UnsignedInt)
        {
            auto* b = reinterpret_cast<unsigned int*>(buffer.Bytes.data() + offset);
            size_t sz = acc->NumberOfComponents();
            for (size_t i = 0; i < sz; ++i)
            {
                res.push_back(b[i]);
            }
        }
        else if (acc->ComponentType == Accessor::ComponentTypeType::Short)
        {
            auto* b = reinterpret_cast<short*>(buffer.Bytes.data() + offset);
            size_t sz = acc->NumberOfComponents();
            for (size_t i = 0; i < sz; ++i)
            {
                res.push_back(b[i]);
            }
        }
        else if (acc->ComponentType == Accessor::ComponentTypeType::UnsignedShort)
        {
            auto* b = reinterpret_cast<unsigned short*>(buffer.Bytes.data() + offset);
            size_t sz = acc->NumberOfComponents();
            for (size_t i = 0; i < sz; ++i)
            {
                res.push_back(b[i]);
            }
        }
        else if (acc->ComponentType == Accessor::ComponentTypeType::UnsignedByte)
        {
            auto* b = reinterpret_cast<unsigned char*>(buffer.Bytes.data() + offset);
            size_t sz = acc->NumberOfComponents();
            for(size_t i = 0; i < sz; ++i)
            {
                res.push_back(b[i]);
            }
        }
        else if (acc->ComponentType == Accessor::ComponentTypeType::Byte)
        {
            auto* b = reinterpret_cast<char*>(buffer.Bytes.data() + offset);
            size_t sz = acc->NumberOfComponents();
            for(size_t i = 0; i < sz; ++i)
            {
                res.push_back(b[i]);
            }
        }
    }
};


#endif //OPENGL_TUTORIALS_GLTF_H
