
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
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>     // for make_mat4
#include <glm/gtc/matrix_transform.hpp>

// "Self-made" Headers for classes
#include "input_controller.h"
#include "shader.h"
#include "skeleton.h"
#include "phyicsBone.h"


// Global variables & MACROS
float gLastTime = 0.0f;

#define MAX_NUM_BONES_PER_VERTEX 4  // ADJUSTABLE - Maximum nr of bones a single vertex can be affected by

static int space_count = 0; // Counter for printig matrices

// INSTANCES of self-made classes
InputController input;
Shader* weightShader = nullptr;
Shader* debugLineShader = nullptr;
Shader* skinningShader = nullptr;
Skeleton gSkeleton;


// INSTANCES of other classes
GLFWwindow* gWindow = nullptr;  // Globally accesssable window - used in input-controller
const aiScene* gScene = nullptr;    // Globally accessable refrence to aiScene, used for retriving model-data via assimp
GLuint gBoneVAO = 0;    // Bone-buffer input VAO
GLuint gBoneVBO = 0;    // Bone-buffer input VBO

// FLAGS
bool gUseRagdoll = false; // Must also have bonelines or normalkinning true or both

// The diffrent "modes" of the program
bool weightVisMode = false;     // Activates the weight-viz mode (need to drag model)
bool boneLinesMode = true;     // Draws the skeleton as yellow lines (can be combine w.normalSkinning)
bool normalSkinning = true;    // Normal shader for skinning



// ------------------------- STRUCTURES -------------------------

// The skeleton comprised of physics-bones instead of regular ones
struct PhysicsSkeleton
{
    std::vector<PhysicsBone> bones;
    bool enabled = false;   // Toggle rag-doll on/off
};
PhysicsSkeleton gPhysicsSkeleton;   // Global variable


// For line-rendering - for debugging bone-viz (not really necessary, but I'm to lazy to remove)
struct DebugVertex
{
    glm::vec3 position;
};


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
    void AddBoneData(unsigned int BoneID, float Weight)
    {
        // Go through all of BoneIds
        for (unsigned int i = 0; i < MAX_NUM_BONES_PER_VERTEX; i++) {
            
            // Add data if there is none
            if (Weights[i] == 0.0) {
                BoneIDs[i] = BoneID;
                Weights[i] = Weight;
                //printf("Adding bone %d weight %f at index %i\n", BoneID, Weight, i);
                return;
            }
        }

        // should never get here - more bones than we have space for
        assert(0);
    }
};

// GPU-side vertex structure, needed to "render"
struct VertexGPU
{
    glm::vec3 Position;     // Vertex position
    glm::vec3 Normal;       // Vertex normal
    glm::ivec4 BoneIDs;     // Indices of bones affecting this vertex
    glm::vec4 Weights;      // Corresponding weights
};


// Other structures: Mapping from vertices to the bones that influece them
std::vector<VertexBoneData> vertex_to_bones;                    // Mapping from vertices to the bones that influece them
std::vector<int> mesh_base_vertex;                              // Stores all start-vertices of all meshes: Mesh 1 starts at index 0, Mesh 2 starts at index N (N = sizeof(Mesh 1))...
std::vector<glm::mat4> gFinalBoneMatrices;                      // Stores transformations for final-bones used to render in vertex-shader

// Global GPU buffers and containers -----------------

// GPU data containers, needed to "render"
std::vector<VertexGPU> gpuVertices;
std::vector<unsigned int> gpuIndices;

// OpenGL object handles, needed to "render"
GLuint gVAO = 0;
GLuint gVBO = 0;
GLuint gEBO = 0;


// ------------------------- UTIL -------------------------

// Apply physics, copies results of physics to localPose in Skeleton::Bone - will be rendered
void applyPhysicsToSkeleton(const PhysicsSkeleton& physics, Skeleton& skeleton)
{
    // Go through all physics-bones
    for (const PhysicsBone& pb : physics.bones)
    {
        // Get corresponding bone from "normal" skeleton
        Bone& bone = skeleton.bones[pb.boneIndex];

        // Get translation- and rotation-matrices from physics-bone
        glm::mat4 T = glm::translate(glm::mat4(1.0f), pb.position);
        glm::mat4 R = glm::mat4_cast(pb.rotation);

        // Set local pose using translation and rotation instead of pose
        bone.localPose = T * R;
    }
}


