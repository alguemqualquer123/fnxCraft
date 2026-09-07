#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <algorithm>
#include <unordered_set>
#include <functional>

bool Model::isGLB(const std::string& p) {
    auto l = p; std::transform(l.begin(), l.end(), l.begin(), ::tolower);
    return l.size()>=4 && l.compare(l.size()-4,4,".glb")==0;
}
bool Model::isGLTF(const std::string& p) {
    auto l = p; std::transform(l.begin(), l.end(), l.begin(), ::tolower);
    return l.size()>=5 && l.compare(l.size()-5,5,".gltf")==0;
}

void Model::clear() {
    meshes.clear(); materials.clear(); textures.clear();
    nodes.clear(); animations.clear(); skeletons.clear();
    totalVertices = totalTriangles = totalDrawCalls = 0;
    totalTextureMemory = 0;
    filePath.clear(); fileName.clear(); fileSize = 0;
    isValid = false; lastError.clear();
    scene = nullptr;
}

bool Model::load(const std::string& path) {
    clear();
    filePath = path;
    fileName = std::filesystem::path(path).filename().string();
    try { fileSize = std::filesystem::file_size(path); } catch(...) { fileSize = 0; }
    
    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, true);
    const aiScene* sc = importer.ReadFile(path, 
        aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    
    if (!sc || !sc->mRootNode) {
        lastError = importer.GetErrorString();
        if(lastError.empty()) lastError = "Falha ao ler arquivo (GLB/GLTF inválido ou corrompido)";
        return false;
    }
    
    // Meshes
    for (unsigned i=0;i<sc->mNumMeshes;i++) {
        auto* m = sc->mMeshes[i];
        MeshInfo mi;
        mi.name = m->mName.C_Str();
        if(mi.name.empty()) mi.name = "Mesh_" + std::to_string(i);
        mi.vertices = m->mNumVertices;
        mi.triangles = m->mNumFaces;
        mi.indices = m->mNumFaces * 3;
        mi.materialIndex = m->mMaterialIndex;
        mi.hasNormals = m->HasNormals();
        mi.hasTangents = m->mTangents != nullptr;
        mi.hasUV = m->HasTextureCoords(0);
        mi.hasSkin = m->HasBones();
        if(m->mNumVertices>0){
            aiVector3D mn = m->mVertices[0], mx = m->mVertices[0];
            for(unsigned v=1;v<m->mNumVertices;v++){
                mn.x = std::min(mn.x, m->mVertices[v].x);
                mn.y = std::min(mn.y, m->mVertices[v].y);
                mn.z = std::min(mn.z, m->mVertices[v].z);
                mx.x = std::max(mx.x, m->mVertices[v].x);
                mx.y = std::max(mx.y, m->mVertices[v].y);
                mx.z = std::max(mx.z, m->mVertices[v].z);
            }
            mi.bboxMin = {mn.x,mn.y,mn.z};
            mi.bboxMax = {mx.x,mx.y,mx.z};
        }
        totalVertices += mi.vertices;
        totalTriangles += mi.triangles;
        totalDrawCalls++;
        meshes.push_back(mi);
    }
    
    // Materials
    for(unsigned i=0;i<sc->mNumMaterials;i++){
        auto* mat = sc->mMaterials[i];
        MaterialInfo mi;
        mi.name = mat->GetName().C_Str();
        if(mi.name.empty()) mi.name = "Material_" + std::to_string(i);
        aiColor4D col; float f;
        if(mat->Get(AI_MATKEY_BASE_COLOR, col)==AI_SUCCESS) mi.baseColor = {col.r,col.g,col.b,col.a};
        else if(mat->Get(AI_MATKEY_COLOR_DIFFUSE, col)==AI_SUCCESS) mi.baseColor = {col.r,col.g,col.b,col.a};
        mat->Get(AI_MATKEY_METALLIC_FACTOR, mi.metallic);
        mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, mi.roughness);
        mat->Get(AI_MATKEY_OPACITY, mi.opacity);
        aiString tex;
        if(mat->GetTexture(aiTextureType_DIFFUSE,0,&tex)==AI_SUCCESS) mi.baseColorTexture = tex.C_Str();
        if(mat->GetTexture(aiTextureType_NORMALS,0,&tex)==AI_SUCCESS) mi.hasNormalMap = true;
        if(mat->GetTexture(aiTextureType_LIGHTMAP,0,&tex)==AI_SUCCESS) mi.hasAOMap = true;
        if(mat->GetTexture(aiTextureType_EMISSIVE,0,&tex)==AI_SUCCESS) mi.hasEmissive = true;
        materials.push_back(mi);
    }
    
    // Textures
    for(unsigned i=0;i<sc->mNumTextures;i++){
        auto* tex = sc->mTextures[i];
        TextureInfo ti;
        ti.path = tex->mFilename.C_Str();
        if(ti.path.empty()) ti.path = "Embedded_" + std::to_string(i);
        ti.width = tex->mWidth;
        ti.height = tex->mHeight;
        ti.channels = 4;
        ti.format = tex->achFormatHint;
        if(ti.format.empty()) ti.format = "RGBA8";
        ti.gpuMemory = (size_t)ti.width * ti.height * 4;
        totalTextureMemory += ti.gpuMemory;
        textures.push_back(ti);
    }
    // Also collect from materials
    std::unordered_set<std::string> seen;
    for(auto &m: materials){
        for(auto *p : {&m.baseColorTexture, &m.normalTexture, &m.metallicRoughnessTexture}){
            if(!p->empty() && seen.insert(*p).second){
                TextureInfo ti; ti.path = *p; ti.width=1024; ti.height=1024; ti.channels=4; ti.format="RGBA8";
                ti.fileSize = 1024*1024*4; ti.gpuMemory = ti.fileSize;
                textures.push_back(ti);
                totalTextureMemory += ti.gpuMemory;
            }
        }
    }
    
    // Nodes (flatten)
    std::function<void(aiNode*,int)> trav = [&](aiNode* n,int parent){
        NodeInfo ni; ni.name = n->mName.C_Str(); if(ni.name.empty()) ni.name="Node";
        aiVector3D p,r; aiVector3D s; n->mTransformation.Decompose(s,r,p);
        ni.position = {p.x,p.y,p.z}; ni.rotation = {r.x,r.y,r.z}; ni.scale = {s.x,s.y,s.z};
        ni.parent = parent; ni.meshIndex = (n->mNumMeshes>0)? n->mMeshes[0] : -1;
        int idx = nodes.size();
        nodes.push_back(ni);
        for(unsigned i=0;i<n->mNumChildren;i++){
            int child = nodes.size();
            nodes[idx].children.push_back(child);
            trav(n->mChildren[i], idx);
        }
    };
    trav(sc->mRootNode, -1);
    
    // Animations
    for(unsigned i=0;i<sc->mNumAnimations;i++){
        auto* a = sc->mAnimations[i];
        AnimationInfo ai; ai.name = a->mName.C_Str(); if(ai.name.empty()) ai.name="Anim_"+std::to_string(i);
        ai.duration = (float)(a->mDuration / (a->mTicksPerSecond>0?a->mTicksPerSecond:25.0));
        ai.channels = a->mNumChannels;
        for(unsigned c=0;c<a->mNumChannels;c++){
            auto* ch = a->mChannels[c];
            for(unsigned k=0;k<ch->mNumPositionKeys;k++) ai.hasTranslation = true;
            for(unsigned k=0;k<ch->mNumRotationKeys;k++) ai.hasRotation = true;
            for(unsigned k=0;k<ch->mNumScalingKeys;k++) ai.hasScale = true;
        }
        ai.hasMorph = false;
        animations.push_back(ai);
    }
    
    // Skeletons (bones)
    for(unsigned i=0;i<sc->mNumMeshes;i++){
        auto* m = sc->mMeshes[i];
        if(m->HasBones()){
            SkeletonInfo si; si.name = "Skeleton_"+std::to_string(i); si.boneCount = m->mNumBones;
            for(unsigned b=0;b<m->mNumBones;b++){
                si.boneNames.push_back(m->mBones[b]->mName.C_Str());
                aiMatrix4x4 inv = m->mBones[b]->mOffsetMatrix;
                glm::mat4 mat;
                mat[0][0]=inv.a1; mat[1][0]=inv.a2; mat[2][0]=inv.a3; mat[3][0]=inv.a4;
                mat[0][1]=inv.b1; mat[1][1]=inv.b2; mat[2][1]=inv.b3; mat[3][1]=inv.b4;
                mat[0][2]=inv.c1; mat[1][2]=inv.c2; mat[2][2]=inv.c3; mat[3][2]=inv.c4;
                mat[0][3]=inv.d1; mat[1][3]=inv.d2; mat[2][3]=inv.d3; mat[3][3]=inv.d4;
                si.inverseBindMatrices.push_back(mat);
            }
            skeletons.push_back(si);
        }
    }
    
    isValid = true;
    // Keep scene alive by not destroying importer (leak for now, will be managed by Model lifetime)
    // For simplicity, we copy what we need and let importer go out of scope (scene will be freed)
    // Instead, we keep a copy via importer.GetOrphanedScene() - but we drop it for now and just use stats
    // To avoid dangling, we don't keep scene pointer
    scene = nullptr;
    return true;
}
