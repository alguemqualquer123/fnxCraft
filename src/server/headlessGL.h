#ifdef OURCRAFT_HEADLESS

// Minimal OpenGL stubs for headless compilation
// These are never called, just satisfy linker

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef float GLfloat;
typedef double GLdouble;
typedef unsigned char GLubyte;
typedef char GLchar;
typedef unsigned int GLbitfield;
typedef long GLintptr;
typedef long GLsizeiptr;
typedef void GLvoid;

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_NO_ERROR 0
#define GL_UNSIGNED_BYTE 0x1401
#define GL_FLOAT 0x1406
#define GL_UNSIGNED_INT 0x1405
#define GL_INT 0x1404
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_BYTE 0x1400
#define GL_DOUBLE 0x140A
#define GL_RGBA 0x1908
#define GL_RGB 0x1907
#define GL_RED 0x1903
#define GL_RG 0x8227
#define GL_DEPTH_COMPONENT 0x1902
#define GL_RGBA8 0x8058
#define GL_RGB8 0x8051
#define GL_R8 0x8229
#define GL_RG8 0x822B
#define GL_RGBA16F 0x881A
#define GL_RGB16F 0x881B
#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815
#define GL_R16F 0x822D
#define GL_RG16F 0x822F
#define GL_RGBA16 0x805B
#define GL_RGB16 0x8054
#define GL_RGB16UI 0x8D77
#define GL_RGBA16UI 0x8D76
#define GL_RGB_INTEGER 0x8D98
#define GL_RGBA_INTEGER 0x8D99
#define GL_R11F_G11F_B10F 0x8C3A
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_LINEAR 0x2601
#define GL_NEAREST 0x2600
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_REPEAT 0x2901
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_FRAMEBUFFER 0x8D40
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_COLOR_ATTACHMENT2 0x8CE2
#define GL_COLOR_ATTACHMENT3 0x8CE3
#define GL_COLOR_ATTACHMENT4 0x8CE4
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_DEPTH_BUFFER_BIT 0x0100
#define GL_STENCIL_BUFFER_BIT 0x0400
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_STREAM_DRAW 0x88E0
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_LINES 0x0001
#define GL_LINE_STRIP 0x0003
#define GL_POINTS 0x0000
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_ONE 1
#define GL_ZERO 0
#define GL_BLEND 0x0BE2
#define GL_DEPTH_TEST 0x0B71
#define GL_CULL_FACE 0x0B44
#define GL_STENCIL_TEST 0x0B90
#define GL_SCISSOR_TEST 0x0C11
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_FRONT_AND_BACK 0x0408
#define GL_FILL 0x1B02
#define GL_LINE 0x1B01
#define GL_POINT 0x1B00
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_DEBUG_SOURCE_APPLICATION 0x8249
#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_COMPUTE_SHADER 0x91B9
#define GL_TRUE 1
#define GL_FALSE 0
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_COMPARE_REF_TO_TEXTURE 0x884E
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_DEPTH32F_STENCIL8 0x8CAD
#define GL_RENDERBUFFER 0x8D41
#define GL_READ_ONLY 0x88B9
#define GL_WRITE_ONLY 0x88B8
#define GL_READ_WRITE 0x88BA
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_ATOMIC_COUNTER_BUFFER 0x92C0
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_ACCESS 0x88BB
#define GL_MAP_READ_BIT 0x0001
#define GL_MAP_WRITE_BIT 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT 0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#define GL_MAP_UNSYNCHRONIZED_BIT 0x0020
#define GL_MAP_FLUSH_EXPLICIT_BIT 0x0010
#define GL_TEXTURE_BORDER_COLOR 0x1004
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_TEXTURE_SWIZZLE_R 0x8E42
#define GL_TEXTURE_SWIZZLE_G 0x8E43
#define GL_TEXTURE_SWIZZLE_B 0x8E44
#define GL_TEXTURE_SWIZZLE_A 0x8E45
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_GENERATE_MIPMAP 0x8191
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#define GL_TEXTURE_3D 0x806F
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_MAX_SAMPLES 0x8D57
#define GL_RENDERBUFFER_SAMPLES 0x8CAB
#define GL_DEPTH_STENCIL 0x84F9
#define GL_RED_INTEGER 0x8D94
#define GL_GREEN_INTEGER 0x8D95
#define GL_BLUE_INTEGER 0x8D96
#define GL_ALPHA_INTEGER 0x8D97
#define GL_RGB_INTEGER 0x8D98
#define GL_RGBA_INTEGER 0x8D99
#define GL_BGR_INTEGER 0x8D9A
#define GL_BGRA_INTEGER 0x8D9B

