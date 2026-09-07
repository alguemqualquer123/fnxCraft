#include "App.h"
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <filesystem>
#include <algorithm>

static void dropCallback(GLFWwindow* w, int count, const char** paths) {
    App* app = (App*)glfwGetWindowUserPointer(w);
    if(count>0) app->onDrop(paths[0]);
}

bool App::init(int w, int h, const std::string& title) {
    winW=w; winH=h;
    if(!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
    if(!window) return false;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    glfwSetWindowUserPointer(window, this);
    glfwSetDropCallback(window, dropCallback);
    glEnable(GL_DEPTH_TEST);
    registerBuiltinSkills(skillManager);
    std::cout << "[GLB Studio] Inicializado com " << skillManager.listSkills().size() << " skills\n";
    return true;
}

void App::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if(window){ glfwDestroyWindow(window); window=nullptr; }
    glfwTerminate();
}

bool App::openFile(const std::string& path) {
    updateStatus("Carregando: " + path);
    auto model = std::make_unique<Model>();
    if(!model->load(path)){
        updateStatus("Erro: " + model->lastError);
        std::cerr << "Failed to load " << path << ": " << model->lastError << "\n";
        return false;
    }
    currentModel = std::move(model);
    lastDiagnostics = diagnose(*currentModel);
    updateStatus("Carregado: " + std::filesystem::path(path).filename().string() + " | " +
        std::to_string(currentModel->meshes.size()) + " meshes, " +
        std::to_string(currentModel->materials.size()) + " materiais");
    // Run diagnostics skill
    SkillContext ctx; ctx.model = currentModel.get();
    skillManager.execute("validate.gltf", ctx);
    return true;
}
void App::onDrop(const std::string& path){ openFile(path); }
void App::updateStatus(const std::string& msg){ statusMessage = msg; std::cout << "[Status] " << msg << "\n"; }

void App::handleShortcuts(){
    if(ImGui::GetIO().WantCaptureKeyboard) return;
    // Focus, Reset, Views
    // Will be handled in viewport controls
}