// Build physics bones - one rigid body per bone w. same hierchy and indices as "normal" skeleton
void buildPhysicsSkeleton(const Skeleton& skeleton, PhysicsSkeleton& physics)
{
    physics.bones.clear();
    physics.bones.reserve(skeleton.bones.size());

    // Go through all bones i skeleton and copy everthing 1 to 1, initialize the rest
    for (size_t i = 0; i < skeleton.bones.size(); i++)
    {
        PhysicsBone pb;
        pb.boneIndex = (int)i;  // Index should always be int
        pb.parentIndex = skeleton.bones[i].parentIndex;

        // Initialize physics pose from bind pose
        const glm::mat4& m = skeleton.bones[i].globalPose;
        pb.position = glm::vec3(m[3]);
        pb.rotation = glm::quat_cast(m);

        physics.bones.push_back(pb);
    }
}

// Makes a line out of each bone from parent to child
void buildBoneDebugLines(const Skeleton& skeleton, std::vector<DebugVertex>& outVertices)
{
    outVertices.clear();

    // Go through all bones
    for (size_t i = 0; i < skeleton.bones.size(); i++)
    {
        const Bone& bone = skeleton.bones[i];

        // Skipp children of root
        if (bone.parentIndex == -1)
            continue;

        const Bone& parent = skeleton.bones[bone.parentIndex];

        // Take pos from parent and child
        glm::vec3 p0 = glm::vec3(parent.globalPose[3]);
        glm::vec3 p1 = glm::vec3(bone.globalPose[3]);

        // Add to outVertices, removes all faces, keeps only relation
        outVertices.push_back({ p0 });
        outVertices.push_back({ p1 });
    }
}

// Calculate FinalBoneMatrices - important for vertex-shader
void buildFinalBoneMatrices(const Skeleton& skeleton, std::vector<glm::mat4>& outMatrices)
{
    outMatrices.resize(skeleton.bones.size());

    // Go through all bones in skeleton
    for (size_t i = 0; i < skeleton.bones.size(); i++)
    {
        outMatrices[i] = skeleton.bones[i].globalPose * skeleton.bones[i].offsetMatrix; // final-bone-matrices = global-pose * offset-matrix
    }
}

// Computes the global-bone-transforms, needed for bone-world-transforms, parent-child-relationships and calculate final-bone-matrices (= global-pose * offset-matrix)
void computeGlobalBoneTransforms(Skeleton& skeleton)
{
    // Go through all bones in skeleton
    for (size_t i = 0; i < skeleton.bones.size(); i++)
    {
        Bone& bone = skeleton.bones[i];

        if (bone.parentIndex == -1)
        {
            // Root bone: local = global (doesnt have parent)
            bone.globalPose = bone.localPose;
        }
        else
        {
            // All bones that is not child of root
            const Bone& parent = skeleton.bones[bone.parentIndex];  // Set parent
            bone.globalPose = parent.globalPose * bone.localPose;   // Calculate global pose, follow chain-logic
        }
    }
}

// Initializes runtime-pose based on bind-pose
void initializeSkeletonPose(Skeleton& skeleton)
{
    // Go through all bones in skeleton
    for (Bone& bone : skeleton.bones)
    {
        // Start runtime pose equal to bind pose
        bone.localPose = bone.localBindPose;
        bone.globalPose = glm::mat4(1.0f);
    }
}

// Returns bone-id from skeleton::boneNameToIndex of input-bone or adds bone to it then return, used when parsing nodes
int get_bone_id(const aiBone* pBone)
{
    std::string bone_name(pBone->mName.C_Str());

    // Check if bone exits in list, if so return id
    auto it = gSkeleton.boneNameToIndex.find(bone_name);
    if (it != gSkeleton.boneNameToIndex.end())
        return it->second;  // Return id

    // Create new bone entry
    Bone bone;
    bone.name = bone_name;
    bone.offsetMatrix = glm::transpose(glm::make_mat4(&pBone->mOffsetMatrix.a1));
    int newIndex = (int)gSkeleton.bones.size();
    gSkeleton.bones.push_back(bone);
    gSkeleton.boneNameToIndex[bone_name] = newIndex;

    // Return index of new bone
    return newIndex;
}

