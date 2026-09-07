#pragma once
#include <string>
#include <functional>
struct GLFWwindow;
struct HtmlUiEngine {
  bool init(GLFWwindow* win);
  void shutdown();
  void update(float dt);
  void render();
  void onResize(int w,int h);
  bool onKey(int key,int scancode,int action,int mods);
  bool onChar(unsigned int c);
  bool onMouse(int button,int action,int mods,double x,double y);
  void loadUrl(const std::string& url);
  void bind(const std::string& name, std::function<void(std::string)> cb);
  void callJS(const std::string& code);
  bool wantCaptureMouse() const { return captureMouse; }
  bool wantCaptureKeyboard() const { return captureKeyboard; }
  bool useHtml=true;
  bool toggle(){ useHtml=!useHtml; return useHtml; }
  bool isHtml() const { return useHtml; }
private:
  bool captureMouse=false, captureKeyboard=false;
  void *view=nullptr;
  void *ctx=nullptr;
};
extern HtmlUiEngine gHtmlUi;