void App::renderViewport(){
    ImGui::Begin("Viewport 3D", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    // Viewport controls bar
    if(ImGui::Button("Focar (F)")) {}
    ImGui::SameLine(); if(ImGui::Button("Reset (R)")) {}
    ImGui::SameLine(); if(ImGui::SmallButton("Frente[1]")) {}
    ImGui::SameLine(); if(ImGui::SmallButton("Tras[2]")) {}
    ImGui::SameLine(); if(ImGui::SmallButton("Esq[3]")) {}
    ImGui::SameLine(); if(ImGui::SmallButton("Dir[4]")) {}
    ImGui::SameLine(); if(ImGui::SmallButton("Cima[5]")) {}
    ImGui::SameLine(); if(ImGui::SmallButton("Baixo[6]")) {}
    
    // Render area
    ImVec2 avail = ImGui::GetContentRegionAvail();
    // Simple gradient background + placeholder for 3D
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    dl->AddRectFilled(p, {p.x+avail.x, p.y+avail.y}, IM_COL32(30,30,35,255));
    // Grid
    for(int i=0;i<10;i++){
        float x = p.x + avail.x*0.1f*i;
        dl->AddLine({x, p.y}, {x, p.y+avail.y}, IM_COL32(50,50,55,255));
    }
    // Placeholder model preview
    if(currentModel && currentModel->isValid){
        std::string txt = currentModel->fileName + "\n" + std::to_string(currentModel->meshes.size())+" meshes\n" + std::to_string(currentModel->materials.size())+" materiais";
        ImVec2 ts = ImGui::CalcTextSize(txt.c_str());
        ImVec2 cp = {p.x + (avail.x-ts.x)*0.5f, p.y + (avail.y-ts.y)*0.5f};
        dl->AddRectFilled({cp.x-10,cp.y-10},{cp.x+ts.x+10,cp.y+ts.y+10}, IM_COL32(60,60,70,255), 5);
        dl->AddText(cp, IM_COL32(220,220,220,255), txt.c_str());
        // Wireframe toggle visualization
        dl->AddRect({p.x+10,p.y+10},{p.x+avail.x-10,p.y+avail.y-10}, IM_COL32(80,80,90,255), 5,0,2);
    } else {
        const char* drop = "Arraste seu modelo 3D aqui\n\nGLB / GLTF / BIN + texturas";
        ImVec2 ts = ImGui::CalcTextSize(drop);
        ImVec2 cp = {p.x + (avail.x-ts.x)*0.5f, p.y + (avail.y-ts.y)*0.5f};
        dl->AddText(cp, IM_COL32(120,120,130,255), drop);
        // Dashed border
        dl->AddRect(p, {p.x+avail.x, p.y+avail.y}, IM_COL32(70,70,80,255), 8,0,2);
    }
    ImGui::Dummy(avail);
    
    // View mode
    ImGui::Separator();
    const char* modes[] = {"Rendered","Solid","Wireframe","Normals","UV","Albedo","Metalness","Roughness"};
    ImGui::Combo("Modo", &viewMode, modes, IM_ARRAYSIZE(modes));
    ImGui::SliderFloat("Exposição", &exposure, 0.1f, 3.0f);
    ImGui::SliderFloat("Gamma", &gamma, 0.1f, 3.0f);
    
    // Options
    static bool wireframe=false, grid=true, bbox=false, skeleton=false;
    ImGui::Checkbox("Grid", &grid); ImGui::SameLine();
    ImGui::Checkbox("Caixa", &bbox); ImGui::SameLine();
    ImGui::Checkbox("Wireframe", &wireframe); ImGui::SameLine();
    ImGui::Checkbox("Esqueleto", &skeleton);
    
    ImGui::End();
}

void App::renderUI(){
    // --- LAYOUT FIXO: DockBuilder na primeira frame ---
    static bool dockBuilt = false;
    ImGuiID dockspaceId = ImGui::GetID("GLBStudio_DockSpace");
    if(!dockBuilt){
        dockBuilt = true;
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);
        ImGuiID dockLeft, dockCenter, dockRight, dockBottom;
        ImGuiID dockTop = dockspaceId;
        // Divide: esquerda 20%, centro 60%, direita 20%
        dockLeft = ImGui::DockBuilderSplitNode(dockTop, ImGuiDir_Left, 0.20f, nullptr, &dockTop);
        dockRight = ImGui::DockBuilderSplitNode(dockTop, ImGuiDir_Right, 0.20f, nullptr, &dockTop);
        // Centro fica como top restante
        dockCenter = dockTop;
        // Bottom 22% para timeline/performance
        dockBottom = ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Down, 0.22f, nullptr, &dockCenter);
        // Dock janelas fixas
        ImGui::DockBuilderDockWindow("Grafo da Cena", dockLeft);
        ImGui::DockBuilderDockWindow("Inspetor", dockRight);
        ImGui::DockBuilderDockWindow("Diagnóstico", dockRight);
        ImGui::DockBuilderDockWindow("Performance", dockBottom);
        ImGui::DockBuilderDockWindow("Linha do Tempo", dockBottom);
        ImGui::DockBuilderDockWindow("Viewport 3D", dockCenter);
        ImGui::DockBuilderDockWindow("Status", dockBottom);
        ImGui::DockBuilderFinish(dockspaceId);
    }
    // Menu bar
    if(ImGui::BeginMainMenuBar()){
        if(ImGui::BeginMenu("Arquivo")){
            if(ImGui::MenuItem("Abrir GLB", "Ctrl+O")) {
                // File picker would go here - for now use drop
                updateStatus("Use drag & drop para abrir GLB/GLTF");
            }
            if(ImGui::MenuItem("Abrir glTF")) { updateStatus("Use drag & drop"); }
            ImGui::Separator();
            if(ImGui::MenuItem("Exportar GLB")) {
                SkillContext ctx; ctx.model=currentModel.get();
                auto r=skillManager.execute("model.optimize", ctx);
                updateStatus(r.message);
            }
            if(ImGui::MenuItem("Screenshot")) {
                SkillContext ctx; skillManager.execute("screenshot", ctx);
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Visualizar")){
            ImGui::MenuItem("Material Preview", nullptr, viewMode==0);
            ImGui::MenuItem("Normals", nullptr, viewMode==3);
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Skills")){
            for(auto &s: skillManager.listSkills()){
                if(ImGui::MenuItem(s.name.c_str())) {
                    SkillContext ctx; ctx.model=currentModel.get();
                    auto r=skillManager.execute(s.id, ctx);
                    updateStatus(r.message);
                }
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Ajuda")){
            ImGui::MenuItem("Sobre GLB Studio");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
    // Docking host fixo
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin("GLBStudio_DockHost", nullptr, host_flags);
    ImGui::PopStyleVar(3);
    ImGui::DockSpace(dockspaceId, ImVec2(0,0), ImGuiDockNodeFlags_None, nullptr);
    ImGui::End();
    
    // Scene Graph - FIXO ESQUERDA 20%
    ImGui::Begin("Grafo da Cena", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    if(currentModel){
        if(ImGui::TreeNodeEx("Cena", ImGuiTreeNodeFlags_DefaultOpen)){
            for(size_t i=0;i<currentModel->nodes.size() && i<20;i++){
                auto &n = currentModel->nodes[i];
                bool hasChildren = !n.children.empty();
                auto flags = hasChildren ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Leaf;
                if(ImGui::TreeNodeEx(n.name.c_str(), flags)){
                    ImGui::Text("Pos: %.1f,%.1f,%.1f", n.position.x, n.position.y, n.position.z);
                    if(n.meshIndex>=0) ImGui::Text("Mesh: %d", n.meshIndex);
                    ImGui::TreePop();
                }
            }
            if(currentModel->nodes.size()>20) ImGui::Text("... e mais %zu nodes", currentModel->nodes.size()-20);
            ImGui::TreePop();
        }
        if(!currentModel->skeletons.empty()){
            if(ImGui::TreeNode("Esqueleto")){
                for(auto &sk: currentModel->skeletons){
                    if(ImGui::TreeNode(sk.name.c_str())){
                        ImGui::Text("%d bones", sk.boneCount);
                        for(size_t i=0;i<sk.boneNames.size() && i<10;i++) ImGui::BulletText("%s", sk.boneNames[i].c_str());
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
    } else {
        ImGui::TextDisabled("Nenhum modelo carregado");
        ImGui::Text("Arraste um .glb/.gltf aqui");
    }
    ImGui::End();
    
    // Inspector
    ImGui::Begin("Inspetor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    if(currentModel){
        if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)){
            ImGui::Text("Posição: 0,0,0");
            ImGui::Text("Rotação: 0,0,0");
            ImGui::Text("Escala: 1,1,1");
        }
        if(ImGui::CollapsingHeader("Estatísticas", ImGuiTreeNodeFlags_DefaultOpen)){
            ImGui::Text("Vértices: %d", currentModel->totalVertices);
            ImGui::Text("Triângulos: %d", currentModel->totalTriangles);
            ImGui::Text("Draw Calls: %d", currentModel->totalDrawCalls);
            ImGui::Text("Memória Textura: %.1f MB", currentModel->totalTextureMemory/1024.0/1024.0);
            ImGui::Text("Arquivo: %.1f MB", currentModel->fileSize/1024.0/1024.0);
        }
        if(ImGui::CollapsingHeader("Meshes")){
            for(auto &m: currentModel->meshes){
                if(ImGui::TreeNode(m.name.c_str())){
                    ImGui::Text("Vértices: %d Triângulos: %d", m.vertices, m.triangles);
                    ImGui::Text("BB: (%.1f,%.1f,%.1f) -> (%.1f,%.1f,%.1f)", m.bboxMin.x,m.bboxMin.y,m.bboxMin.z, m.bboxMax.x,m.bboxMax.y,m.bboxMax.z);
                    ImGui::Text("Normais: %s Tangentes: %s UV: %s", m.hasNormals?"sim":"não", m.hasTangents?"sim":"não", m.hasUV?"sim":"não");
                    ImGui::TreePop();
                }
            }
        }
        if(ImGui::CollapsingHeader("Materiais")){
            for(auto &mat: currentModel->materials){
                if(ImGui::TreeNode(mat.name.c_str())){
                    ImGui::ColorEdit4("Base Color", &mat.baseColor.x);
                    ImGui::SliderFloat("Metallic", &mat.metallic, 0,1);
                    ImGui::SliderFloat("Roughness", &mat.roughness, 0,1);
                    ImGui::Text("Textura: %s", mat.baseColorTexture.empty()?"nenhuma":mat.baseColorTexture.c_str());
                    ImGui::TreePop();
                }
            }
        }
        if(ImGui::CollapsingHeader("Texturas")){
            for(auto &t: currentModel->textures){
                ImGui::BulletText("%s %dx%d %s %.1fMB", t.path.c_str(), t.width, t.height, t.format.c_str(), t.gpuMemory/1024.0/1024.0);
            }
        }
        if(!currentModel->animations.empty() && ImGui::CollapsingHeader("Animações")){
            for(auto &a: currentModel->animations){
                ImGui::BulletText("%s (%.1fs, %d canais)", a.name.c_str(), a.duration, a.channels);
                ImGui::Text("  Trans:%s Rot:%s Scale:%s", a.hasTranslation?"s":"n", a.hasRotation?"s":"n", a.hasScale?"s":"n");
            }
            static bool playing=false; static float speed=1.0f, t=0;
            ImGui::Checkbox("Play", &playing); ImGui::SameLine();
            ImGui::SliderFloat("Velocidade", &speed, 0.1f, 3.0f);
            ImGui::SliderFloat("Linha do tempo", &t, 0, 5);
        }
    } else {
        ImGui::TextDisabled("Selecione um objeto");
    }
    ImGui::End();
    
    // Diagnostics
    ImGui::Begin("Diagnóstico", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    if(currentModel){
        ImGui::Text("Saúde do Modelo: %s", lastDiagnostics.isHealthy()?"Saudável":"Problemas");
        ImGui::Text("Erros: %d Avisos: %d Info: %d", lastDiagnostics.errors, lastDiagnostics.warnings, lastDiagnostics.infos);
        for(auto &d: lastDiagnostics.issues){
            ImVec4 col = d.severity==Severity::Error?ImVec4(1,0.3f,0.3f,1): d.severity==Severity::Warning?ImVec4(1,0.8f,0.2f,1):ImVec4(0.6f,0.8f,1,1);
            const char* icon = d.severity==Severity::Error?"✕": d.severity==Severity::Warning?"⚠":"✓";
            ImGui::TextColored(col, "%s [%s] %s", icon, d.category.c_str(), d.description.c_str());
            if(!d.recommendation.empty()){
                ImGui::SameLine(); if(ImGui::SmallButton(("Fix##"+d.description).c_str())) updateStatus("Aplicando: "+d.fix);
            }
            ImGui::TextDisabled("  Local: %s | %s", d.location.c_str(), d.recommendation.c_str());
        }
        if(ImGui::Button("Re-analisar")){
            lastDiagnostics = diagnose(*currentModel);
        }
        ImGui::SameLine();
        if(ImGui::Button("Otimizar")){
            SkillContext ctx; ctx.model=currentModel.get();
            auto r=skillManager.execute("model.optimize", ctx);
            updateStatus(r.message);
        }
    } else {
        ImGui::TextDisabled("Carregue um modelo para diagnosticar");
    }
    ImGui::End();
    
    // Performance
    ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame: %.1f ms", 1000.0f/ImGui::GetIO().Framerate);
    if(currentModel){
        ImGui::Text("Draw Calls: %d", currentModel->totalDrawCalls);
        ImGui::Text("Triângulos: %d", currentModel->totalTriangles);
        ImGui::Text("Vértices: %d", currentModel->totalVertices);
        ImGui::Text("Texturas: %zu (%.1f MB)", currentModel->textures.size(), currentModel->totalTextureMemory/1024.0/1024.0);
        ImGui::Text("Materiais: %zu", currentModel->materials.size());
        ImGui::Text("Esqueletos: %zu", currentModel->skeletons.size());
        ImGui::Text("Animações: %zu", currentModel->animations.size());
    }
    ImGui::End();
    
    // Timeline
    ImGui::Begin("Linha do Tempo", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::TextDisabled("Arraste para scrub, play/pause acima");
    ImGui::Button("Play", {60,0}); ImGui::SameLine();
    ImGui::Button("Pause", {60,0}); ImGui::SameLine();
    ImGui::Button("Stop", {60,0});
    ImGui::End();
    
    renderViewport();
    
    // Status bar
    ImGui::Begin("Status", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    ImGui::Text("%s", statusMessage.c_str());
    ImGui::End();
}

void App::mainLoop(){
    while(!glfwWindowShouldClose(window) && !shouldClose){
        glfwPollEvents();
        handleShortcuts();
        // Ctrl+K palette
        if(ImGui::GetIO().KeyCtrl && glfwGetKey(window, GLFW_KEY_K)==GLFW_PRESS){
            ImGui::OpenPopup("Command Palette");
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        renderUI();
        
        ImGui::Render();
        int w,h; glfwGetFramebufferSize(window, &w, &h);
        glViewport(0,0,w,h);
        glClearColor(0.12f,0.12f,0.14f,1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
            GLFWwindow* backup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup);
        }
        glfwSwapBuffers(window);
    }
}

void App::run(const std::string& initialFile){
    if(!initialFile.empty()) openFile(initialFile);
    mainLoop();
}
