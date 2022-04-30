#include "utils.hpp"
#include <span>

char buffer[SCRATCH_BUFFER_SIZE];
std::span<u8> bufferU8((u8*)buffer, SCRATCH_BUFFER_SIZE);

namespace shader_srcs
{

ConstStr header =
R"GLSL(
#version 330
#define PI 3.1415926535897932
)GLSL";

}

static const char* geGlErrStr(GLenum const err)
{
	switch (err) {
	case GL_NO_ERROR: return "GL_NO_ERROR";
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
	case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
	case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
	default:
		assert(!"unknown error");
		return nullptr;
	}
}

void glErrorCallback(const char* name, void* funcptr, int len_args, ...) {
	GLenum error_code;
	error_code = glad_glGetError();
	if (error_code != GL_NO_ERROR) {
		fprintf(stderr, "ERROR %s in %s\n", geGlErrStr(error_code), name);
		assert(false);
	}
}

// --- shader utils ---

char* checkCompileErrors(u32 shad, std::span<char> buffer)
{
    i32 ok;
    glGetShaderiv(shad, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLsizei outSize;
        glGetShaderInfoLog(shad, buffer.size(), &outSize, buffer.data());
        return buffer.data();
    }
    return nullptr;
}

char* checkLinkErrors(u32 prog, std::span<char> buffer)
{
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(prog, buffer.size(), nullptr, buffer.data());
        return buffer.data();
    }
    return nullptr;
}

static void printCodeWithLines(std::span<const char*> srcs)
{
    printf("%4d| ", 0);
    int line = 1;
    for (const char* s : srcs)
    {
        int start = 0;
        int end = 0;
        while (s[end]) {
            if (s[end] == '\n') {
                printf("%.*s\n", end - start, s + start);
                printf("%4d| ", line);
                start = end = end + 1;
                line++;
            }
            else
                end++;
        }
    }
    printf("\n");
}

void printShaderCodeWithHeader(const char* src)
{
    const char* srcs[2] = { shader_srcs::header, src };
    printCodeWithLines(srcs);
}

u32 easyCreateShader(const char* name, const char* src, GLenum type)
{
    static ConstStr s_shaderTypeNames[] = { "VERT", "FRAG", "GEOM" };
    const char* typeName = nullptr;
    switch (type) {
    case GL_VERTEX_SHADER:
        typeName = s_shaderTypeNames[0]; break;
    case GL_FRAGMENT_SHADER:
        typeName = s_shaderTypeNames[1]; break;
    case GL_GEOMETRY_SHADER:
        typeName = s_shaderTypeNames[2]; break;
    default:
        assert(false);
    }

    const u32 shad = glCreateShader(type);
    ConstStr srcs[] = { shader_srcs::header, src };
    glShaderSource(shad, 2, srcs, nullptr);
    glCompileShader(shad);
    if (const char* errMsg = checkCompileErrors(shad, buffer)) {
        printf("Error in '%s'(%s):\n%s", name, typeName, errMsg);
        printShaderCodeWithHeader(src);
        assert(false);
    }
    return shad;
}

u32 easyCreateShaderProg(const char* name, const char* vertShadSrc, const char* fragShadSrc, u32 vertShad, u32 fragShad)
{
    u32 prog = glCreateProgram();

    glAttachShader(prog, vertShad);
    glAttachShader(prog, fragShad);
    defer(
        glDetachShader(prog, vertShad);
    glDetachShader(prog, fragShad);
    );

    glLinkProgram(prog);
    if (const char* errMsg = checkLinkErrors(prog, buffer)) {
        printf("%s\n", errMsg);
        printf("Vertex Shader:\n");
        printShaderCodeWithHeader(vertShadSrc);
        printf("Fragment Shader:\n");
        printShaderCodeWithHeader(fragShadSrc);
        assert(false);
    }

    return prog;
}

u32 easyCreateShaderProg(const char* name, const char* vertShadSrc, const char* fragShadSrc)
{
    const u32 vertShad = easyCreateShader(name, vertShadSrc, GL_VERTEX_SHADER);
    const u32 fragShad = easyCreateShader(name, fragShadSrc, GL_FRAGMENT_SHADER);
    const u32 prog = easyCreateShaderProg(name, vertShadSrc, fragShadSrc, vertShad, fragShad);
    glDeleteShader(vertShad);
    glDeleteShader(fragShad);
    return prog;
}
