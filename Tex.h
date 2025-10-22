#ifndef OPENGL_TUTORIALS_TEX_H
#define OPENGL_TUTORIALS_TEX_H
#include "Shader.h"
#include "PrimitiveMesh.h"

#include "stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_access.hpp"

#include <glad/glad.h>
#include <string>

struct Tex
{
    static constexpr unsigned TEX_SAMPLER_COUNT = 31;
    static constexpr unsigned TEX_ID_PLACEHOLDER = 888888888;
    static constexpr unsigned TEX_ID_PLACEHOLDER_FOR_INIT = 888888887;

    enum Dimensions
    {
        _2D = GL_TEXTURE_2D,
    };

    enum Format
    {
        Red  = GL_RED,
        RGB  = GL_RGB,
        RGBA = GL_RGBA,
    };

    enum WrapType
    {
        Repeat         = GL_REPEAT,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        ClampToEdge    = GL_CLAMP_TO_EDGE,
        ClampToBorder  = GL_CLAMP_TO_BORDER,
    };

    enum ResizeStrategy
    {
        Nearest = GL_NEAREST,
        Linear  = GL_LINEAR,
    };

    bool IsInUse() const { return IsOnGPU() && _isTextureIdInUse[_samplerNumber] == _texId; }
    bool IsOnGPU() const { return _isTextureOnGPU[_samplerNumber] == _texId; }

    static Tex LoadFromHDDToRAM(const std::string &path, Tex::Dimensions type = Tex::Dimensions::_2D);
    static Tex LoadFromHDDToGPU(const std::string &path,
                          unsigned int samplerNumber,
                          Tex::Dimensions type = Tex::Dimensions::_2D,
                          bool genMipMap = false,
                          int lod = 0);

    static void SetInvertVerticallyOnLoad(bool invert = true) { stbi_set_flip_vertically_on_load(invert); }

    Tex& Use();
    Tex& SendToGPU(bool genMipMap = false, int lod = 0);
    Tex& SendToGPU(unsigned int samplerNumber, bool genMipMap = false, int lod = 0);
    Tex& ApplyTo(Shader const& shader, std::string const& parameterName);
    Tex& ClearAtRAM();
    Tex& ClearAtGPU();

    Tex& ChangeWrap(Tex::WrapType wrapX, Tex::WrapType wrapY);
    Tex& ChangeResizeStrategy(Tex::ResizeStrategy onMin, Tex::ResizeStrategy onMax);
    Tex& ChangeFormatAtCPU(Tex::Format format);
    Tex& ChangeFormatAtGPU(Tex::Format format);
private:
    static unsigned _isTextureIdInUse[TEX_SAMPLER_COUNT];
    static unsigned _isTextureOnGPU[TEX_SAMPLER_COUNT];
    unsigned _type;
    void* _imagePointer = nullptr;
    int _width;
    int _height;
    int _numColCh;
    Tex::Format _gpuFormat = Tex::Format::RGB;
    Tex::Format _format = Tex::Format::RGB;
    unsigned int _texId{ Tex::TEX_ID_PLACEHOLDER_FOR_INIT };
    unsigned int _samplerNumber{ Tex::TEX_ID_PLACEHOLDER_FOR_INIT };
    Tex::WrapType _wrapX = Tex::WrapType::Repeat;
    Tex::WrapType _wrapY = Tex::WrapType::Repeat;
    Tex::ResizeStrategy _onMin = Tex::ResizeStrategy::Linear;
    Tex::ResizeStrategy _onMax = Tex::ResizeStrategy::Nearest;

    static unsigned SamplerNumberToFlag(unsigned number);
public:

    struct TexBuffer
    {
        static TexBuffer LoadMat4FromRAMToGPU(std::vector<glm::mat4> const& data,
                                          int samplerNumber,
                                          PrimitiveMesh::DrawType drawType = PrimitiveMesh::DrawType::Dynamic)
        {
            std::vector<float> flat;
            FlatVecOfMats4(data, flat);
            return LoadFromRAMToGPU(flat, samplerNumber, drawType);
        }

        static TexBuffer LoadFromRAMToGPU(std::vector<float> const& data,
                                          int samplerNumber,
                                          PrimitiveMesh::DrawType drawType = PrimitiveMesh::DrawType::Dynamic)
        {
            return TexBuffer()
                .CreateBuffersOnGPU(data,
                                    samplerNumber,
                                    drawType);
        }

        bool IsInUse() const { return IsOnGPU() && Tex::_isTextureIdInUse[_samplerNumber] == _texId; }
        bool IsOnGPU() const { return Tex::_isTextureOnGPU[_samplerNumber] == _texId; }

        inline TexBuffer& ApplyTo(Shader const& shader, std::string const& parameterName)
        {
            return SendToGPU(shader.GetUniformId(parameterName));
        }

        TexBuffer& SendToGPU(int uniformId)
        {
            glActiveTexture(GL_TEXTURE0 + _samplerNumber);
            glBindTexture(GL_TEXTURE_BUFFER, _texId);
            glUniform1i(uniformId, _samplerNumber);
            _isTextureIdInUse[_samplerNumber] = _texId;
            return *this;
        }

        TexBuffer& SetData(std::vector<float> const& flat)
        {
            glBindBuffer(GL_TEXTURE_BUFFER, _bufferId);
            glBufferSubData(GL_TEXTURE_BUFFER, 0, flat.size() * sizeof(float), flat.data());
            return *this;
        }

        TexBuffer& SetDataAsMat4s(std::vector<glm::mat4> const& data)
        {
            std::vector<float> flat;
            FlatVecOfMats4(data, flat);
            return SetData(flat);
        }

        void Clear()
        {
            Tex::_isTextureIdInUse[_samplerNumber] = Tex::TEX_ID_PLACEHOLDER;
            Tex::_isTextureOnGPU[_samplerNumber] = Tex::TEX_ID_PLACEHOLDER;
            glDeleteBuffers(1, &_bufferId);
            glDeleteTextures(1, &_texId);
        }

    private:
        int _samplerNumber{ -1 };
        unsigned int _bufferId{ Tex::TEX_ID_PLACEHOLDER_FOR_INIT };
        unsigned int _texId{ Tex::TEX_ID_PLACEHOLDER_FOR_INIT };

        static void FlatVecOfMats4(std::vector<glm::mat4> const& mats, std::vector<float>& flat)
        {
            flat.reserve(mats.size() * 16);

            for (const glm::mat4 &item: mats)
            {
                flat.insert(flat.end(), glm::value_ptr(item), glm::value_ptr(item) + 16);
            }
        }

        TexBuffer& CreateBuffersOnGPU(std::vector<float> const& data,
                                      int samplerNumber,
                                      PrimitiveMesh::DrawType drawType)
        {
            _samplerNumber = samplerNumber;

            glGenBuffers(1, &_bufferId);
            glBindBuffer(GL_TEXTURE_BUFFER, _bufferId);
            glBufferData(GL_TEXTURE_BUFFER,
                         data.size() * sizeof(float),
                         data.data(),
                         static_cast<unsigned int>(drawType));

            glGenTextures(1, &_texId);
            glBindTexture(GL_TEXTURE_BUFFER, _texId);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _bufferId);

            Tex::_isTextureOnGPU[_samplerNumber] = _texId;
            Tex::_isTextureIdInUse[_samplerNumber] = _texId;
            return *this;
        }
    };

};

#endif //OPENGL_TUTORIALS_TEX_H
