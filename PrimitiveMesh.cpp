#include "PrimitiveMesh.h"

PrimitiveMesh::AttributeType ParseGLTFAttributeType(std::string const& type)
{
    if (type == "VEC4")
    {
        return PrimitiveMesh::AttributeType::Vec4;
    }
    if (type == "VEC3")
    {
        return PrimitiveMesh::AttributeType::Vec3;
    }
    if (type == "VEC2")
    {
        return PrimitiveMesh::AttributeType::Vec2;
    }
    if (type == "SCALAR")
    {
        return PrimitiveMesh::AttributeType::Float;
    }

    return PrimitiveMesh::AttributeType::Float;
}

PrimitiveMesh PrimitiveMesh::LoadFromGLTF(std::string path)
{
    using json = nlohmann::json;
    using byte = unsigned char;

    PrimitiveMesh self;

    std::string text = AssetsUtility::LoadAllTextFromFile(path);

    json root = json::parse(text);
    std::string uri = root["buffers"][0]["uri"];

    std::string dir  = path.substr(0, path.find_last_of('/'));
    std::string bin = AssetsUtility::LoadAllTextFromFile(dir + '/' + uri);
    std::vector<byte> binBytes(std::begin(bin), std::end(bin));

    json accessors = root["accessors"];
    /*
    "accessors": [
        {
            "type": "VEC3",
            "count": 625,
            "byteOffset": 0,
            "bufferView": 2,
            ...
        },
        ...
    ]
    */

    json bufferViews = root["bufferViews"];
    /*
    "bufferViews": [
        {
          "byteLength": 10000,
          "byteOffset": 4956,
          "byteStride": 8,
          "target": 34962,
          "buffer": 0,
          "name": "floatBufferViews",
        },
        ...
    ]
    */

    json mesh = root["meshes"][0];
    json primitive = mesh["primitives"][0];
    json attributes = primitive["attributes"];
    /*
     "attributes":
     {
        "NORMAL": 1,
        "POSITION": 0,
        "TEXCOORD_0": 2,
        "TEXCOORD_1": 3
     }
    */

    struct AttrData {
        std::string name;
        AttributeType type;
        size_t start;
        size_t step;
        size_t end;
        size_t count;
        std::vector<float> floats;

        static AttrData Create(std::string const& name, AttributeType type, size_t offset0, size_t offset1, size_t step, size_t count)
        {
            return {
                name,
                type,
                offset0 + offset1,
                step,
                offset0 + offset1 + count * step,
                count
            };
        }
    };
    std::map<unsigned int, AttrData> attrDatas;
    size_t count;
    for (auto& [name, accessorIdx] : primitive["attributes"].items())
    {
        json accessor = accessors[(unsigned int)accessorIdx];
        if (accessor["componentType"] != GLTFValueType::Float)
        {
            std::cerr << "ERROR! gltf mesh importer works only with float type\n";
            continue;
        }
        json bufferView = bufferViews[(unsigned int)accessor["bufferView"]];
        AttributeType type = ParseGLTFAttributeType(accessor["type"]);
        attrDatas.emplace((unsigned int)accessorIdx, AttrData::Create
        (
            name,
            type,
            (size_t)bufferView.value("byteOffset", 0),
            (size_t)accessor.value("byteOffset", 0),
            (size_t)bufferView.value("byteStride", sizeof(float) * (unsigned)type),
            (size_t)accessor["count"]
        ));
        AttrData& data = attrDatas[(unsigned int)accessorIdx];
        for (size_t idx = data.start; idx < data.end; idx += data.step)
        {
            for (size_t i = 0; i < (size_t)data.type; ++i)
            {
                float value;
                memcpy(&value, binBytes.data() + idx + i*sizeof(float), sizeof(float));
                data.floats.push_back(value);
            }
        }
        count = data.count;
    }
    std::vector<AttributeType> allAttributes;
    size_t step = 0;
    for (auto& [_, v] : attrDatas)
    {
        allAttributes.push_back(v.type);
        step += (size_t)v.type;
    }

    std::vector<float> allFloats;
    if (!attrDatas.empty())
    {
        for (size_t v = 0; v < count; ++v)
        {
            for (auto&[_,data] : attrDatas)
            {
                for (size_t i = 0; i < (size_t)data.type; ++i)
                {
                    allFloats.push_back(data.floats[v * (size_t)data.type + i]);
                }
            }
        }
    }
    attrDatas.clear();

    json indicesAccessor = accessors[(unsigned int)primitive["indices"]];
    json bufferView = bufferViews[(unsigned int)indicesAccessor["bufferView"]];
    GLTFValueType valueType = indicesAccessor["componentType"];
    AttrData data = AttrData::Create
    (
        "indices",
        ParseGLTFAttributeType(indicesAccessor["type"]),
        (size_t)bufferView.value("byteOffset", 0),
        (size_t)indicesAccessor.value("byteOffset", 0),
        valueType == GLTFValueType::UnsignedInt || valueType == GLTFValueType::Float
            ? 4
            : 2,
        (size_t)indicesAccessor["count"]
    );
    std::vector<int> allIndices;
    for (size_t i = data.start; i < data.end; i += data.step)
    {
        if (valueType == GLTFValueType::UnsignedInt)
        {
            unsigned int value;
            memcpy(&value, binBytes.data() + i, sizeof(unsigned int));
            allIndices.push_back(value);
        }
        else if (valueType == GLTFValueType::UnsignedShort)
        {
            unsigned short value;
            memcpy(&value, binBytes.data() + i, sizeof(unsigned short));
            allIndices.push_back(value);
        }
        else if (valueType == GLTFValueType::Short)
        {
            short value;
            memcpy(&value, binBytes.data() + i, sizeof(short));
            allIndices.push_back(value);
        }
    }

    self.Init(allAttributes, allFloats, allIndices).SendToGPU().ClearAtRAM();
    return self;
}

