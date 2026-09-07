#include "Diagnostics.h"

DiagnosticsResult diagnose(const Model& m) {
    DiagnosticsResult r;
    auto add = [&](Severity s, const std::string& cat, const std::string& desc, const std::string& loc, const std::string& rec, const std::string& fix=""){
        r.issues.push_back({s,cat,desc,loc,rec,fix});
        if(s==Severity::Error) r.errors++; else if(s==Severity::Warning) r.warnings++; else r.infos++;
    };
    if(!m.isValid) add(Severity::Error, "FileStructure", "Modelo inválido ou corrompido", "File", "Verifique se o GLB não está corrompido", "Tente re-exportar");
    if(m.totalVertices>100000) add(Severity::Warning, "Performance", "Muitos vértices ("+std::to_string(m.totalVertices)+")", "Geometry", "Considere decimation/LOD", "Optimize Geometry");
    if(m.textures.size()>10) add(Severity::Warning, "Performance", std::to_string(m.textures.size())+" texturas", "Textures", "Combine texturas", "Merge Materials");
    for(auto &t: m.textures) if(t.width>=4096 || t.height>=4096) add(Severity::Warning, "Textures", "Textura 4K detectada: "+t.path, "Textures", "Reduza para 2K", "Compress Textures");
    for(auto &mesh: m.meshes){
        if(!mesh.hasNormals) add(Severity::Warning, "Geometry", "Mesh sem normais: "+mesh.name, "Mesh", "Gere normais", "Gen Tangents");
        if(!mesh.hasTangents) add(Severity::Warning, "Geometry", "Mesh sem tangentes: "+mesh.name, "Mesh", "Gere tangentes", "");
        if(!mesh.hasUV) add(Severity::Warning, "Geometry", "Mesh sem UV: "+mesh.name, "Mesh", "Adicione UVs", "");
    }
    if(m.totalDrawCalls>100) add(Severity::Warning, "Performance", std::to_string(m.totalDrawCalls)+" draw calls", "Performance", "Merge meshes/materials", "Reduce Draw Calls");
    if(m.materials.size()>50) add(Severity::Warning, "Materials", std::to_string(m.materials.size())+" materiais", "Materials", "Combine materiais", "");
    if(m.animations.empty()) add(Severity::Info, "Animations", "Sem animações", "Animations", "Nenhuma ação necessária","");
    for(auto &s: m.skeletons) if(s.boneCount>100) add(Severity::Warning, "Skeleton", "Muitos bones: "+std::to_string(s.boneCount), "Skeleton", "Simplifique esqueleto","");
    if(r.issues.empty()) add(Severity::Info, "FileStructure", "Modelo saudável", "File", "Nenhum problema","");
    return r;
}
