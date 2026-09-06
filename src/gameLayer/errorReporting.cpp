#include <errorReporting.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <csignal>
#include <cstring>
#include <string>
#ifndef _WIN32
#include <execinfo.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cxxabi.h>
#endif
void GLAPIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131222) return;
	if (type == GL_DEBUG_TYPE_PERFORMANCE) return;
	std::cout << "---------------\nDebug message (" << id << "): " << message << "\n";
}
void enableReportGlErrors()
{
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}
static std::string getTimestamp(){
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	char buf[64]; strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S", std::localtime(&t));
	return buf;
}
static void writeLogFile(const std::string &path, const std::string &msg){
	std::filesystem::create_directories("logs");
	std::filesystem::create_directories("build/logs");
	std::ofstream f(path, std::ios::app);
	if(f){ f << "[" << getTimestamp() << "] " << msg << "\n"; f.flush(); }
	std::ofstream f2("logs/game.log", std::ios::app);
	if(f2 && path!="logs/game.log"){ f2 << "[" << getTimestamp() << "] " << msg << "\n"; f2.flush(); }
	std::ofstream f3("build/logs/game.log", std::ios::app);
	if(f3 && path!="build/logs/game.log"){ f3 << "[" << getTimestamp() << "] " << msg << "\n"; f3.flush(); }
}
void createErrorFile()
{
	std::filesystem::create_directories("logs");
	std::filesystem::create_directories("build/logs");
	std::ofstream f("logs/game.log", std::ios::app);
	if(f) f << "[" << getTimestamp() << "] === Game started ===\n";
	std::ofstream f2("build/logs/game.log", std::ios::app);
	if(f2) f2 << "[" << getTimestamp() << "] === Game started ===\n";
	std::cout << "[log] game.log inicializado em logs/game.log\n";
}
std::string getHumanReadableSignal(int sig){
	switch(sig){
		case SIGSEGV: return "SIGSEGV (11) - Falha de segmentacao: acesso invalido a memoria (ponteiro nulo ou corrompido)";
		case SIGABRT: return "SIGABRT (6) - Abortado: assert falhou, excecao nao tratada ou std::abort()";
		case SIGFPE: return "SIGFPE (8) - Erro aritmetico: divisao por zero ou overflow";
		case SIGILL: return "SIGILL (4) - Instrucao ilegal: codigo corrompido ou CPU incompativel";
		case SIGTERM: return "SIGTERM (15) - Terminacao solicitada externamente";
#ifdef SIGBUS
		case SIGBUS: return "SIGBUS (7) - Erro de barramento: acesso desalinhado ou mmap falhou";
#endif
		default: return "Signal " + std::to_string(sig) + " - Erro desconhecido";
	}
}
std::string demangleSymbol(const char *mangled){
#ifndef _WIN32
	int status=0;
	char *dem = abi::__cxa_demangle(mangled,nullptr,nullptr,&status);
	std::string r = (status==0 && dem) ? dem : mangled;
	if(dem) free(dem);
	return r;
#else
	return mangled;
#endif
}
#ifndef _WIN32
static std::string addr2lineStr(void *addr){
	char exe[512]={0};
	ssize_t l = readlink("/proc/self/exe", exe, sizeof(exe)-1);
	if(l<=0) return "";
	std::string cmd = std::string("addr2line -e \"") + exe + "\" -f -C -p " + std::to_string((uintptr_t)addr) + " 2>&1";
	FILE *p = popen(cmd.c_str(),"r");
	if(!p) return "";
	char buf[1024]={0};
	std::string out;
	while(fgets(buf,sizeof(buf),p)) out+=buf;
	pclose(p);
	while(!out.empty() && (out.back()=='\n' || out.back()=='\r')) out.pop_back();
	if(out.find("??")!=std::string::npos && out.find(":0")!=std::string::npos) return "";
	return out;
}
static std::string dladdrStr(void *addr){
	Dl_info info{};
	if(dladdr(addr,&info) && info.dli_sname){
		std::string dem = demangleSymbol(info.dli_sname);
		char off[64]={0};
		snprintf(off,sizeof(off)," +0x%lx",(uintptr_t)addr - (uintptr_t)info.dli_saddr);
		std::string s = dem + off;
		if(info.dli_fname) s += std::string(" [") + info.dli_fname + "]";
		return s;
	}
	return "";
}
#endif
static void crashHandler(int sig){
	std::string sigHuman = getHumanReadableSignal(sig);
	std::string header = "========== CRASH DETECTADO ==========\n" + sigHuman + "\nVeja: logs/crash.log | logs/game.log | build/logs/crash.log\n";
	writeLogFile("logs/crash.log", header);
	writeLogFile("logs/game.log", header);
	std::cerr << "\n" << header << "\n";
	std::string btHeader = "Stacktrace (endereco -> funcao demangleada -> arquivo:linha):";
	writeLogFile("logs/crash.log", btHeader);
	std::cerr << btHeader << "\n";
#ifndef _WIN32
	void *arr[64]; int n = backtrace(arr,64);
	char **syms = backtrace_symbols(arr,n);
	std::string fullBt;
	for(int i=0;i<n;i++){
		std::string raw = syms ? syms[i] : "";
		std::string mangled;
		auto p1 = raw.find('(');
		auto p2 = raw.find('+', p1==std::string::npos?0:p1);
		if(p1!=std::string::npos && p2!=std::string::npos) mangled = raw.substr(p1+1, p2-p1-1);
		std::string dem = mangled.empty() ? raw : demangleSymbol(mangled.c_str());
		std::string loc = addr2lineStr(arr[i]);
		if(loc.empty()) loc = dladdrStr(arr[i]);
		if(loc.empty()) loc = raw;
		if(loc.find("??")!=std::string::npos && !dem.empty() && dem!=raw) loc = dem + " -> " + loc;
		else if(!dem.empty() && dem!=raw && loc.find(dem)==std::string::npos) loc = dem + " @ " + loc;
		std::string line = "[" + std::to_string(i) + "] " + loc + "  (" + raw + ")";
		fullBt += line + "\n";
		std::cerr << line << "\n";
		if(i==2){
			std::string hint = "  ^-- possivel origem do erro acima";
			if(loc.find("SplashScreen")!=std::string::npos) hint = "  ^-- ERRO em SplashScreen::draw";
			else if(loc.find("glfw")!=std::string::npos) hint = "  ^-- ERRO dentro do GLFW - janela invalida";
			else if(loc.find("initGame")!=std::string::npos) hint = "  ^-- ERRO durante initGame()";
			fullBt += hint + "\n"; std::cerr << hint << "\n";
		}
	}
	if(syms) free(syms);
	writeLogFile("logs/crash.log", fullBt);
	writeLogFile("logs/game.log", fullBt);
	std::string footer = "\nDICA: Se \"addr2line\" mostrar \"??:0\", recompile com -DCMAKE_BUILD_TYPE=RelWithDebInfo\nO jogo NAO fechara sozinho - pressione ENTER ou aguarde 30s.\n";
	writeLogFile("logs/crash.log", footer);
	std::cerr << footer << "\nPressione ENTER para fechar...\n"; std::cerr.flush();
	if(::isatty(STDIN_FILENO)){
		fd_set fds; FD_ZERO(&fds); FD_SET(STDIN_FILENO,&fds);
		struct timeval tv{30,0};
		select(STDIN_FILENO+1,&fds,nullptr,nullptr,&tv);
		if(FD_ISSET(STDIN_FILENO,&fds)){ char c; ::read(STDIN_FILENO,&c,1); }
	}else{
		sleep(5);
		if(::system("which zenity >/dev/null 2>&1")==0){
			std::string z = "zenity --error --title='ourCraft crashou!' --text='"
				+ sigHuman + "\\n\\nVeja logs/crash.log' --width=520 2>/dev/null &";
			::system(z.c_str()); sleep(6);
		}else if(::system("which kdialog >/dev/null 2>&1")==0){
			::system("kdialog --error 'ourCraft crashou! Veja logs/crash.log' 2>/dev/null &"); sleep(6);
		}else sleep(5);
	}
	_Exit(1);
#else
	std::string msg = sigHuman + "\nStacktrace indisponivel no Windows\n";
	writeLogFile("logs/crash.log", msg);
	std::cerr << msg << "\nPressione ENTER...\n"; std::cin.get(); std::exit(1);
#endif
}
void initCrashHandler(){
	std::signal(SIGSEGV, crashHandler);
	std::signal(SIGABRT, crashHandler);
	std::signal(SIGFPE, crashHandler);
	std::signal(SIGILL, crashHandler);
#ifndef _WIN32
	std::signal(SIGTERM, crashHandler);
#ifdef SIGBUS
	std::signal(SIGBUS, crashHandler);
#endif
#endif
	std::set_terminate([](){
		std::string m = "std::terminate - excecao nao tratada!";
		writeLogFile("logs/crash.log", m);
		std::cerr << m << "\n"; 
		try{ std::rethrow_exception(std::current_exception()); }catch(const std::exception &e){
			std::string em = std::string("what(): ") + e.what();
			writeLogFile("logs/crash.log", em); std::cerr << em << "\n";
		}catch(...){ writeLogFile("logs/crash.log","excecao desconhecida"); }
		crashHandler(SIGABRT);
	});
}
void reportError(const char *message)
{
	std::string msg = message;
	writeLogFile("logs/game.log", msg);
	std::cout << msg << "\n";
	std::cerr << msg << "\n";
}
void reportErrorDetailed(const char *file, int line, const char *func, const char *message){
	std::string rel = file;
	auto pos = rel.find("ourCraft/");
	if(pos!=std::string::npos) rel = rel.substr(pos+9);
	std::string msg = rel + ":" + std::to_string(line) + " (" + func + ") -> " + message;
	reportError(msg.c_str());
}
