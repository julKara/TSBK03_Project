
// Assimp headers
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// C++ headers
#include <iostream>
#include <string>

// ------------------------- UTIL -------------------------


// Parses a single bone
void parse_single_bone(int bone_index, const aiBone* pBone)
{
    // Print bone-info: indes, its name (C-string, null char at the end) and number of weights affecting it
    printf("\t\t Bone %d: '%s' num vertices affected by this bone: %d\n", bone_index, pBone->mName.C_Str(), pBone->mNumWeights);

    // Loop through all weights
    for (unsigned int i = 0; i < pBone->mNumWeights; i++) {
        
        if (i == 0) printf("\n");   // Skip first

        const aiVertexWeight& vw = pBone->mWeights[i];  // Get current vertex-weight from bone
        
        // Print bone-info: Bone.index, Vertex-index and it weight
        printf("\t\t %d: vertex id %d weight %.2f\n", i, vw.mVertexId, vw.mWeight);
    }

    printf("\n");
}

// Parses all bones in a mesh
void parse_mesh_bones(const aiMesh* pMesh)
{
    // Loop as many times as there are bones in mesh - prase every bone in mesh
    for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
        parse_single_bone(i, pMesh->mBones[i]);
    }
}

// Counts number of vertices, their indices and bones per mesh, and prints them out
void parse_meshes(const aiScene* pScene)
{
    printf("*******************************************************\n");
    printf("Parsing %d meshes\n\n", pScene->mNumMeshes);    // Print total nr of meshes

    int total_vertices = 0;
    int total_indices = 0;
    int total_bones = 0;

    // Loop as many times as there are meshes in the scene
    for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
        
        // Get mesh at index i from mMeshes array (stores all meshes in the scene)
        const aiMesh* pMesh = pScene->mMeshes[i];

        // Get nr vertices, indices and bones of current mesh
        int num_vertices = pMesh->mNumVertices;
        int num_indices = pMesh->mNumFaces * 3; // Nr indices are nr of faces x3 (have already ran triangulate)
        int num_bones = pMesh->mNumBones;

        // Print info for current mesh
        printf("  Mesh %d '%s': vertices %d indices %d bones %d\n\n", i, pMesh->mName.C_Str(), num_vertices, num_indices, num_bones);

        // Add to total
        total_vertices += num_vertices;
        total_indices += num_indices;
        total_bones += num_bones;

        // Test if current mesh has bones, and if so, parse them
        if (pMesh->HasBones())
            parse_mesh_bones(pMesh);

        printf("\n");
    }

    // Print total nr of nertices, indices and bones in scene
    printf("\nTotal vertices %d total indices %d total bones %d\n", total_vertices, total_indices, total_bones);
}

// Parses the file-read scene (meshes - since we will only use models FOR NOW - will handle all scene stuff)
void parse_scene(const aiScene* pScene)
{
    parse_meshes(pScene);
}

// ------------------------- LOADING -------------------------

// Load-flags for reading: Triangulate all polygons in mesh + generate normals + join identical vertices (may needed after triangulate)
#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices)

// Loading model in "Models"-folder
bool loadModel(const std::string& filename)
{
    // Build full path: Models/<filename>
    std::string fullPath = "../Models/" + filename;
    
    Assimp::Importer importer;  // Assimp importer, handles Assimp parsing
    const aiScene* pScene = importer.ReadFile(fullPath, ASSIMP_LOAD_FLAGS); // Create scene by reading file (root structure of the imported data)

    // Test scene
    if (!pScene) {
        std::cerr << "Error parsing '" << fullPath << "': "
            << importer.GetErrorString() << "\n";
        return false;
    }

    // Parse scene (only parses meshes)
    parse_scene(pScene);
    
    // Return true if succesful process
    return true;
}

// ------------------------- MAIN -------------------------
int main()
{
    // Change this to load any model in the Models folder (can handle "any" file-types)
    std::string modelName = "boblampclean.md5mesh";  // Model name: WRITE HERE

    /* Examples:
        * Vanguard.dae  // RIGGED
        * boblampclean.md5mesh // RIGGED, less meshes (from Doom 3)
        * spider.obj    // NOT RIGGED
        * dragon.obj    // NOT RIGGED, ONE MESH
    */

    // Test if loading succesful
    if (!loadModel(modelName)) {
        std::cerr << "Failed to load model.\n";
        return 1;
    }

    return 0;
}
