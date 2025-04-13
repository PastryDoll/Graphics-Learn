
#ifndef MODEL_H
#define MODEL_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

#define MAX_TEXTURES 64

#define ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices |    \
    aiProcess_Triangulate |              \
    aiProcess_GenSmoothNormals |         \
    aiProcess_LimitBoneWeights |         \
    aiProcess_SplitLargeMeshes |         \
    aiProcess_ImproveCacheLocality |     \
    aiProcess_RemoveRedundantMaterials | \
    aiProcess_FindDegenerates |          \
    aiProcess_FindInvalidData |          \
    aiProcess_GenUVCoords |              \
    aiProcess_CalcTangentSpace |         \
    aiProcess_FlipUVs)

struct Model 
{
    Mesh* meshes;
    int numMeshes;
    int textures_loaded_count = 0;
    Texture textures_loaded[MAX_TEXTURES];
    char directory[256];
};

void DrawModel(Model* model, Shader* shader)
{
    for(int mesh_idx = 0; mesh_idx < model->numMeshes; mesh_idx++)
    {
        Mesh* currMesh = model->meshes + mesh_idx;
        activateMesh(currMesh, shader);
        drawMesh(currMesh, shader);
    }

}  

int loadMaterialTextures(aiMaterial *mat, aiTextureType type, int texture_type, Texture *out_textures,  Model *model)
{
    int count = 0;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for(int j = 0; j < model->textures_loaded_count; j++)
        {
            if (strcmp(model->textures_loaded[j].path, str.C_Str()) == 0)
            {
                out_textures[count++] = model->textures_loaded[j];
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture =  createTextureFromFile(str.C_Str(), model->directory,texture_type, true);
            strncpy_s(texture.path, sizeof(texture.path), str.C_Str(), _TRUNCATE);

            out_textures[count++] = texture;
            if (model->textures_loaded_count < MAX_TEXTURES)
            {
                model->textures_loaded[model->textures_loaded_count++] = texture;
            }
        }
    }
    return count;
}

Mesh processMesh(aiMesh *mesh, const aiScene *scene, Model* model)
    {
        Vertex* vertices = (Vertex *)malloc(mesh->mNumVertices * sizeof(Vertex));
        unsigned int * indices = (unsigned int *)malloc(mesh->mNumFaces * 3 * sizeof(unsigned int)); // Assuming triangular faces
        unsigned int numVertices = mesh->mNumVertices;
        unsigned int numIndices = mesh->mNumFaces * 3; //  * Together with the #aiProcess_Triangulate flag you can then be sure that
                                                       //  * #aiFace::mNumIndices is always 3.

        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; 
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices[i]= vertex;
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        unsigned int index = 0;
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices[index++] = face.mIndices[j];        
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN
        Texture* textures = (Texture *)malloc(MAX_TEXTURES * sizeof(Texture));
        int textureCount = 0;
        textureCount += loadMaterialTextures(material, aiTextureType_DIFFUSE, TEXTURE_DIFFUSE, textures + textureCount,model);
        textureCount += loadMaterialTextures(material, aiTextureType_SPECULAR, TEXTURE_SPECULAR, textures + textureCount,model);
        textureCount += loadMaterialTextures(material, aiTextureType_HEIGHT, TEXTURE_NORMAL, textures + textureCount,model);
        textureCount += loadMaterialTextures(material, aiTextureType_AMBIENT, TEXTURE_HEIGHT, textures + textureCount,model);

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, numVertices, indices, numIndices, textures, textureCount);
    }


void processNode(aiNode *node, const aiScene *scene, Model *model)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        model->meshes[model->numMeshes++] = processMesh(mesh, scene, model);			
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, model);
    }
}  

Model* ModelInit(const char* path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, ASSIMP_LOAD_FLAGS);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("ERROR::ASSIMP::%s\n", import.GetErrorString());
        return NULL;
    }

    Model* model = (Model*)malloc(sizeof(Model));
    if (!model) {
        printf("ERROR::Failed to allocate Model\n");
        return NULL;
    }

    model->textures_loaded_count = 0;

    const char* last_slash = strrchr(path, '/');
    if (last_slash != NULL) {
        size_t length = last_slash - path;
        strncpy_s(model->directory, sizeof(model->directory), path, length);
        model->directory[length] = '\0';
    } else {
        model->directory[0] = '\0';
    }
    model->numMeshes = scene->mNumMeshes;
    model->meshes = (Mesh*)malloc(sizeof(Mesh) * model->numMeshes);
    if (!model->meshes) {
        printf("ERROR::Failed to allocate Meshes\n");
        free(model);
        return NULL;
    }

    model->numMeshes = 0;
    processNode(scene->mRootNode, scene, model);

    return model;
}
#endif
