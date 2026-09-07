#include "app/App.h"
#include <iostream>

int main(int argc, char** argv) {
    try {
        App app;
        if (!app.init(1600, 1000, "GLB Studio - Advanced GLTF/GLB Inspector")) {
            std::cerr << "Failed to init GLB Studio\n";
            return 1;
        }
        std::string initialFile;
        if (argc > 1) initialFile = argv[1];
        app.run(initialFile);
        app.shutdown();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