inline void glEnable(GLenum cap) {}
inline void glDisable(GLenum cap) {}
inline void glBlendFunc(GLenum sfactor, GLenum dfactor) {}
inline void glBlendFunci(GLuint buf, GLenum sfactor, GLenum dfactor) {}
inline void glBlendEquation(GLenum mode) {}
inline void glDepthFunc(GLenum func) {}
inline void glDepthMask(unsigned char flag) {}
inline void glCullFace(GLenum mode) {}
inline void glFrontFace(GLenum mode) {}
inline void glPolygonMode(GLenum face, GLenum mode) {}
inline void glViewport(GLint x, GLint y, GLsizei w, GLsizei h) {}
inline void glScissor(GLint x, GLint y, GLsizei w, GLsizei h) {}
inline void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {}
inline void glClearDepth(GLdouble depth) {}
inline void glClearDepthf(GLfloat depth) {}
inline void glClear(GLbitfield mask) {}
inline void glClearBufferfv(GLenum buf, GLint drawbuf, const GLfloat *value) {}
inline void glClearBufferiv(GLenum buf, GLint drawbuf, const GLint *value) {}
inline void glClearBufferuiv(GLenum buf, GLint drawbuf, const GLuint *value) {}
inline GLenum glGetError(void) { return GL_NO_ERROR; }
inline void glGetIntegerv(GLenum pname, GLint *params) {}
inline void glGetFloatv(GLenum pname, GLfloat *params) {}
inline void glGetDoublev(GLenum pname, GLdouble *params) {}
inline const GLubyte *glGetString(GLenum name) { return (const GLubyte *)""; }
inline const GLubyte *glGetStringi(GLenum name, GLuint index) { return (const GLubyte *)""; }
inline void glReadPixels(GLint x, GLint y, GLsizei w, GLsizei h, GLenum fmt, GLenum type, void *data) {}
inline void glFinish(void) {}
inline void glFlush(void) {}
inline void glPixelStorei(GLenum pname, GLint param) {}
inline void glLineWidth(GLfloat w) {}
inline void glPointSize(GLfloat s) {}

// Texture functions
inline void glGenTextures(GLsizei n, GLuint *textures) { for (GLsizei i = 0; i < n; i++) textures[i] = 0; }
inline void glDeleteTextures(GLsizei n, const GLuint *textures) {}
inline void glBindTexture(GLenum target, GLuint texture) {}
inline void glActiveTexture(GLenum texture) {}
inline void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {}
inline void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {}
inline void glTexParameteri(GLenum target, GLenum pname, GLint param) {}
inline void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {}
inline void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params) {}
inline void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei w, GLsizei h, GLint border) {}
inline void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei w, GLsizei h) {}
inline void glGenerateMipmap(GLenum target) {}
inline void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels) {}
inline void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params) {}

// Framebuffer functions
inline void glGenFramebuffers(GLsizei n, GLuint *framebuffers) { for (GLsizei i = 0; i < n; i++) framebuffers[i] = 0; }
inline void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) {}
inline void glBindFramebuffer(GLenum target, GLuint framebuffer) {}
inline void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {}
inline void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {}
inline GLenum glCheckFramebufferStatus(GLenum target) { return GL_FRAMEBUFFER_COMPLETE; }
inline void glDrawBuffer(GLenum mode) {}
inline void glReadBuffer(GLenum mode) {}
inline void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {}
inline void glDrawBuffers(GLsizei n, const GLenum *bufs) {}

