#https://github.com/alguemqualquer123/cmakeSetup


cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

#cmake_policy(SET CMP0069 NEW)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

#if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#	add_compile_options(-ffast-math)
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#	add_compile_options(/fp:fast)
#endif()


#make shure the runtime library is statically linked
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(fnxCraft)


if (MSVC)
	# Set the compiler flags for AVX
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
	# Uncomment the following line for AVX2 instead of AVX
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
else()
	# Set the compiler flags for AVX
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
	# Uncomment the following line for AVX2 instead of AVX
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -fpermissive -Wno-error -Wno-template-body -Wno-invalid-offsetof")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error")
endif()


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Configure Assimp
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE) # Make sure zlib is enabled if you're linking against libzstd_static
set(ASSIMP_BUILD_NO_EXCEPTIONS ON CACHE BOOL "" FORCE) 
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_EXPORTER OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_MTL_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_MTL_EXPORTER OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_EXPORTER OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTFB_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTFB_EXPORTER OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
option(ASSIMP_BUILD_STATIC_LIB "If the library shall be built as static library." ON)
set(ASSIMP_BUILD_ZLIB "1" CACHE STRING "If zlib is used, static linking. Specify Zlib < 1.3")
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)



add_subdirectory(thirdparty/glfw-3.3.7)
add_subdirectory(thirdparty/glad)
add_subdirectory(thirdparty/stb_image)
add_subdirectory(thirdparty/stb_truetype)
add_subdirectory(thirdparty/gl2d)
add_subdirectory(thirdparty/raudio)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/imgui-docking)
add_subdirectory(thirdparty/FastNoiseSIMD)
add_subdirectory(thirdparty/FastNoise2)
add_subdirectory(thirdparty/enet-1.3.18)
add_subdirectory(thirdparty/glui)
add_subdirectory(thirdparty/assimp-5.2.4)
add_subdirectory(thirdparty/magic_enum-0.9.3)
add_subdirectory(thirdparty/safeSave)
add_subdirectory(thirdparty/profilerLib)
add_subdirectory(thirdparty/zstd-1.5.5/build/cmake)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lua/CMakeLists.txt")
 add_subdirectory(thirdparty/lua)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/noiseTool/CMakeLists.txt")
 add_subdirectory(noiseTool)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/apps/noiseTool/CMakeLists.txt")
 add_subdirectory(apps/noiseTool)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/PBRTool/CMakeLists.txt")
 add_subdirectory(PBRTool)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/apps/PBRTool/CMakeLists.txt")
 add_subdirectory(apps/PBRTool)
endif()


# MY_SOURCES is a list of all the source files for my game 
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(FILTER MY_SOURCES EXCLUDE REGEX ".*/src/server/.*")
file(GLOB_RECURSE SHARED_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shared/*.cpp")


add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)

#set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")


target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/") # This is useful to get an ASSETS_PATH in your IDE during development but you should comment this if you compile a release version and uncomment the next line
#target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") # Uncomment this line to setup the ASSETS_PATH macro to the final assets directory when you share the game


target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} ${SHARED_SOURCES} )


if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")


	#add_compile_options(-fsanitize=address)
	#add_link_options(-fsanitize=address)

endif()

if(WIN32)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _WIN32) 
  
endif()

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/shared/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/gameLayer/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/platform/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lua")


target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm glfw glad stb_image stb_truetype 
	gl2d raudio imgui fastNoiseSIMD FastNoise2 enet glui
	magic_enum safeSave profilerLib
	libzstd_static assimp lua_static)

# Windows: ícone no executável
if(WIN32)
	target_sources(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/resources/icon.rc")
endif()

# ============================================
# OUTPUT SEPARADO: game/ e server/ com seus arquivos (sem misturar)
# ============================================
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/game"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>" "${CMAKE_SOURCE_DIR}/game/fnxCraft" 2>/dev/null || true
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/resources/icon.png" "${CMAKE_SOURCE_DIR}/game/icon.png" 2>/dev/null || true
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/resources/icon.ico" "${CMAKE_SOURCE_DIR}/game/icon.ico" 2>/dev/null || true
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/game/resources/scripts" 2>/dev/null || true
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/scripts" "${CMAKE_SOURCE_DIR}/game/resources/scripts" 2>/dev/null || true
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/config/lua.toml" "${CMAKE_SOURCE_DIR}/game/config/lua.toml" 2>/dev/null || true
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/game/config" 2>/dev/null || true
	COMMENT "Gerando pasta game/ com binário e ícone (separado de build)"
)


# ============================================
# DEDICATED SERVER (headless, no OpenGL)
# ============================================
file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cpp")
file(GLOB_RECURSE PERSISTENCE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/gameLayer/persistence/*.cpp")
set(SERVER_EXTRA_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/gameLayer/GamePaths.cpp")

add_executable(fnxCraftServer ${SERVER_SOURCES} ${PERSISTENCE_SOURCES} ${SERVER_EXTRA_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/safeSave/src/safeSave.cpp")

set_property(TARGET fnxCraftServer PROPERTY CXX_STANDARD 17)
set_property(TARGET fnxCraftServer PROPERTY OUTPUT_NAME "fnxCraftServer")

target_compile_definitions(fnxCraftServer PUBLIC FNXCRAFT_HEADLESS=1)
target_compile_definitions(fnxCraftServer PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
target_compile_definitions(fnxCraftServer PUBLIC PLATFORM_LINUX)

target_include_directories(fnxCraftServer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/shared/")
target_include_directories(fnxCraftServer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(fnxCraftServer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/gameLayer/")
target_include_directories(fnxCraftServer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/platform/")
target_include_directories(fnxCraftServer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/")
target_include_directories(fnxCraftServer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm/")
target_include_directories(fnxCraftServer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/safeSave/include/")
target_include_directories(fnxCraftServer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/enet-1.3.18/include/")

target_link_libraries(fnxCraftServer PRIVATE enet libzstd_static glm glad)

if(NOT MSVC)
	target_compile_options(fnxCraftServer PRIVATE -pthread)
	target_link_options(fnxCraftServer PRIVATE -pthread)
endif()

add_custom_command(TARGET fnxCraftServer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/server"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:fnxCraftServer>" "${CMAKE_SOURCE_DIR}/server/fnxCraftServer" 2>/dev/null || true
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/server/data/worlds"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/server/data/players"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/server/logs"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/server/backups"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/server/plugins"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/resources/icon.png" "${CMAKE_SOURCE_DIR}/server/icon.png" 2>/dev/null || true
	COMMENT "Gerando pasta server/ com binário e estrutura (separado de build)"
)


