#pragma once
#include <glad/glad.h>
#include <string>
void GLAPIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
void enableReportGlErrors();
void createErrorFile();
void initCrashHandler();
void reportError(const char *message);
void reportErrorDetailed(const char *file, int line, const char *func, const char *message);
#define REPORT_ERROR(msg) reportErrorDetailed(__FILE__, __LINE__, __func__, msg)
std::string getHumanReadableSignal(int sig);
std::string demangleSymbol(const char *mangled);
