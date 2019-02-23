#include "ComplexObjectImporter.h"
#include "datastructures.h"

#include <iostream>
using namespace std;

void ComplexObjectImporter::recursiveProcess(aiNode* node,const aiScene* scene)
{
    //process
    for(unsigned int i=0; i<node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, i, scene);
    }

    std::cout << "processing recursively" << std::endl;

    //recursion
    for(unsigned int i=0; i<node->mNumChildren; i++)
    {
        recursiveProcess(node->mChildren[i],scene);
    }
}

void ComplexObjectImporter::processMesh(aiMesh* c_mesh, unsigned int _id, const aiScene* scene)
{
    std::vector<VertexData> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureData*> textures;
    aiMaterial* mat = scene->mMaterials[c_mesh->mMaterialIndex];

    MaterialData tmp_mat;
    aiColor3D tmp3D;

    // Get ambient, diffuse and specular components
    aiColor4D diffuse;
    tmp_mat.diffuse = vec4f(0.8f, 0.8f, 0.8f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
    {
        tmp_mat.diffuse.x = diffuse.r;
        tmp_mat.diffuse.y = diffuse.g;
        tmp_mat.diffuse.z = diffuse.b;
        tmp_mat.diffuse.w = 1.0f;
    }

    aiColor4D ambient;
    tmp_mat.ambient = vec4f(0.2f, 0.2f, 0.2f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &ambient))
    {
        tmp_mat.ambient.x = ambient.r;
        tmp_mat.ambient.y = ambient.g;
        tmp_mat.ambient.z = ambient.b;
        tmp_mat.ambient.w = 1.0f;
    }

    aiColor4D specular;
    tmp_mat.specular = vec4f(0.0f, 0.0f, 0.0f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &specular))
    {
        tmp_mat.specular.x = specular.r;
        tmp_mat.specular.y = specular.g;
        tmp_mat.specular.z = specular.b;
        tmp_mat.specular.w = 1.0f;
    }

    aiColor4D emission;
    tmp_mat.emissive = vec4f(0.0f, 0.0f, 0.0f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &emission))
    {
        tmp_mat.emissive.x = emission.r;
        tmp_mat.emissive.y = emission.g;
        tmp_mat.emissive.z = emission.b;
        tmp_mat.emissive.w = 1.0f;
    }

    // get shininess
    float shininess = 0.0;
    unsigned int max;
    aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS, &shininess, &max);
    tmp_mat.shininess = shininess;

    // get attributes
    char attr;
    if(c_mesh->HasPositions()) attr |= A_POS;
    if(c_mesh->HasNormals())   attr |= A_NOR;
    if(c_mesh->HasTextureCoords(0)) attr |= A_UV; // 1 texcoords set (at position 0)
    if(c_mesh->HasTangentsAndBitangents())
    {
        attr |= A_TAN;
        attr |= A_BIT;
    }
    if(c_mesh->HasVertexColors(0)) attr |= A_COL; // 1 color set at (position 0)

    for(unsigned int i=0; i<c_mesh->mNumVertices; i++)
    {
        VertexData tmp;
        vec3f tmpVec;

        //position
        tmpVec.x=c_mesh->mVertices[i].x;
        tmpVec.y=c_mesh->mVertices[i].y;
        tmpVec.z=c_mesh->mVertices[i].z;
        tmp.position=tmpVec;

        //normals
        if(c_mesh->HasNormals())
        {
            tmpVec.x=c_mesh->mNormals[i].x;
            tmpVec.y=c_mesh->mNormals[i].y;
            tmpVec.z=c_mesh->mNormals[i].z;
            tmp.normal=tmpVec;
        }

        //UVs
        if(c_mesh->mTextureCoords[0])
        {
            tmpVec.x=c_mesh->mTextureCoords[0][i].x;
            tmpVec.y=c_mesh->mTextureCoords[0][i].y;
        }
        else
        {
            tmpVec.x=tmpVec.y=tmpVec.z=0.0;
        }
        tmp.U=tmpVec.x;
        tmp.V=tmpVec.y;

        //tangent
        if(c_mesh->mTangents)
        {
            tmpVec.x=c_mesh->mTangents[i].x;
            tmpVec.y=c_mesh->mTangents[i].y;
            tmpVec.z=c_mesh->mTangents[i].z;
        }
        else
        {
            tmpVec.x=1.0;           // dont know why these
            tmpVec.y=tmpVec.z=0;
        }
        tmp.tangent=tmpVec;

        // bitangent
        if(c_mesh->mBitangents)
        {
            tmpVec.x=c_mesh->mBitangents[i].x;
            tmpVec.y=c_mesh->mBitangents[i].y;
            tmpVec.z=c_mesh->mBitangents[i].z;
        }
        else
        {
            tmpVec.x=0.0;
            tmpVec.y=1.0;
            tmpVec.z=0.0; // dont know why these
        }

        //colors
        if(c_mesh->mColors[0])
        {
            //!= material color
            tmpVec.x=c_mesh->mColors[0][i].r;
            tmpVec.y=c_mesh->mColors[0][i].g;
            tmpVec.z=c_mesh->mColors[0][i].b;
        }
        else  tmpVec=vec3f(1.0f, 1.0f, 1.0f); // if no colors specifically set, use white
        tmp.color=vec4f(tmpVec.x, tmpVec.y, tmpVec.z, 1.0f);

        vertices.push_back(tmp);
    }

    // get indices
    for(unsigned int i=0; i<c_mesh->mNumFaces; i++)
    {
        aiFace face=c_mesh->mFaces[i];
        for(unsigned int j=0; j<face.mNumIndices; j++) //0..2
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // get textures of that mesh
    for(unsigned int i=0; i<mat->GetTextureCount(aiTextureType_DIFFUSE); i++)
    {
        if(i<0) cout << "object " << _id << " has muptiple textures" << endl;
        aiString str;
        mat->GetTexture(aiTextureType_DIFFUSE, i, &str);
        TextureData* texdata = new TextureData();
        if(!loadTexture(texdata, str.C_Str()))
            cout << "loadTexture failed: " << str.C_Str() << endl;
        else
            textures.push_back(texdata);
    }

    meshes.push_back(new MeshData(_id, attr, tmp_mat,
                                  &vertices, &indices, &textures));
}

