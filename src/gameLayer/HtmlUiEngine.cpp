#include "HtmlUiEngine.h"
#include <iostream>
#include <filesystem>
HtmlUiEngine gHtmlUi;
bool HtmlUiEngine::init(GLFWwindow* win){
  std::cout<<"[HtmlUi] init file://resources/ui/index.html (RmlUi/Ultralight stub) - responsivo, temas hot-reload\n";
  std::filesystem::create_directories("resources/ui/themes");
  return true;
}
void HtmlUiEngine::shutdown(){}
void HtmlUiEngine::update(float dt){}
void HtmlUiEngine::render(){
  if(!useHtml) return;
}
void HtmlUiEngine::onResize(int w,int h){}
bool HtmlUiEngine::onKey(int k,int s,int a,int m){ return false; }
bool HtmlUiEngine::onChar(unsigned c){ return false; }
bool HtmlUiEngine::onMouse(int b,int a,int m,double x,double y){ return false; }
void HtmlUiEngine::loadUrl(const std::string& u){ std::cout<<"[HtmlUi] load "<<u<<"\n"; }
void HtmlUiEngine::bind(const std::string& n,std::function<void(std::string)> cb){}
void HtmlUiEngine::callJS(const std::string& c){}
