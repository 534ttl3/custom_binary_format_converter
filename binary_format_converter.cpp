
#include "datastructures.h"
#include "ComplexObjectImporter.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <stdlib.h>
using namespace std;

bool WriteMeshesToBinaryFile(const std::string& filename, std::vector<MeshData*>& meshes)
{
    // format:
    // 0: number of meshes (uint32_t)
    // id (uint_32), number of vertices(uint32_t), number of indices, number of textures(uint32_t) (*number of meshes)
    // MeshData (*number of meshes)
    //       => id
    //       => vertex => y/n: pos, normal, uv, tangent, bitangent, color (*number of vertices) => number of vertices*sizeof(..._VD) as offset to next
    //       => index: (uint32_t) (*number of indices)
    //       => texture: => numofcolors (uint32_t) => width (uint32_t), height(uint32_t), rawpixels (bytesize*char) (*number of textures)

    ofstream bin_out(filename.c_str(), ios::out | ios::binary);
    if(bin_out.good())
    {
        cout << "WriteMeshesToBinaryFile " << filename << endl;
        // seekp to the beginning of the file
        bin_out.seekp(0);
        // write number of meshes (uint32_t)
        uint32_t numofmeshes = meshes.size();
        bin_out.write((char*)&numofmeshes, sizeof(uint32_t));
        // now write number of vertices(uint32_t), number of indices (uint32_t), number of textures(uint32_t) (*number of meshes)

        for(unsigned int i=0; i<meshes.size(); i++)
        {
            cout << "Id[" << i << "] " << meshes[i]->id << endl;
            uint32_t writebuf = meshes[i]->id;
            bin_out.write((char*)&writebuf, sizeof(uint32_t));
            writebuf = meshes[i]->vertices.size();
            bin_out.write((char*)&writebuf, sizeof(uint32_t));
            writebuf = meshes[i]->indices.size();
            bin_out.write((char*)&writebuf, sizeof(uint32_t));
            writebuf = meshes[i]->textures.size();
            bin_out.write((char*)&writebuf, sizeof(uint32_t));

            // next write material properties
            /*
                vec4f diffuse;
                vec4f ambient;
                vec4f specular;
                float shininess;
                vec4f emissive;
            */
            cout << "writing material" << endl;
            bin_out.write((char*)&meshes[i]->mat_data, sizeof(MaterialData));

            // write vertex data of this mesh
            cout << "writing attributes: " << endl;
            if(meshes[i]->attributes & A_POS)
                cout << "A_POS set" << endl;
            if(meshes[i]->attributes & A_NOR)
                cout << "A_NOR set" << endl;
            if(meshes[i]->attributes & A_UV)
                cout << "A_UV set" << endl;
            if(meshes[i]->attributes & A_TAN)
                cout << "A_TAN set" << endl;
            if(meshes[i]->attributes & A_BIT)
                cout << "A_BIT set" << endl;
            if(meshes[i]->attributes & A_COL)
                cout << "A_COL set" << endl;

            unsigned char attr_bitmask = meshes[i]->attributes;
            bin_out.write((char*)&attr_bitmask, sizeof(unsigned char));

            cout << "writing vertex data" << endl;
            cout << "number of vertices: " << meshes[i]->vertices.size() << endl;
            for(unsigned int j=0; j<meshes[i]->vertices.size(); j++)
            {
                if(meshes[i]->attributes & A_POS)
                    bin_out.write((char*)&meshes[i]->vertices.at(j).position, sizeof(vec3f));
                if(meshes[i]->attributes & A_NOR)
                    bin_out.write((char*)&meshes[i]->vertices.at(j).normal, sizeof(vec3f));
                if(meshes[i]->attributes & A_UV)
                {
                    bin_out.write((char*)&meshes[i]->vertices.at(j).U, sizeof(float));
                    bin_out.write((char*)&meshes[i]->vertices.at(j).V, sizeof(float));
                }
                if(meshes[i]->attributes & A_TAN)
                    bin_out.write((char*)&meshes[i]->vertices.at(j).tangent, sizeof(vec3f));
                if(meshes[i]->attributes & A_BIT)
                    bin_out.write((char*)&meshes[i]->vertices.at(j).bitangent, sizeof(vec3f));
                if(meshes[i]->attributes & A_COL)
                    bin_out.write((char*)&meshes[i]->vertices.at(j).color, sizeof(vec4f));
            }

            cout << "writing index data" << endl;
            cout << "number of indices: " << meshes[i]->indices.size() << endl;

            for(unsigned int j=0; j<meshes[i]->indices.size(); j++)
            {
                bin_out.write((char*)&meshes[i]->indices.at(j), sizeof(uint32_t));
            }

            cout << "writing texture data" << endl;
            cout << "number of textures: " << meshes[i]->textures.size() << endl;

            for(unsigned int j=0; j<meshes[i]->textures.size(); j++)
            {
                cout << "numofcolors " << meshes[i]->textures.at(j)->numofcolors << endl;
                bin_out.write((char*)&meshes[i]->textures.at(j)->numofcolors, sizeof(uint32_t));

                cout << "width " << meshes[i]->textures.at(j)->width << endl;
                bin_out.write((char*)&meshes[i]->textures.at(j)->width,  sizeof(uint32_t));

                cout << "height " << meshes[i]->textures.at(j)->height << endl;
                bin_out.write((char*)&meshes[i]->textures.at(j)->height, sizeof(uint32_t));

                bin_out.write((char*)meshes[i]->textures.at(j)->rawpixels,
                              meshes[i]->textures.at(j)->width*meshes[i]->textures.at(j)->height
                              *meshes[i]->textures.at(j)->numofcolors * sizeof(unsigned char));
            }
        }
        bin_out.close();

        cout << "number of meshes: " << meshes.size() << endl;

        unsigned int numoftex = 0;
        for(unsigned int i=0; i<meshes.size(); i++)
        {
            numoftex += meshes[i]->textures.size();
        }

        cout << "absolute number of textures: " << numoftex << endl;


        return true;
    }

    cout << "file not good" << endl;
    return false;
}