bool ComplexObjectImporter::loadTexture(TextureData* _texdata, const char* filename)
{
    SDL_Surface *surface;
    int nOfColors;

    if(surface = IMG_Load(filename))
    {
        // Check that the image's width is a power of 2
        if ( (surface->w & (surface->w - 1)) != 0 )
            printf("warning: %s's width is not a power of 2\n", filename);

        // Also check if the height is a power of 2
        if ( (surface->h & (surface->h - 1)) != 0 )
            printf("warning: %s's height is not a power of 2\n", filename);

        // get the number of channels in the SDL surface
        nOfColors = surface->format->BytesPerPixel;

        if(nOfColors != 4 && nOfColors != 3) //3:GL_RGB, 4:GL_RGBA
            printf("warning: the image is not truecolor..  this will probably break\n");
        // this error should not go unhandled

        _texdata->numofcolors = nOfColors;
        _texdata->width = surface->w;
        _texdata->height = surface->h;
        _texdata->AssignRawPixels((unsigned char*)surface->pixels);

        printf("SDL loaded %s successfully\n", filename);
    }
    else
    {
        printf("SDL could not load %s: %s\n", filename, SDL_GetError());
        SDL_Quit();
        return false;
    }

    // Free the SDL_Surface only if it was successfully created
    if (surface)
        SDL_FreeSurface(surface);

    return true;
}

bool ComplexObjectImporter::LoadComplexObjectFromFile(const char* filename)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_GenSmoothNormals |
                           aiProcess_Triangulate |
                           aiProcess_CalcTangentSpace |
                           aiProcess_JoinIdenticalVertices |
                           aiProcess_FlipUVs);
    if(!scene)
    {
        std::cout << "invalid file " << filename << std::endl;
        return false;
    }
    else
        std::cout << "valid file " << filename << std::endl;

    // if no vertex positions are set or there is no root node
    if(scene->mFlags==AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "The file was not successfuly opened " << filename << std::endl
                  << "(vertex positions not set or no valid RootNode)" << std::endl;
        return false;
    }

    recursiveProcess(scene->mRootNode, scene);
}

ComplexObjectImporter::~ComplexObjectImporter()
{
    for(int i=0; i<meshes.size(); i++)
        delete meshes[i];
}


std::vector<MeshData*>& ComplexObjectImporter::GetMeshes()
{
    return meshes;
}
