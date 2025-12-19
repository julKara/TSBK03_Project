
// Assimp headers
#include <assimp/Importer.hpp>      // C++ importer
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post-processing flags

// C++ headers
#include <iostream>
#include <string>
#include <map>
#include <assert.h>

// Others from Include - folder
#include <glew.h>
#include <GLFW/glfw3.h>


// Self-made Headers
#include "input_controller.h"
#include "shader.h"


// CONSTANTS & MACROS
float gLastTime = 0.0f;

#define MAX_NUM_BONES_PER_VERTEX 4  // ADJUSTABLE

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))  //

// INSTANCES of self-made classes
InputController input;
Shader* weightShader = nullptr;


// INSTANCES of other classes
GLFWwindow* gWindow = nullptr;



// ------------------------- STRUCTURES -------------------------

// Stores vertex-bone data, unique per vertex (boneId, weighs affecting the bone)
struct VertexBoneData
{
    // Bone-data arrays: (bone-id;s, weights for each bone)
    unsigned int BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 }; // Can only store 0 and positive nr;s
    float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };

    // Constructor
    VertexBoneData()
    {
    }

    // Fills upp bone-data arrays for this bone
    void AddBoneData(unsigned int boneID, float weight)
    {
        // Go though BoneIDs array and fill it
        for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(BoneIDs); i++) {
            
            // If no current data - fill it
            if (Weights[i] == 0.0) {
                
                // Add data
                BoneIDs[i] = boneID;
                Weights[i] = weight;

                printf("\t\t\t\tBone-id %d weight %f boneIDs-index %i\n", boneID, weight, i);
                return;
            }
        }

        // Should never get here, triggers for bugs or if more than MAX_NUM_BONES_PER_VERTEX
        assert(0);
    }
};

// Other structures: Mapping from vertices to the bones that influece them
std::vector<VertexBoneData> vertex_to_bones;                    // Mapping from vertices to the bones that influece them
std::vector<int> mesh_base_vertex;                              // Stores all start-vertices of all meshes: Mesh 1 starts at index 0, Mesh 2 starts at index N (N = sizeof(Mesh 1))...
std::map<std::string, unsigned int> bone_name_to_index_map;     // Mapping of bone-name to index, Assimp uses strings for names otherwise, effective for getting bone-ids


// ------------------------- UTIL -------------------------

// Returns bone-id of input-bone and adds bones to bone_name_to_index_map
int get_bone_id(const aiBone* pBone)
{
    int bone_id = 0;
    std::string bone_name(pBone->mName.C_Str());

    // If bone-name is not in map, add it
    if (bone_name_to_index_map.find(bone_name) == bone_name_to_index_map.end()) {
        
        bone_id = (int)bone_name_to_index_map.size(); // bone-id becomes the current size of map
        bone_name_to_index_map[bone_name] = bone_id;    // "Connect" name to id via map
    }
    else {  // If bone is in map, return id of input-bone's name

        bone_id = bone_name_to_index_map[bone_name];
    }

    return bone_id;
}

// Parses a single bone - fills bone_name_to_index_map and vertex_to_bones
void parse_single_bone(int mesh_index, const aiBone* pBone)
{
    // Print bone-info: indes, its name (C-string, null char at the end) and number of weights affecting it
    //printf("\t\t Bone %d: '%s' num vertices affected by this bone: %d\n", bone_index, pBone->mName.C_Str(), pBone->mNumWeights);  // OLD

    printf("\t\tBone '%s': num vertices affected by this bone: %d\n", pBone->mName.C_Str(), pBone->mNumWeights);

    // Get bone-id of input-bone while adding bone to bone_name_to_index_map
    int bone_id = get_bone_id(pBone);
    printf("\t\tbone id %d\n", bone_id);
    
    // Loop through all weights for this bone
    for (unsigned int i = 0; i < pBone->mNumWeights; i++) {
        
        if (i == 0) printf("\n");   // Skip first

        const aiVertexWeight& vw = pBone->mWeights[i];  // Influence of this bone on a vertex

        unsigned int global_vertex_id = mesh_base_vertex[mesh_index] + vw.mVertexId; // Vertex-id becomes: N (index of first vert in current mesh) + index of vert influenced by bone
        printf("\t\t\tVertex id %d \n", global_vertex_id);    // Global index = base index + local mesh index

        // Assert if possible to add bone-data to vertex_to_bones and then do that
        assert(global_vertex_id < vertex_to_bones.size());
        vertex_to_bones[global_vertex_id].AddBoneData(bone_id, vw.mWeight);
        
        // Print bone-info: Bone.index, Vertex-index and it weight
        //printf("\t\t %d: vertex id %d weight %.2f\n", i, vw.mVertexId, vw.mWeight);   // OLD
    }

    printf("\n");
}

