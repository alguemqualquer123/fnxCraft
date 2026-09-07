#pragma once
#include <string>
#include <memory>
#include "core/Model.h"
#include "core/Diagnostics.h"
#include "skills/SkillManager.h"

struct GLFWwindow;

class App {
public:
    bool init(int w, int h, const std::string& title);
    void run(const std::string& initialFile);
    void shutdown();
    
    // File operations
    bool openFile(const std::string& path);
    void onDrop(const std::string& path);
    
private:
    GLFWwindow* window = nullptr;
    int winW = 1600, winH = 1000;
    bool shouldClose = false;
    
    std::unique_ptr<Model> currentModel;
    DiagnosticsResult lastDiagnostics;
    SkillManager skillManager;
    
    // UI state
    bool showDemo = false;
    std::string statusMessage = "Pronto. Arraste um GLB/GLTF aqui";
    float exposure = 1.0f;
    float gamma = 2.2f;
    int viewMode = 0; // 0=Rendered, 1=Solid, 2=Wireframe, 3=Normals, 4=UV
    
    void mainLoop();
    void renderUI();
    void renderViewport();
    void handleShortcuts();
    void updateStatus(const std::string& msg);
};
