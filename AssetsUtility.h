#ifndef OPENGL_TUTORIALS_ASSETSUTILITY_H
#define OPENGL_TUTORIALS_ASSETSUTILITY_H
#include <string>
#include <fstream>
#include <sstream>

struct AssetsUtility {

    static std::string LoadAllTextFromFile(std::string path)
    {
        std::replace(std::begin(path), std::end(path), '\\', '/');
        std::ifstream vertFile(path);
        std::ostringstream sstr = {};
        sstr << vertFile.rdbuf();
        return sstr.str();
    }

};

#endif //OPENGL_TUTORIALS_ASSETSUTILITY_H
