#ifndef OPENGL_TUTORIALS_GPUUTILITY_H
#define OPENGL_TUTORIALS_GPUUTILITY_H

#include <glad/glad.h>
#include <vector>

struct GPUUtility {
    static void SendMat4ToGPU(int uniformId, std::vector<float>& mats)
    {
        glUniformMatrix4fv(uniformId, mats.size() / 16, GL_FALSE, mats.data());
    }
};

#endif //OPENGL_TUTORIALS_GPUUTILITY_H
