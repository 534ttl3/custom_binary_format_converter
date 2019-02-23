#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <iostream>
#include <vector>
#include <cstdint>
using namespace std;

#define MAX_NUMOF_ATTRIBUTES 6

// attributes

const unsigned char A_POS = 0b00000001;
const unsigned char A_NOR = 0b00000010;
const unsigned char A_UV  = 0b00000100;
const unsigned char A_TAN = 0b00001000;
const unsigned char A_BIT = 0b00010000;
const unsigned char A_COL = 0b00100000;

const unsigned char POS                 = A_POS;
const unsigned char POS_COL             = A_POS | A_COL;
const unsigned char POS_NOR             = A_POS | A_NOR;
const unsigned char POS_NOR_COL         = A_POS | A_NOR | A_COL;
const unsigned char POS_NOR_UV          = A_POS | A_NOR | A_UV;
const unsigned char POS_NOR_UV_TAN_BIT  = A_POS | A_NOR | A_UV | A_TAN | A_BIT;

struct vec3f
{
    vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }
    vec3f() { }
    float x, y, z;
};

struct vec4f
{
    vec4f(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) { }
    vec4f() { }
    float x, y, z, w;
};


// different vertexdata structures
struct VertexData
{
    vec3f position;
    vec3f normal;
    float U, V;
    vec3f tangent;
    vec3f bitangent;
    vec4f color;
};

struct MaterialData
{
    vec4f diffuse;
    vec4f ambient;
    vec4f specular;
    float shininess;
    vec4f emissive;
};

struct TextureData
{
    TextureData()
    {
        numofcolors = 0; width = 0; height = 0; rawpixels = nullptr;
    }

    // call this after numofcolors, width and height are set
    void AssignRawPixels(unsigned char* _pixels)
    {
        if(numofcolors==0 || height==0 || width==0)
        {
            cout << "Error AllocatePixelMemory(): "
                 << " numofcolors=" << numofcolors
                 << " height=" << height
                 << " width=" << width << endl;
        }

        rawpixels = new unsigned char[width*height*numofcolors];

        for(unsigned int i=0; i<width*height*numofcolors; i++)
        {
            rawpixels[i] = _pixels[i];
            if(i < 20)
                cout << (int) rawpixels[i] << " ";
        }
        cout << endl;
    }

    ~TextureData()
    {
        delete rawpixels;
    }

    uint32_t numofcolors;
    uint32_t width;
    uint32_t height;
    unsigned char* rawpixels;
};

struct MeshData
{
    MeshData(unsigned int _id, const unsigned char attrib_flags, MaterialData& _mat,
             std::vector<VertexData>* vertexdata,
             std::vector<unsigned int>* indexdata,
             std::vector<TextureData*>* texturedata=nullptr)
    {
        vertices=*vertexdata;
        indices=*indexdata;
        if(texturedata)
            textures=*texturedata;
        id = _id;

        mat_data = _mat;
        attributes = attrib_flags;

        cout << "number of vertices in MeshData: " << vertices.size() << endl;
        cout << "number of indices  in MeshData: " << indices.size() << endl;
        cout << "number of textures in MeshData: " << textures.size() << endl;
    }

    ~MeshData()
    {
        for(unsigned int i=0; i<textures.size(); i++)
            delete textures[i];
    }

    unsigned char attributes; // bitwise selection
    MaterialData mat_data;
    unsigned int id;
    std::vector<VertexData> vertices;
    std::vector<TextureData*> textures;
    std::vector<unsigned int> indices;
};


#endif // DATASTRUCTURES_H
