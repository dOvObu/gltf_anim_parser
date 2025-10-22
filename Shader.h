#ifndef OPENGL_TUTORIALS_SHADER_H
#define OPENGL_TUTORIALS_SHADER_H

#include "AssetsUtility.h"

#include <glad/glad.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>


struct Shader
{
    static Shader LoadFromHDDToGPU(const std::string &commonPathPart);
    static int GetUniformIdOfCurrentShader(std::string const& name);

    bool IsReadyToUse() const { return _isReadyToUse; }
    bool IsInUsage() const { return _shaderId == _currentShaderId; }
    std::map<int, std::string> const& GetInputs() const { return _inputs; }

    void Use() const;
    int GetUniformId(std::string const& name) const;

private:
    static unsigned int _currentShaderId;
    unsigned int _shaderId;
    std::map<unsigned int, unsigned int> _shaderSteps;
    std::map<int, std::string> _inputs;
    bool _isReadyToUse = false;

    void Init()
    {
        _shaderId = glCreateProgram();
    }
    bool Attach(std::string const& path);
    bool Link();
    void LoadInputsDescription(std::string const& text);
    static std::string GetExtension(std::string const& path);
    static bool IsShaderStepCompiled(std::string const& filePath, int shaderStepId);
    static bool IsShaderLinked(int shaderId);
};

#endif //OPENGL_TUTORIALS_SHADER_H
