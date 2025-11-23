#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>

// ------------------------- Existing Code -------------------------

void parse_single_bone(int bone_index, const aiBone* pBone)
{
    printf("      Bone %d: '%s' num vertices affected by this bone: %d\n",
        bone_index, pBone->mName.C_Str(), pBone->mNumWeights);

    for (unsigned int i = 0; i < pBone->mNumWeights; i++) {
        if (i == 0) printf("\n");
        const aiVertexWeight& vw = pBone->mWeights[i];
        printf("       %d: vertex id %d weight %.2f\n",
            i, vw.mVertexId, vw.mWeight);
    }

    printf("\n");
}

void parse_mesh_bones(const aiMesh* pMesh)
{
    for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
        parse_single_bone(i, pMesh->mBones[i]);
    }
}

void parse_meshes(const aiScene* pScene)
{
    printf("*******************************************************\n");
    printf("Parsing %d meshes\n\n", pScene->mNumMeshes);

    int total_vertices = 0;
    int total_indices = 0;
    int total_bones = 0;

    for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
        const aiMesh* pMesh = pScene->mMeshes[i];

        int num_vertices = pMesh->mNumVertices;
        int num_indices = pMesh->mNumFaces * 3;
        int num_bones = pMesh->mNumBones;

        printf("  Mesh %d '%s': vertices %d indices %d bones %d\n\n",
            i, pMesh->mName.C_Str(), num_vertices, num_indices, num_bones);

        total_vertices += num_vertices;
        total_indices += num_indices;
        total_bones += num_bones;

        if (pMesh->HasBones())
            parse_mesh_bones(pMesh);

        printf("\n");
    }

    printf("\nTotal vertices %d total indices %d total bones %d\n",
        total_vertices, total_indices, total_bones);
}

void parse_scene(const aiScene* pScene)
{
    parse_meshes(pScene);
}

// ---------------------------------------------------------------

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices)

// ---------------------------------------------------------------
//            NEW FUNCTION TO LOAD ANY MODEL BY NAME
// ---------------------------------------------------------------

bool loadModel(const std::string& filename)
{
    // Build full path: Models/<filename>
    std::string fullPath = "../Models/" + filename;
    
    Assimp::Importer importer;
    const aiScene* pScene = importer.ReadFile(fullPath, ASSIMP_LOAD_FLAGS);

    if (!pScene) {
        std::cerr << "Error parsing '" << fullPath << "': "
            << importer.GetErrorString() << "\n";
        return false;
    }

    parse_scene(pScene);
    return true;
}

// ---------------------------------------------------------------
//                     Updated main()
// ---------------------------------------------------------------

int main()
{
    // Change this to load any model in the Models folder
    std::string modelName = "boblampclean.md5mesh";  // Model name

    /* Examples:
        * Vanguard.dae  // RIGGED
        * boblampclean.md5mesh // RIGGED, less meshes
        * spider.obj    // NOT RIGGED
        * dragon.obj    // NOT RIGGED, ONE MESH
    */

    if (!loadModel(modelName)) {
        std::cerr << "Failed to load model.\n";
        return 1;
    }

    return 0;
}