PrimitiveMesh PrimitiveMesh::CreateAndLoadToGPU(const std::vector<AttributeType> &format,
                                                const std::vector<float> &vertices,
                                                const std::vector<int> &indices,
                                                PrimitiveMesh::DrawType drawType)
{
    return PrimitiveMesh()
        .Init(format, vertices, indices)
        .SendToGPU(drawType)
        .ClearAtRAM();
}

PrimitiveMesh &PrimitiveMesh::Init(const std::vector<AttributeType> &format,
                                   const std::vector<float> &vertices,
                                   const std::vector<int> &indices)
{
    _format = format;
    _vertices = vertices;
    _indices = indices;
    return *this;
}

PrimitiveMesh &PrimitiveMesh::SendToGPU(PrimitiveMesh::DrawType drawType)
{
    _vertAttributeObjectId = CreateVertexAttributeObject();
    _vertBufferObjectId = CreateVerticesBuffer(_vertices.data(), _vertices.size(), static_cast<unsigned>(drawType));
    _indexArrayBufferId = CreateIndicesBuffer(_indices.data(), _indices.size(), static_cast<unsigned>(drawType));
    _vertIndicesCount = _indices.size();
    RegisterAttributes(_format);
    _isOnGPU = true;
    return *this;
}

void PrimitiveMesh::Draw() const
{
    glBindVertexArray(_vertAttributeObjectId);
    glDrawElements(GL_TRIANGLES, _vertIndicesCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void PrimitiveMesh::Clear()
{
    ClearAtRAM();
    ClearAtGPU();
}

PrimitiveMesh& PrimitiveMesh::ClearAtRAM()
{
    _format.clear();
    _vertices.clear();
    _indices.clear();
    return *this;
}

PrimitiveMesh& PrimitiveMesh::ClearAtGPU()
{
    glDeleteVertexArrays(1, &_vertAttributeObjectId);
    glDeleteBuffers(1, &_vertBufferObjectId);
    glDeleteBuffers(1, &_indexArrayBufferId);
    _isOnGPU = false;
    return *this;
}

unsigned int PrimitiveMesh::CreateVertexAttributeObject() const
{
    unsigned int vertAttributeObjectId;
    glGenVertexArrays(1, &vertAttributeObjectId);
    glBindVertexArray(vertAttributeObjectId);
    return vertAttributeObjectId;
}


template<typename T>
unsigned PrimitiveMesh::CreateBuffer(T *array, size_t length, unsigned bufferType, unsigned drawType)
{
    unsigned int vertBufferObjectId;
    glGenBuffers(1, &vertBufferObjectId);
    glBindBuffer(bufferType, vertBufferObjectId);
    glBufferData(bufferType,
                 sizeof(T) * length,
                 array,
                 drawType);

    return vertBufferObjectId;
}

template<typename T>
unsigned PrimitiveMesh::CreateVerticesBuffer(T *array, size_t length, unsigned drawType)
{
    return CreateBuffer(array, length, GL_ARRAY_BUFFER, drawType);
}

template<typename T, size_t Length>
unsigned PrimitiveMesh::CreateVerticesBuffer(T const (&array)[Length], unsigned drawType)
{
    return CreateVerticesBuffer(array, Length, drawType);
}

template<typename T>
unsigned PrimitiveMesh::CreateIndicesBuffer(T *array, size_t length, unsigned drawType)
{
    return CreateBuffer(array, length, GL_ELEMENT_ARRAY_BUFFER, drawType);
}

template<typename T, size_t Length>
unsigned PrimitiveMesh::CreateIndicesBuffer(T const (&array)[Length], unsigned drawType)
{
    return CreateIndicesBuffer(array, Length, drawType);
}

void PrimitiveMesh::RegisterAttributes(std::vector<AttributeType> const &format)
{
    unsigned step = 0;
    for (AttributeType attr: format)
    {
        step += sizeof(float) * (int) attr;
    }

    unsigned offset = 0;
    unsigned attrIdx = 0;
    for (AttributeType attr: format)
    {
        int size = (int) attr;

        glVertexAttribPointer(attrIdx,
                              size,
                              GL_FLOAT,
                              GL_FALSE,
                              step,
                              reinterpret_cast<void *>(offset));

        glEnableVertexAttribArray(attrIdx);
        ++attrIdx;
        offset += sizeof(float) * size;
    }
}