bool ReadObjectFromBinFile(const std::string& filename, std::vector<MeshData*>& meshes)
{
    ifstream bin_in(filename.c_str(), ios::in | ios::binary);
    if(!bin_in.good())
    {
        cout << "bin_in not good" << endl;
        return false;
    }
    // first read the number of meshes (uint32_t)
    uint32_t numofmeshes = 0;
    bin_in.read((char*)&numofmeshes, sizeof(uint32_t));
    if(!numofmeshes)
    {
        cout << "Error ReadFileFromBinFormat: numofmeshes=" << numofmeshes << endl;
        return false;
    }
    // next read all the individual meshes
    for(unsigned int i=0; i<numofmeshes; i++)
    {
        vector<VertexData> vertices;
        vector<unsigned int> indices;
        vector<TextureData*> textures;

        uint32_t id;
        bin_in.read((char*)&id, sizeof(uint32_t));
        uint32_t numofvertices = 0;
        bin_in.read((char*)&numofvertices, sizeof(uint32_t));
        if(!numofvertices)
        {
            cout << "Error ReadFileFromBinFormat: numofvertices=" << numofvertices << endl;
            return false;
        }
        uint32_t numofindices;
        bin_in.read((char*)&numofindices, sizeof(uint32_t));
        if(!numofindices)
        {
            cout << "Error ReadFileFromBinFormat: numofindices=" << numofindices << endl;
            return false;
        }

        uint32_t numoftextures;
        bin_in.read((char*)&numoftextures, sizeof(uint32_t));
        if(numoftextures > 1)
        {
            cout << "Warning ReadFileFromBinFormat: numoftextures=" << numoftextures << endl;
            return false;
        }

        // next read material properties
        MaterialData material;

        cout << "reading material" << endl;
        bin_in.read((char*)&material.diffuse, sizeof(vec4f));
        bin_in.read((char*)&material.ambient, sizeof(vec4f));
        bin_in.read((char*)&material.specular, sizeof(vec4f));
        bin_in.read((char*)&material.shininess, sizeof(float));
        bin_in.read((char*)&material.emissive, sizeof(vec4f));

        cout << "reading vertex attributes" << endl;
        // read vertex attribute attr_bitmask
        unsigned char attr_bitmask = 0;
        bin_in.read((char*)&attr_bitmask, sizeof(unsigned char));
        if(!attr_bitmask)
        {
            cout << "Error ReadFileFromBinFormat: reading the attribute attr_bitmask failed" << endl;
            return false;
        }

        cout << "read vertex attributes:" << endl;
        if(attr_bitmask & A_POS)
            cout << "A_POS set" << endl;
        if(attr_bitmask & A_NOR)
            cout << "A_NOR set" << endl;
        if(attr_bitmask & A_UV)
            cout << "A_UV set" << endl;
        if(attr_bitmask & A_TAN)
            cout << "A_TAN set" << endl;
        if(attr_bitmask & A_BIT)
            cout << "A_BIT set" << endl;
        if(attr_bitmask & A_COL)
            cout << "A_COL set" << endl;

        cout << "reading vertices" << endl;
        cout << "number of vertices: " << numofvertices << endl;
        for(unsigned int j=0; j<numofvertices; j++)
        {
            VertexData tmpvertex;

            // fill rest with some data
            tmpvertex.position = vec3f(0.0f, 0.0f, 0.0f);
            tmpvertex.normal = vec3f(0.0f, 1.0f, 0.0f);
            tmpvertex.U = 0.0f; tmpvertex.V = 0.0f;
            tmpvertex.tangent = vec3f(1.0f, 0.0f, 0.0f);
            tmpvertex.bitangent = vec3f(0.0f, 0.0f, 1.0f);
            tmpvertex.color = vec4f(1.0f, 1.0f, 1.0f, 1.0f);

            if(attr_bitmask & A_POS)
                bin_in.read((char*)&tmpvertex.position, sizeof(vec3f));
            if(attr_bitmask & A_NOR)
                bin_in.read((char*)&tmpvertex.normal, sizeof(vec3f));
            if(attr_bitmask & A_UV)
            {
                bin_in.read((char*)&tmpvertex.U, sizeof(float));
                bin_in.read((char*)&tmpvertex.V, sizeof(float));
            }
            if(attr_bitmask & A_TAN)
                bin_in.read((char*)&tmpvertex.tangent, sizeof(vec3f));
            if(attr_bitmask & A_BIT)
                bin_in.read((char*)&tmpvertex.bitangent, sizeof(vec3f));
            if(attr_bitmask & A_COL)
                bin_in.read((char*)&tmpvertex.color, sizeof(vec4f));
            vertices.push_back(tmpvertex);
        }

        cout << "reading indices" << endl;
        cout << "number of indices:" << numofindices << endl;
        for(unsigned int i=0; i<numofindices; i++)
        {
            uint32_t index;
            bin_in.read((char*)&index, sizeof(uint32_t));
            indices.push_back(index);
        }

        cout << "reading textures" << endl;
        cout << "number of textures: " << numoftextures << endl;

        for(unsigned int j=0; j<numoftextures; j++)
        {
            TextureData* ptexdata = new TextureData();

            bin_in.read((char*)&ptexdata->numofcolors, sizeof(uint32_t));
            cout << "numofcolors " << ptexdata->numofcolors << endl;

            bin_in.read((char*)&ptexdata->width,  sizeof(uint32_t));
            cout << "width " << ptexdata->width << endl;

            bin_in.read((char*)&ptexdata->height, sizeof(uint32_t));
            cout << "height " << ptexdata->height << endl;

            unsigned char* tmppixels = new unsigned char[ptexdata->numofcolors*
                                                         ptexdata->width*
                                                         ptexdata->height];
            bin_in.read((char*)tmppixels,
                ptexdata->width*ptexdata->height*ptexdata->numofcolors*sizeof(unsigned char));

            ptexdata->AssignRawPixels(tmppixels);
            delete tmppixels;

            textures.push_back(ptexdata);
        }

        meshes.push_back(new MeshData(id, attr_bitmask, material,
                                      &vertices, &indices, &textures));
    }

    bin_in.close();

    return true;
}