// Used for printing matrices
void print_space()
{
    for (int i = 0; i < space_count; i++) {
        printf(" ");
    }
}

// Prints out matrices - used for offset-matrices
void print_assimp_matrix(const aiMatrix4x4& m)
{
    print_space(); printf("\t\t%f %f %f %f\n", m.a1, m.a2, m.a3, m.a4);
    print_space(); printf("\t\t%f %f %f %f\n", m.b1, m.b2, m.b3, m.b4);
    print_space(); printf("\t\t%f %f %f %f\n", m.c1, m.c2, m.c3, m.c4);
    print_space(); printf("\t\t%f %f %f %f\n", m.d1, m.d2, m.d3, m.d4);
}

// Normalizes bone weights per vertex so that the sum equals 1.0, prevents "seams" and incorrect heat visualization
void normalize_vertex_bone_weights()
{
    for (size_t v = 0; v < vertex_to_bones.size(); v++)
    {
        float sum = 0.0f;

        // Sum all weights for this vertex
        for (int i = 0; i < MAX_NUM_BONES_PER_VERTEX; i++)
            sum += vertex_to_bones[v].Weights[i];

        // If the vertex has any bone influence
        if (sum > 0.0f)
        {
            // Normalize weights
            for (int i = 0; i < MAX_NUM_BONES_PER_VERTEX; i++)
                vertex_to_bones[v].Weights[i] /= sum;
        }
    }
}

// ------------------------- PARSING -------------------------

