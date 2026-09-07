#pragma once
#include "Model.h"
#include <vector>
#include <string>

enum class Severity { Info, Warning, Error };

struct Diagnostic {
    Severity severity;
    std::string category; // Geometry, Materials, Textures, Animations, Skeleton, Performance, Compatibility, FileStructure
    std::string description;
    std::string location;
    std::string recommendation;
    std::string fix;
};

struct DiagnosticsResult {
    std::vector<Diagnostic> issues;
    int errors = 0, warnings = 0, infos = 0;
    bool isHealthy() const { return errors==0; }
};

DiagnosticsResult diagnose(const Model& model);
