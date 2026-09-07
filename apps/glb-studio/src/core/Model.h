#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

struct MeshInfo {
    std::string name;
    int vertices = 0;
    int triangles = 0;
    int indices = 0;
    int materialIndex = -1;
    glm::vec3 bboxMin{0}, bboxMax{0};
    bool hasNormals = false;
    bool hasTangents = false;
    bool hasUV = false;
    bool hasSkin = false;
};

struct MaterialInfo {
    std::string name;
    glm::vec4 baseColor{1};
    float metallic = 0;
    float roughness = 1;
    float opacity = 1;
    bool hasNormalMap = false;
    bool hasAOMap = false;
    bool hasEmissive = false;
    std::string baseColorTexture;
    std::string normalTexture;
    std::string metallicRoughnessTexture;
};

struct TextureInfo {
    std::string path;
    int width = 0, height = 0;
    int channels = 0;
    std::string format = "Unknown";
    size_t fileSize = 0;
    size_t gpuMemory = 0;
    bool hasMipmaps = false;
};

struct NodeInfo {
    std::string name;
    glm::vec3 position{0}, rotation{0}, scale{1};
    int meshIndex = -1;
    int parent = -1;
    std::vector<int> children;
};

struct AnimationInfo {
    std::string name;
    float duration = 0;
    int channels = 0;
    bool hasTranslation = false;
    bool hasRotation = false;
    bool hasScale = false;
    bool hasMorph = false;
};

struct SkeletonInfo {
    std::string name;
    int boneCount = 0;
    std::vector<std::string> boneNames;
    std::vector<glm::mat4> inverseBindMatrices;
};

class Model {
public:
    bool load(const std::string& path);
    void clear();
    
    std::string filePath;
    std::string fileName;
    size_t fileSize = 0;
    
    std::vector<MeshInfo> meshes;
    std::vector<MaterialInfo> materials;
    std::vector<TextureInfo> textures;
    std::vector<NodeInfo> nodes;
    std::vector<AnimationInfo> animations;
    std::vector<SkeletonInfo> skeletons;
    
    // Stats
    int totalVertices = 0;
    int totalTriangles = 0;
    int totalDrawCalls = 0;
    size_t totalTextureMemory = 0;
    
    // Raw Assimp scene for rendering (simplified)
    void* scene = nullptr;
    bool isValid = false;
    std::string lastError;
    
    // Helpers
    static bool isGLB(const std::string& path);
    static bool isGLTF(const std::string& path);
};
