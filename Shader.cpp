#include "Shader.h"
unsigned int Shader::_currentShaderId{ 0 };


Shader Shader::LoadFromHDDToGPU(const std::string &commonPathPart)
{
    Shader shader;
    shader.Init();
    if (!shader.Attach(commonPathPart + ".vert"))
    {
        return shader;
    }
    if (!shader.Attach(commonPathPart + ".frag"))
    {
        return shader;
    }
    shader.Link();
    return shader;
}

int Shader::GetUniformIdOfCurrentShader(std::string const& name)
{
    return glGetUniformLocation(_currentShaderId, name.c_str());
}

int Shader::GetUniformId(const std::string &name) const
{
    return glGetUniformLocation(_shaderId, name.c_str());
}

bool Shader::Attach(std::string const &path)
{
    std::string text = AssetsUtility::LoadAllTextFromFile(path);
    char const *textC = text.c_str();
    std::string ext = GetExtension(path);
    unsigned int shaderStepType = ext == ".vert"
                                  ? GL_VERTEX_SHADER
                                  : GL_FRAGMENT_SHADER;

    if (shaderStepType == GL_VERTEX_SHADER)
    {
        LoadInputsDescription(text);
    }

    unsigned shaderStepId = glCreateShader(shaderStepType);
    glShaderSource(shaderStepId, 1, &textC, NULL);
    glCompileShader(shaderStepId);
    if (!IsShaderStepCompiled(path, shaderStepId))
    {
        glDeleteShader(shaderStepId);
        return false;
    }

    glAttachShader(_shaderId, shaderStepId);
    if (_shaderSteps.contains(shaderStepType))
    {
        glDeleteShader(_shaderSteps[shaderStepType]);
    }
    _shaderSteps[shaderStepType] = shaderStepId;
    _isReadyToUse = false;
    return true;
}

bool Shader::Link()
{
    glLinkProgram(_shaderId);
    if (!IsShaderLinked(_shaderId))
    {
        return false;
    }
    for (auto const &[_, step]: _shaderSteps)
    {
        glDeleteShader(step);
    }
    _shaderSteps.clear();
    _isReadyToUse = true;
    return true;
}

void SkipWhiteSpace(std::string const& text, size_t& i)
{
    while (i < text.size() && isspace(text[i]))
    {
        ++i;
    }
}

void SkipId(std::string const& text, size_t& i)
{
    while (i < text.size() && isalnum(text[i]))
    {
        ++i;
    }
}

bool TryToFindTextPosition(std::string const& src, std::string const& text, size_t& start)
{
    if (start > src.size())
    {
        return false;
    }
    start = src.find(text, start);

    if (start == std::string::npos)
    {
        return false;
    }

    start += text.size();
    return true;
}

bool TryToSkipText(std::string const& src, std::string const& text, size_t& idx)
{
    if (!src.substr(idx).starts_with(text))
    {
        return false;
    }

    idx += text.size();
    return true;
}

bool TryToParseId(std::string const& src, std::string& id, size_t& i)
{
    while (i < src.size() && (src[i] == '_' || isalnum(src[i])))
    {
        id.push_back(src[i]);
        ++i;
    }
    return !id.empty();
}

std::string IdToGLTFAttr(std::string id)
{
    size_t i = 0;
    while (i < id.size() && id[i] == '_')
    {
        ++i;
    }
    id = id.substr(i);

    for (size_t j = 0; j < id.length(); ++j)
    {
        id[j] = tolower(id[j]);
    }

    if (id.starts_with("p"))
    {
        return "POSITION";
    }

    if (id.starts_with("t"))
    {
        return "TANGENT";
    }

    if (id.starts_with("n"))
    {
        return "NORMAL";
    }

    std::string digit;

    for (size_t j = 0; j < id.length(); ++j)
    {
        if (isdigit(id[j]))
        {
            digit += id[j];
        }
    }

    if (digit.empty())
    {
        digit = "0";
    }

    if (id.starts_with("uv")
     || id.starts_with("t")
     || id.starts_with("c"))
    {
        return "TEXCOORD_" + digit;
    }

    if (id.starts_with("j"))
    {
        return "JOINTS_" + digit;
    }

    if (id.starts_with("w"))
    {
        return "WEIGHTS_" + digit;
    }

    std::cerr << "ERROR! can't find such id: \'" << id << '\'' << std::endl;

    return "";
}

void Shader::LoadInputsDescription(std::string const& text)
{
    size_t i = 0;
    for(;;)
    {
        if (!TryToFindTextPosition(text, "layout", i)) break;
        SkipWhiteSpace(text, i);

        if (!TryToSkipText(text, "(", i)) break;
        SkipWhiteSpace(text, i);

        if (!TryToSkipText(text, "location", i)) break;
        SkipWhiteSpace(text, i);

        if (!TryToSkipText(text, "=", i)) break;
        SkipWhiteSpace(text, i);

        int locationIndex = atoi(text.c_str() + i);

        if (!TryToFindTextPosition(text, ")", i)) break;
        SkipWhiteSpace(text, i);

        SkipId(text, i);
        SkipWhiteSpace(text, i);

        std::string type;
        if (!TryToParseId(text, type, i)) break;
        SkipWhiteSpace(text, i);

        std::string id;
        if (!TryToParseId(text, id, i)) break;

        _inputs[locationIndex] = IdToGLTFAttr(id);
    }
}

void Shader::Use() const
{
    glUseProgram(_shaderId);
    _currentShaderId = _shaderId;
}

std::string Shader::GetExtension(const std::string &path)
{
    size_t last = path.find_last_of('.');
    return path.substr(last, path.length() - last);
}

bool Shader::IsShaderStepCompiled(const std::string &filePath, int shaderStepId)
{
    int success;
    char log[512];
    glGetShaderiv(shaderStepId, GL_COMPILE_STATUS, &success);
    if (success)
    {
        return true;
    }
    glGetShaderInfoLog(shaderStepId, 512, NULL, log);
    std::cerr << "Shader compilation error:\n" << "    Shader path: " << filePath << '\n' << log << '\n';
    return false;
}

bool Shader::IsShaderLinked(int shaderId)
{
    int success;
    char log[512];
    glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
    if (success)
    {
        return true;
    }
    glGetShaderInfoLog(shaderId, 512, NULL, log);
    std::cerr << "Shader linking error:\n" << log << '\n';
    return false;
}
