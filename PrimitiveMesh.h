#ifndef OPENGL_TUTORIALS_PRIMITIVEMESH_H
#define OPENGL_TUTORIALS_PRIMITIVEMESH_H

#include "json.hpp"
#include "AssetsUtility.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>

struct PrimitiveMesh
{
    enum class AttributeType
    {
        Float = 1,
        Vec2  = 2,
        Vec3  = 3,
        Vec4  = 4,
    };

    enum class DrawType
    {
        Static = GL_STATIC_DRAW,
        Dynamic = GL_DYNAMIC_DRAW,
        Stream = GL_STREAM_DRAW
    };

    enum class GLTFValueType
    {
        Float         = 5126, // array of 4 unsigned char
        UnsignedInt   = 5125, // array of 4 unsigned char
        UnsignedShort = 5123, // array of 2 unsigned char
        Short         = 5122, // array of 2 unsigned char
    };

    bool IsOnGPU() { return _isOnGPU; }


    static PrimitiveMesh LoadFromGLTF(std::string path);

    static PrimitiveMesh CreateAndLoadToGPU(std::vector<AttributeType> const& format,
                                            std::vector<float>         const& vertices,
                                            std::vector<int>           const& indices,
                                            DrawType drawType = DrawType::Static);

    PrimitiveMesh& Init(std::vector<AttributeType> const& format,
                        std::vector<float>         const& vertices,
                        std::vector<int>           const& indices);

    PrimitiveMesh& SendToGPU(DrawType drawType = DrawType::Static);
    void Draw() const;
    void Clear();
    PrimitiveMesh& ClearAtRAM();
    PrimitiveMesh& ClearAtGPU();

private:
    unsigned int _vertAttributeObjectId;
    unsigned int _vertBufferObjectId;
    unsigned int _indexArrayBufferId;
    unsigned int _vertIndicesCount = 0;
    bool _isOnGPU;
    std::vector<AttributeType> _format;
    std::vector<float> _vertices;
    std::vector<int> _indices;

    unsigned int CreateVertexAttributeObject() const;

    template<typename T>                unsigned CreateBuffer        (T* array, size_t length , unsigned bufferType, unsigned drawType);
    template<typename T>                unsigned CreateVerticesBuffer(T* array, size_t length , unsigned drawType);
    template<typename T, size_t Length> unsigned CreateVerticesBuffer(T  const(&array)[Length], unsigned drawType);
    template<typename T>                unsigned CreateIndicesBuffer (T* array, size_t length , unsigned drawType);
    template<typename T, size_t Length> unsigned CreateIndicesBuffer (T  const(&array)[Length], unsigned drawType);

    static void RegisterAttributes(std::vector<AttributeType> const& format);
};

#endif //OPENGL_TUTORIALS_PRIMITIVEMESH_H
