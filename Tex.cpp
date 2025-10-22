#include "Tex.h"
unsigned Tex::_isTextureIdInUse[Tex::TEX_SAMPLER_COUNT]{
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,

        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,

        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,

        TEX_ID_PLACEHOLDER,
};
unsigned Tex::_isTextureOnGPU[Tex::TEX_SAMPLER_COUNT]{
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,

        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,

        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,
        TEX_ID_PLACEHOLDER,

        TEX_ID_PLACEHOLDER,
};


Tex Tex::LoadFromHDDToRAM(const std::string &path, Tex::Dimensions type)
{
    Tex self;
    self._imagePointer = stbi_load(path.c_str(), &self._width, &self._height, &self._numColCh, 0);
    self._type = type;
    self._format =
            ( self._numColCh == 1 ? Tex::Format::Red
            : self._numColCh == 3 ? Tex::Format::RGB
            : self._numColCh == 4 ? Tex::Format::RGBA
                                  : Tex::Format::RGB);
    return self;
}

Tex Tex::LoadFromHDDToGPU(const std::string &path,
                           unsigned int samplerNumber,
                           Tex::Dimensions type,
                           bool genMipMap,
                           int lod)
{
    return LoadFromHDDToRAM(path, type)
            .SendToGPU(samplerNumber, genMipMap, lod)
            .ClearAtRAM();
}

Tex &Tex::Use()
{
    glBindTexture(_type, _texId);
    _isTextureIdInUse[_samplerNumber] = _texId;
    return *this;
}

Tex &Tex::SendToGPU(bool genMipMap, int lod)
{
    SendToGPU(_samplerNumber, genMipMap, lod);
    return *this;
}

Tex &Tex::SendToGPU(unsigned int samplerNumber, bool genMipMap, int lod)
{
    glGenTextures(1, &_texId);
    _samplerNumber = samplerNumber;
    glActiveTexture(SamplerNumberToFlag(_samplerNumber));
    glBindTexture(_type, _texId);
    glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, _onMin);
    glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, _onMax);
    glTexParameteri(_type, GL_TEXTURE_WRAP_S, _wrapX);
    glTexParameteri(_type, GL_TEXTURE_WRAP_T, _wrapY);
    if (_type == Tex::Dimensions::_2D)
    {
        glTexImage2D(_type, lod, _gpuFormat, _width, _height, 0, _format, GL_UNSIGNED_BYTE, _imagePointer);
    }
    if (genMipMap)
    {
        glGenerateMipmap(_type);
    }
    _isTextureOnGPU[_samplerNumber] = _texId;
    return *this;
}

Tex &Tex::ApplyTo(const Shader &shader, const std::string &parameterName)
{
    glUniform1i(shader.GetUniformId(parameterName), _samplerNumber);
    return *this;
}

Tex &Tex::ClearAtRAM()
{
    if (_imagePointer == nullptr)
    {
        return *this;
    }
    stbi_image_free(_imagePointer);
    _imagePointer = nullptr;
    return *this;
}

Tex &Tex::ClearAtGPU()
{
    glDeleteTextures(1, &_texId);
    _isTextureIdInUse[_samplerNumber] = Tex::TEX_ID_PLACEHOLDER;
    _isTextureOnGPU[_samplerNumber] = Tex::TEX_ID_PLACEHOLDER;
    return *this;
}

Tex &Tex::ChangeWrap(Tex::WrapType wrapX, Tex::WrapType wrapY)
{
    _wrapX = wrapX;
    _wrapY = wrapY;
    return *this;
}

Tex &Tex::ChangeResizeStrategy(Tex::ResizeStrategy onMin, Tex::ResizeStrategy onMax)
{
    _onMin = onMin;
    _onMax = onMax;
    return *this;
}

Tex &Tex::ChangeFormatAtCPU(Tex::Format format)
{
    _format = format;
    return *this;
}

Tex &Tex::ChangeFormatAtGPU(Tex::Format format)
{
    _gpuFormat = format;
    return *this;
}

unsigned Tex::SamplerNumberToFlag(unsigned int number)
{
    switch (number)
    {
        case 0  : return GL_TEXTURE0;  break;
        case 1  : return GL_TEXTURE1;  break;
        case 2  : return GL_TEXTURE2;  break;
        case 3  : return GL_TEXTURE3;  break;
        case 4  : return GL_TEXTURE4;  break;
        case 5  : return GL_TEXTURE5;  break;
        case 6  : return GL_TEXTURE6;  break;
        case 7  : return GL_TEXTURE7;  break;
        case 8  : return GL_TEXTURE8;  break;
        case 9  : return GL_TEXTURE9;  break;
        case 10 : return GL_TEXTURE10; break;
        case 11 : return GL_TEXTURE11; break;
        case 12 : return GL_TEXTURE12; break;
        case 13 : return GL_TEXTURE13; break;
        case 14 : return GL_TEXTURE14; break;
        case 15 : return GL_TEXTURE15; break;
        case 16 : return GL_TEXTURE16; break;
        case 17 : return GL_TEXTURE17; break;
        case 18 : return GL_TEXTURE18; break;
        case 19 : return GL_TEXTURE19; break;
        case 20 : return GL_TEXTURE20; break;
        case 21 : return GL_TEXTURE21; break;
        case 22 : return GL_TEXTURE22; break;
        case 23 : return GL_TEXTURE23; break;
        case 24 : return GL_TEXTURE24; break;
        case 25 : return GL_TEXTURE25; break;
        case 26 : return GL_TEXTURE26; break;
        case 27 : return GL_TEXTURE27; break;
        case 28 : return GL_TEXTURE28; break;
        case 29 : return GL_TEXTURE29; break;
        case 30 : return GL_TEXTURE30; break;
        default : break;
    }
    return GL_TEXTURE31;
}