// Parses a single bone - fills bone_name_to_index_map and vertex_to_bones
void parse_single_bone(int mesh_index, const aiBone* pBone)
{
    printf("\t\tBone '%s': num vertices affected by this bone: %d\n", pBone->mName.C_Str(), pBone->mNumWeights);

    // Get (and/or set) bone-id from skeleton::boneNameToIndex 
    int bone_id = get_bone_id(pBone);
    //printf("\t\tbone id %d\n", bone_id);

    // Print the offset-matrix of the current bone (vertex- to bone-coordintes or local-to-bone-space)
    printf("\t\tOffset-Matrix:\n");
    print_assimp_matrix(pBone->mOffsetMatrix);
    
    //printf("Called! %d", pBone->mNumWeights);
    
    // Loop through all weights for this bone
    for (unsigned int i = 0; i < pBone->mNumWeights; i++) {
        
        if (i == 0) printf("\n");   // Skip first

        const aiVertexWeight& vw = pBone->mWeights[i];  // Influence of this bone on a vertex

        unsigned int global_vertex_id = mesh_base_vertex[mesh_index] + vw.mVertexId; // Vertex-id becomes: N (index of first vert in current mesh) + index of vert influenced by bone
        //printf("\t\t\tVertex id %d \n", global_vertex_id);    // Global index = base index + local mesh index

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

// Parses each individual node recurivly by going down the tree.
void parse_node(const aiNode* pNode, int parentBoneIndex = -1)
{
    print_space();
    printf("Node name: '%s' num children %d num meshes %d\n",
        pNode->mName.C_Str(),
        pNode->mNumChildren,
        pNode->mNumMeshes);

    print_space();
    printf("Node transformation:\n");
    print_assimp_matrix(pNode->mTransformation);

    int currentBoneIndex = parentBoneIndex;

    // If this node corresponds to a bone, store hierarchy
    auto it = gSkeleton.boneNameToIndex.find(pNode->mName.C_Str());
    if (it != gSkeleton.boneNameToIndex.end())
    {
        currentBoneIndex = it->second;
        gSkeleton.bones[currentBoneIndex].parentIndex = parentBoneIndex;
        gSkeleton.bones[currentBoneIndex].localBindPose = glm::transpose(glm::make_mat4(&pNode->mTransformation.a1));
    }

    space_count += 4;   // Only used for printing matrices

    for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
        printf("\n");
        print_space(); printf("--- %d ---\n", i);
        parse_node(pNode->mChildren[i], currentBoneIndex);
    }

    space_count -= 4;
}

// Parses scene from the root
void parse_hierarchy(const aiScene* pScene)
{
    printf("\n**************************************************\n");
    printf("Parsing the node hierarchy\n");

    parse_node(pScene->mRootNode, -1);
}

// Parses the file-read scene, fills up many data structures
void parse_scene(const aiScene* pScene)
{
    // Go through all meshes and fill up data-arrays
    parse_meshes(pScene);

    // Go through scene-node and find all nodes and their realationships (parent-child, like blender)
    parse_hierarchy(pScene);
}

// ------------------------- LOADING & UPLOADING  -------------------------

// Upload bones as uniform attribute to shader
void uploadBoneMatrices(Shader* shader, const std::vector<glm::mat4>& matrices)
{
    // Safety clamp in case model has more bones than shader supports
    int count = (int)matrices.size();
    count = std::min(count, 128);

    // Upload bones as uniform attribute to shader
    shader->SetMat4Array("uBones", matrices.data(), count);
}

// Converts parsed Assimp + bone data into GPU-ready buffers, fills GPU vertex-data
void build_gpu_buffers(const aiScene* pScene)
{
    gpuVertices.resize(vertex_to_bones.size());

    // Go through all meshes
    for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
    {
        const aiMesh* pMesh = pScene->mMeshes[i];
        
        // Go through all vertices within current mesh
        for (unsigned int v = 0; v < pMesh->mNumVertices; v++)
        {
            unsigned int globalID = mesh_base_vertex[i] + v;

            // Copy position
            gpuVertices[globalID].Position = glm::vec3(
                pMesh->mVertices[v].x,
                pMesh->mVertices[v].y,
                pMesh->mVertices[v].z
            );

            // Copy normals
            gpuVertices[globalID].Normal = glm::vec3(
                pMesh->mNormals[v].x,
                pMesh->mNormals[v].y,
                pMesh->mNormals[v].z
            );

            // Copy bone IDs
            gpuVertices[globalID].BoneIDs = glm::ivec4(
                vertex_to_bones[globalID].BoneIDs[0],
                vertex_to_bones[globalID].BoneIDs[1],
                vertex_to_bones[globalID].BoneIDs[2],
                vertex_to_bones[globalID].BoneIDs[3]
            );

            // Copy bone weights
            gpuVertices[globalID].Weights = glm::vec4(
                vertex_to_bones[globalID].Weights[0],
                vertex_to_bones[globalID].Weights[1],
                vertex_to_bones[globalID].Weights[2],
                vertex_to_bones[globalID].Weights[3]
            );
        }

        // Build index buffer by going through all faces within current mesh
        for (unsigned int f = 0; f < pMesh->mNumFaces; f++)
        {
            const aiFace& face = pMesh->mFaces[f];
            gpuIndices.push_back(mesh_base_vertex[i] + face.mIndices[0]);
            gpuIndices.push_back(mesh_base_vertex[i] + face.mIndices[1]);
            gpuIndices.push_back(mesh_base_vertex[i] + face.mIndices[2]);
        }
    }

    // Check Assimp metadata makes it to the CPU-buffer
    /*for (int i = 0; i < 5 && i < gpuVertices.size(); i++)
    {
        printf("V[%d] = %f %f %f\n",
            i,
            gpuVertices[i].Position.x,
            gpuVertices[i].Position.y,
            gpuVertices[i].Position.z
        );
    }*/


}

// Uploads GPU vertex/index data to OpenGL bu creating VAO, VBO and EBO
void create_opengl_buffers()
{
    // Create VAO, VBO and EBO
    glGenVertexArrays(1, &gVAO);
    glGenBuffers(1, &gVBO);
    glGenBuffers(1, &gEBO);

    // Bind attributes to shader via VAO
    glBindVertexArray(gVAO);

    // Bind vertices to VBO
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        gpuVertices.size() * sizeof(VertexGPU),
        gpuVertices.data(),
        GL_STATIC_DRAW
    );

    // Bind indices to EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        gpuIndices.size() * sizeof(unsigned int),
        gpuIndices.data(),
        GL_STATIC_DRAW
    );

    // Upload attributes to shader (ordered) ----------------------

    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexGPU), (void*)0);
    glEnableVertexAttribArray(0);

    // Vertex normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexGPU),(void*)offsetof(VertexGPU, Normal));
    glEnableVertexAttribArray(1);

    // Bone IDs (integer attribute!)
    glVertexAttribIPointer(2, 4, GL_INT, sizeof(VertexGPU), (void*)offsetof(VertexGPU, BoneIDs));
    glEnableVertexAttribArray(2);

    // Bone weights
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(VertexGPU), (void*)offsetof(VertexGPU, Weights));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}