// Renderbuffer functions
inline void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) { for (GLsizei i = 0; i < n; i++) renderbuffers[i] = 0; }
inline void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) {}
inline void glBindRenderbuffer(GLenum target, GLuint renderbuffer) {}
inline void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {}
inline void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {}

// Buffer functions
inline void glGenBuffers(GLsizei n, GLuint *buffers) { for (GLsizei i = 0; i < n; i++) buffers[i] = 0; }
inline void glDeleteBuffers(GLsizei n, const GLuint *buffers) {}
inline void glBindBuffer(GLenum target, GLuint buffer) {}
inline void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {}
inline void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {}
inline void *glMapBuffer(GLenum target, GLenum access) { return nullptr; }
inline void *glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) { return nullptr; }
inline GLboolean glUnmapBuffer(GLenum target) { return GL_TRUE; }
inline void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void *data) {}

// VAO functions
inline void glGenVertexArrays(GLsizei n, GLuint *arrays) { for (GLsizei i = 0; i < n; i++) arrays[i] = 0; }
inline void glDeleteVertexArrays(GLsizei n, const GLuint *arrays) {}
inline void glBindVertexArray(GLuint array) {}
inline void glEnableVertexAttribArray(GLuint index) {}
inline void glDisableVertexAttribArray(GLuint index) {}
inline void glVertexAttribPointer(GLuint index, GLint size, GLenum type, unsigned char normalized, GLsizei stride, const void *pointer) {}
inline void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer) {}
inline void glVertexAttribDivisor(GLuint index, GLuint divisor) {}

// Draw functions
inline void glDrawArrays(GLenum mode, GLint first, GLsizei count) {}
inline void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {}
inline void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {}
inline void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) {}
inline void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance) {}
inline void glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance) {}
inline void glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount) {}
inline void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei drawcount) {}

// Shader functions
inline GLuint glCreateShader(GLenum type) { return 0; }
inline void glDeleteShader(GLuint shader) {}
inline void glShaderSource(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length) {}
inline void glCompileShader(GLuint shader) {}
inline void glGetShaderiv(GLuint shader, GLenum pname, GLint *params) {}
inline void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {}
inline void glAttachShader(GLuint program, GLuint shader) {}
inline void glDetachShader(GLuint program, GLuint shader) {}

// Program functions
inline GLuint glCreateProgram(void) { return 0; }
inline void glDeleteProgram(GLuint program) {}
inline void glLinkProgram(GLuint program) {}
inline void glValidateProgram(GLuint program) {}
inline void glGetProgramiv(GLuint program, GLenum pname, GLint *params) {}
inline void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {}
inline void glUseProgram(GLuint program) {}
inline GLint glGetUniformLocation(GLuint program, const GLchar *name) { return -1; }
inline GLuint glGetAttribLocation(GLuint program, const GLchar *name) { return 0; }
inline GLuint glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName) { return 0; }
inline void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {}
inline GLuint glGetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar *name) { return 0; }
inline void glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding) {}
inline void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) {}
inline void glMemoryBarrier(GLbitfield barriers) {}
inline void glDispatchComputeIndirect(GLintptr indirect) {}
inline void glMultiDrawArraysIndirect(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride) {}
inline void glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride) {}