// Parses all bones in a mesh
void parse_mesh_bones(int mesh_index, const aiMesh* pMesh)
{
    // Loop as many times as there are bones in mesh - prase every bone in mesh
    for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
        parse_single_bone(mesh_index, pMesh->mBones[i]);    // Connects bone with mesh-index
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

    // Resize to fit all meshes's base vertices
    mesh_base_vertex.resize(pScene->mNumMeshes);

    // Loop as many times as there are meshes in the scene
    for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
        
        // Set base vertex as index total nr of vertices before adding new ones 
        mesh_base_vertex[i] = total_vertices;
        
        // Get mesh at index i from mMeshes array (stores all meshes in the scene)
        const aiMesh* pMesh = pScene->mMeshes[i];

        // Get nr vertices, indices and bones of current mesh
        int num_vertices = pMesh->mNumVertices;
        int num_indices = pMesh->mNumFaces * 3; // Nr indices are nr of faces x3 (have already ran triangulate)
        int num_bones = pMesh->mNumBones;

        // Print info for current mesh
        printf("\tMesh %d '%s': vertices %d indices %d bones %d\n\n", i, pMesh->mName.C_Str(), num_vertices, num_indices, num_bones);
        
        // Add to total
        total_vertices += num_vertices;
        total_indices += num_indices;
        total_bones += num_bones;

        // Resize, per mesh, so it can contain all vertices's bone-data (must be done repeatitly bc parse_mesh_bone() uses it)
        vertex_to_bones.resize(total_vertices);
        
        // Test if current mesh has bones, and if so, parse them
        if (pMesh->HasBones())
            parse_mesh_bones(i, pMesh); // Conects mesh with mesh-index

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
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    gWindow = glfwCreateWindow(1280, 720, "Bone Weight Viewer", nullptr, nullptr);
    if (!gWindow) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(gWindow);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to init GLEW\n";
        return -1;
    }

    // Test openGL
    /*const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "OpenGL version: " << version << std::endl;*/
    
    weightShader = new Shader(
        "../Shaders/weight_visualization.vs",
        "../Shaders/weight_visualization.fs"
    );

    glEnable(GL_DEPTH_TEST);

    // Load model -------------------------
    // Change this to load any model in the Models folder (can handle "any" file-types)
    std::string modelName = "boblampclean.md5mesh";  // Model name: WRITE HERE

    /* Examples:
        * Vanguard.dae  // RIGGED, must change MAX_NUM_BONES_PER_VERTEX to 6
        * boblampclean.md5mesh // RIGGED, less meshes (from Doom 3)
        * spider.obj    // NOT RIGGED
        * dragon.obj    // NOT RIGGED, ONE MESH
    */

    // Test if loading succesful
    if (!loadModel(modelName)) {
        std::cerr << "Failed to load model.\n";
        return 1;
    }

    while (!glfwWindowShouldClose(gWindow)) {

        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - gLastTime;
        gLastTime = currentTime;

        glfwPollEvents();
        input.Update(deltaTime);

        int width, height;
        glfwGetFramebufferSize(gWindow, &width, &height);
        float aspect = (float)width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 modelMatrix(1.0f);

        glm::mat4 MVP =
            input.GetCamera().GetProjectionMatrix(aspect) *
            input.GetCamera().GetViewMatrix() *
            modelMatrix;

        weightShader->Use();
        weightShader->SetMat4("MVP", MVP);
        weightShader->SetInt("uSelectedBone", input.GetCurrentBoneIndex());

        // Draw call will go here

        glfwSwapBuffers(gWindow);
    }

    glfwTerminate();
    return 0;
}