// Load-flags for reading: Triangulate all polygons in mesh + generate normals + join identical vertices (may needed after triangulate)
#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices)

// Loads model in "Models"-folder and initializes new structurs
bool loadModel(const std::string& filename)
{
    // Build full path: Models/<filename>
    std::string fullPath = "../Models/" + filename;

    Assimp::Importer importer;  // Assimp importer, handles Assimp parsing
    const aiScene* pScene = importer.ReadFile(fullPath, ASSIMP_LOAD_FLAGS); // Use flags from above

    // Test scene
    if (!pScene) {
        std::cerr << "Error parsing '" << fullPath << "': "
            << importer.GetErrorString() << "\n";
        return false;
    }

    // IMPORTANT:
    // Clear previous data so loading multiple models works correctly and bone indices start from 0 again
    vertex_to_bones.clear();
    mesh_base_vertex.clear();
    gpuVertices.clear();
    gpuIndices.clear();
    gSkeleton.bones.clear();               
    gSkeleton.boneNameToIndex.clear();      

    // Parse scene (meshes + bones + hierarchy)
    parse_scene(pScene);

    // Normalize weights AFTER all bones are known
    normalize_vertex_bone_weights();

    // Initialize runtime pose from bind pose
    initializeSkeletonPose(gSkeleton);

    // Compute initial global transforms (bind pose)
    computeGlobalBoneTransforms(gSkeleton);

    // Build physics skeleton ONCE from bind pose
    buildPhysicsSkeleton(gSkeleton, gPhysicsSkeleton);

    // Set global aiScene
    gScene = pScene;

    // Build and create GPU buffers
    build_gpu_buffers(importer.GetScene());
    create_opengl_buffers();

    // Inform input controller how many bones are available
    input.SetMaxBoneIndex(static_cast<int>(gSkeleton.bones.size()));

    // Only returns true if all previous steps succeed
    return true;
}