int main(int argc, char *argv[])
{
    cout << "tested last under windows7 64bit, intel core i5, 6GB RAM" << endl;
    if(sizeof(char)==1         && sizeof(unsigned char)==1 && sizeof(int)==4   &&
       sizeof(unsigned int)==4 && sizeof(uint32_t)==4      && sizeof(float)==4 &&
       sizeof(double)==8       && sizeof(long)==4          && sizeof(uint16_t)==2)
    {
        cout << "checking datatypes... are the same" << endl;
    }
    else
    {
        cout << "checking datatypes... someting is different, "
             << "this will probably crash [RETURN]" << endl;

    }


    cout << "Convert File: ";
    string filename;
    getline(cin, filename);
    cout << "importing " << filename << endl;

    ComplexObjectImporter Importer;
    if(!Importer.LoadComplexObjectFromFile(filename.c_str()))
    {
        cout << "Import failed" << endl;
        getchar();
        return -1;
    }
    else
    {
        cout << "Import successful" << endl;
    }

    if(!WriteMeshesToBinaryFile((filename+string(".mybin")), Importer.GetMeshes()))
    {
        cout << "WriteMeshesToBinaryFile failed" << endl;
        getchar();
        return -1;
    }
    else
    {
        cout << "WriteMeshesToBinaryFile successful" << endl;
    }

    getchar();

    vector<MeshData*> meshes;

    if(!ReadObjectFromBinFile((filename+string(".mybin")), meshes))
    {
        cout << "ReadObjectFromBinFile failed" << endl;

        getchar();
        return -1;
    }
    else
    {
        cout << "ReadObjectFromBinFile successful" << endl;
    }

    getchar();
    SDL_Quit();
    return 0;
}
