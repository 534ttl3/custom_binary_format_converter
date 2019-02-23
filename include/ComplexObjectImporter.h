#ifndef COMPLEXOBJECTLOADER_H
#define COMPLEXOBJECTLOADER_H

#include "datastructures.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <vector>
#include <SDL2/SDL_image.h>

class ComplexObjectImporter
{
    std::vector<MeshData*> meshes;
    void recursiveProcess(aiNode* node, const aiScene* scene);
    void processMesh(aiMesh* mesh, unsigned int _id, const aiScene* scene);
    bool loadTexture(TextureData* texdata, const char* filename);
public:
    bool LoadComplexObjectFromFile(const char* filename);
    ~ComplexObjectImporter();
    std::vector<MeshData*>& GetMeshes();
};

#endif