// ------------------------- MAIN -------------------------
int main()
{

    // ----------------------------------------------------
    // Initialize GLFW (make veiwing-window)
    // ----------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    // Request an OpenGL 3.3 core profile context before creating a window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the application window
    gWindow = glfwCreateWindow(1280, 720, "Model Viewer", nullptr, nullptr);
    if (!gWindow) {
        glfwTerminate();
        return -1;
    }

    // Make the OpenGL context current for this window
    glfwMakeContextCurrent(gWindow);

    // ----------------------------------------------------
    // Initialize GLEW (loads OpenGL function pointers)
    // ----------------------------------------------------
    
    // Start GLEW
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        std::cerr << "GLEW error: " << glewGetErrorString(glewErr) << std::endl;
        return -1;
    }

    if (boneLinesMode) {
        
        // Upload and draw debugging-bones, use DebugVertex
        glGenVertexArrays(1, &gBoneVAO);
        glGenBuffers(1, &gBoneVBO);

        glBindVertexArray(gBoneVAO);
        glBindBuffer(GL_ARRAY_BUFFER, gBoneVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex) * 256, nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    // Clear the spurious OpenGL error caused by GLEW + core profile
    glGetError();

    // Print some OpenGL info to check OpenGL works
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // ----------------------------------------------------
    // Global OpenGL settings
    // ----------------------------------------------------

    // Set the background color (dark bluish gray)
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // Enable depth testing so triangles are occluded
    glEnable(GL_DEPTH_TEST);

    // Disable face culling temporarily to avoid winding issues, turned on later again
    glDisable(GL_CULL_FACE);


    // ----------------------------------------------------
    // Create and compile shaders
    // ----------------------------------------------------
    
    if (weightVisMode) {
        weightShader = new Shader(
            "weight_visualization.vs",
            "weight_visualization.fs"
        );
    }

    if (boneLinesMode) {
        debugLineShader = new Shader(
            "debugLine_Shader.vs",
            "debugLine_Shader.fs"
        );
    }

    if (normalSkinning || boneLinesMode) {
        skinningShader = new Shader(
            "skinning.vs",
            "skinning.fs"
        );
    }

    // ----------------------------------------------------
    // Load model using Assimp and build GPU buffers
    // ----------------------------------------------------
    
    // Change this to load any model in the Models folder
    std::string modelName = "boblampclean.md5mesh";

    /*
        Examples:
        * Vanguard.dae          // RIGGED (many bones, Doom 3 test model increase max-boness to 7), super large
        * boblampclean.md5mesh  // RIGGED (best for testing, may need to be rotated to be in view)
        * spider.obj            // NOT RIGGED
        * dragon.obj            // NOT RIGGED, ONE MESH
        
        Used mostly for small tests:
        * single_bone.fbx                       // RIGGED, (test offset-matrix, should be diagonal since bone-origin is in origin)
        * two_bones_translation.fbx             // RIGGED, (test offset-matrix, second matrix should be MOSTLY diagonal since bone-origin is from in origin of first bone (offset-along y-axis))
        * two_bones_translation_rotation.fbx    // RIGGED, ANIMATED, (second bone offset-by 45 degrees)
        
        WARNING: You probably have to press Q to see most models since I tested on a large one.
    */

    // Load model, parse meshes + bones, build VBO/VAO/EBO
    if (!loadModel(modelName)) {
        std::cerr << "Failed to load model.\n";
        return 1;
    }

    // Check skeleton (parent '-1' means parent is root)
    printf("\nSkeleton summary:\n");
    printf("Total bones: %zu\n", gSkeleton.bones.size());
    for (size_t i = 0; i < gSkeleton.bones.size(); i++)
    {
        printf("Bone %zu: '%s' parent %d\n",
            i,
            gSkeleton.bones[i].name.c_str(),
            gSkeleton.bones[i].parentIndex);
    }

    // ----------------------------------------------------
    // Main render loop
    // ----------------------------------------------------
    while (!glfwWindowShouldClose(gWindow)) {

        // ------------------------------------------------
        // Time handling (used for camera movement)
        // ------------------------------------------------
        
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - gLastTime;
        gLastTime = currentTime;

        // ------------------------------------------------
        // Input handling
        // ------------------------------------------------
        
        glfwPollEvents();

        // Update input controller based on input
        input.Update(deltaTime);

        // ------------------------------------------------
        // Update skeleton pose (animation / physics step)
        // ------------------------------------------------
        
        // Currently streches model in funny way
        if (gUseRagdoll)
        {
            // Fake "physics" (real physics will go here)
            float t = (float)glfwGetTime();

            for (PhysicsBone& pb : gPhysicsSkeleton.bones)
            {
                if (pb.parentIndex != -1)
                {
                    pb.rotation =
                        glm::angleAxis(sinf(t) * 0.3f, glm::vec3(0, 0, 1));
                }
            }

            // Apply physics result to skeleton
            applyPhysicsToSkeleton(gPhysicsSkeleton, gSkeleton);
        }

        // TESTING - Makes model bend over, MUST USE SKINNING SHADER or Lines, NOT UseRagdoll
        // Comment out for a static model
        if ((skinningShader || boneLinesMode) && gSkeleton.bones.size() > 1)
        {
            gSkeleton.bones[1].localPose =
                glm::rotate(glm::mat4(1.0f),
                    sinf((float)glfwGetTime()) * 0.5f,
                    glm::vec3(0, 0, 1));
        }

        // Rebuild transforms
        computeGlobalBoneTransforms(gSkeleton);
        buildFinalBoneMatrices(gSkeleton, gFinalBoneMatrices);
        if (weightVisMode)
        {
            uploadBoneMatrices(weightShader, gFinalBoneMatrices); // Use weight-shader
        }
        else {
            uploadBoneMatrices(skinningShader, gFinalBoneMatrices); // Use skinning-shader
        }


        // ------------------------------------------------
        // Window/viewport handling
        // ------------------------------------------------

        int width, height;
        glfwGetFramebufferSize(gWindow, &width, &height);
        float aspect = (float)width / (float)height;

        glViewport(0, 0, width, height);

        // Clear color + depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ------------------------------------------------
        // Rebuild transformation matrices
        // -----------------------------------------------

        // Model matrix (identity for now — model stays at origin)
        glm::mat4 modelMatrix(1.0f);

        // Apply drag on ,odel from input controller
        modelMatrix = input.GetModelRotationMatrix() * modelMatrix;

        // The camera matrices MUST come from the camera object.
        glm::mat4 MVP =
            input.GetCamera().GetProjectionMatrix(aspect) *
            input.GetCamera().GetViewMatrix() *
            modelMatrix;

        // ------------------------------------------------
        // Rendering logic based on mode
        // ------------------------------------------------
        
        // If "fake physics" is used, may be rendered using debugLine and/or skinning shader
        if (gUseRagdoll) {
            
            // ------------------------------------------------
            // Rendering - Bones as lines
            // ------------------------------------------------

            if (boneLinesMode) {
                glDisable(GL_DEPTH_TEST);

                std::vector<DebugVertex> boneDebugVerts;
                buildBoneDebugLines(gSkeleton, boneDebugVerts);

                glBindBuffer(GL_ARRAY_BUFFER, gBoneVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, boneDebugVerts.size() * sizeof(DebugVertex), boneDebugVerts.data());

                debugLineShader->Use();
                debugLineShader->SetMat4("MVP", MVP);

                glBindVertexArray(gBoneVAO);
                glDrawArrays(GL_LINES, 0, (GLsizei)boneDebugVerts.size());
                glBindVertexArray(0);

                glEnable(GL_DEPTH_TEST);
            }

            // ------------------------------------------------
            // Rendering - Normal shader
            // ------------------------------------------------
            if (normalSkinning) {
                skinningShader->Use();
                skinningShader->SetMat4("MVP", MVP);

                glBindVertexArray(gVAO);
                glDrawElements(GL_TRIANGLES, (GLsizei)gpuIndices.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }
        else {  // No "physics"
            
            // ------------------------------------------------
            // Rendering - Bones as lines
            // ------------------------------------------------
            if (boneLinesMode) {
                glDisable(GL_DEPTH_TEST);

                std::vector<DebugVertex> boneDebugVerts;
                buildBoneDebugLines(gSkeleton, boneDebugVerts);

                glBindBuffer(GL_ARRAY_BUFFER, gBoneVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, boneDebugVerts.size() * sizeof(DebugVertex), boneDebugVerts.data());

                debugLineShader->Use();
                debugLineShader->SetMat4("MVP", MVP);

                glBindVertexArray(gBoneVAO);
                glDrawArrays(GL_LINES, 0, (GLsizei)boneDebugVerts.size());
                glBindVertexArray(0);

                glEnable(GL_DEPTH_TEST);
            }
            
            // ------------------------------------------------
            // Rendering - Weights
            // ------------------------------------------------
            if (weightVisMode) {

                // Activate the shader program
                weightShader->Use();

                // Upload MVP matrix to the shader
                weightShader->SetMat4("MVP", MVP);

                // World-space light direction
                weightShader->SetVec3("uLightDir", glm::normalize(glm::vec3(7.5f, 10.0f, 1.5f)));

                // Camera position
                weightShader->SetVec3("uViewPos", glm::vec3(0, 2, 5));

                // Upload currently selected bone index
                // Used in fragment shader for weight visualization
                weightShader->SetInt("uSelectedBone", input.GetCurrentBoneIndex());

                // Bind the VAO containing vertex + index buffers
                glBindVertexArray(gVAO);

                // Draw the entire mesh using indexed triangles
                glDrawElements(GL_TRIANGLES, (GLsizei)gpuIndices.size(), GL_UNSIGNED_INT, 0);

                // Unbind VAO (just for clean code)
                glBindVertexArray(0);

            }

            // ------------------------------------------------
            // Rendering - Normal Skinning Shader
            // ------------------------------------------------
            if (normalSkinning) {
                skinningShader->Use();
                skinningShader->SetMat4("MVP", MVP);

                glBindVertexArray(gVAO);
                glDrawElements(GL_TRIANGLES, (GLsizei)gpuIndices.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }
        
        // Present rendered image to the screen
        glfwSwapBuffers(gWindow);
    }

    // Cleanup and exit
    glfwTerminate();
    return 0;
}