// Uniform functions
inline void glUniform1i(GLint location, GLint v0) {}
inline void glUniform2i(GLint location, GLint v0, GLint v1) {}
inline void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {}
inline void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {}
inline void glUniform1f(GLint location, GLfloat v0) {}
inline void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {}
inline void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {}
inline void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {}
inline void glUniform1fv(GLint location, GLsizei count, const GLfloat *value) {}
inline void glUniform2fv(GLint location, GLsizei count, const GLfloat *value) {}
inline void glUniform3fv(GLint location, GLsizei count, const GLfloat *value) {}
inline void glUniform4fv(GLint location, GLsizei count, const GLfloat *value) {}
inline void glUniform1iv(GLint location, GLsizei count, const GLint *value) {}
inline void glUniform2iv(GLint location, GLsizei count, const GLint *value) {}
inline void glUniform3iv(GLint location, GLsizei count, const GLint *value) {}
inline void glUniform4iv(GLint location, GLsizei count, const GLint *value) {}
inline void glUniformMatrix2fv(GLint location, GLsizei count, unsigned char transpose, const GLfloat *value) {}
inline void glUniformMatrix3fv(GLint location, GLsizei count, unsigned char transpose, const GLfloat *value) {}
inline void glUniformMatrix4fv(GLint location, GLsizei count, unsigned char transpose, const GLfloat *value) {}
inline void glUniform1ui(GLint location, GLuint v0) {}
inline void glUniform2ui(GLint location, GLuint v0, GLuint v1) {}
inline void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2) {}
inline void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {}
inline void glUniform1uiv(GLint location, GLsizei count, const GLuint *value) {}
inline void glProgramUniform1i(GLuint program, GLint location, GLint v0) {}
inline void glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1) {}
inline void glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2) {}
inline void glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {}
inline void glProgramUniform1f(GLuint program, GLint location, GLfloat v0) {}
inline void glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1) {}
inline void glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {}
inline void glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {}
inline void glProgramUniform1ui(GLuint program, GLint location, GLuint v0) {}
inline void glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, unsigned char transpose, const GLfloat *value) {}
inline void glBindBufferBase(GLenum target, GLuint index, GLuint buffer) {}
inline void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {}
inline void glFenceSync(GLenum condition, GLbitfield flags) {}
inline void glDeleteSync(void *sync) {}
inline GLenum glClientWaitSync(void *sync, GLbitfield flags, GLuint64 timeout) { return 0; }
inline void glWaitSync(void *sync, GLbitfield flags, GLint64 timeout) {}
inline void glDrawArraysIndirect(GLenum mode, const void *indirect) {}
inline void glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect) {}
inline void glEnablei(GLenum target, GLuint index) {}
inline void glDisablei(GLenum target, GLuint index) {}
inline void glDepthRangef(GLfloat n, GLfloat f) {}
inline void glPolygonOffset(GLfloat factor, GLfloat units) {}
inline void glStencilFunc(GLenum func, GLint ref, GLuint mask) {}
inline void glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {}
inline void glStencilMask(GLuint mask) {}
inline void glColorMask(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {}
inline void glLogicOp(GLenum opcode) {}
inline void glSampleCoverage(GLfloat value, unsigned char invert) {}
inline void glMinSampleShading(GLfloat value) {}
inline void glPatchParameteri(GLenum pname, GLint value) {}

// Debug
inline void glEnable(GLenum cap, GLuint index) {}
inline void glDebugMessageCallback(void *callback, void *userParam) {}
inline void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, unsigned char enabled) {}
inline void glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar *message) {}
inline void glPopDebugGroup(void) {}
inline void glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar *label) {}

// Query
inline void glGenQueries(GLsizei n, GLuint *ids) { for (GLsizei i = 0; i < n; i++) ids[i] = 0; }
inline void glDeleteQueries(GLsizei n, const GLuint *ids) {}
inline void glBeginQuery(GLenum target, GLuint id) {}
inline void glEndQuery(GLenum target) {}
inline void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params) {}
inline void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params) {}

// Timer
inline GLuint64 glGetGLuint64(GLenum target, GLuint index) { return 0; }

// Sync
inline void glFenceSync() {}
inline void glDeleteSync() {}
inline GLenum glClientWaitSync() { return 0; }
inline void glWaitSync() {}

typedef uint64_t GLuint64;
typedef void *GLsync;

#endif // OURCRAFT_HEADLESS
