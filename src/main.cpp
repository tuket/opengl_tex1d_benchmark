#include "utils.hpp"
#include <GLFW/glfw3.h>

constexpr int W = 1280;
constexpr int H = 1024;

GLFWwindow* window;

namespace shader_srcs
{

ConstStr vertShad =
R"GLSL(
layout(location = 0)in vec3 a_pos;
layout(location = 1)in float a_tc;

out float v_tc;

void main()
{
	gl_Position = vec4(a_pos, 1);
	v_tc = a_tc;
}
)GLSL";

ConstStr fragShad_1d =
R"GLSL(
layout(location = 0) out vec4 o_color;

in float v_tc;

uniform sampler1D u_tex;

void main()
{
	o_color = vec4(texture(u_tex, v_tc).rgb, 1);
}
)GLSL";

ConstStr fragShad_2d =
R"GLSL(
layout(location = 0) out vec4 o_color;

in float v_tc;

uniform sampler2D u_tex;

void main()
{
	o_color = vec4(texture(u_tex, vec2(v_tc, 0)).rgb, 1);
}
)GLSL";

}

static void glfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window = glfwCreateWindow(1280, 720, "pcv", nullptr, nullptr);
    if (window == nullptr)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (gladLoadGL() == 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }
    glad_set_post_callback(glErrorCallback);

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int w, int h) {} );
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {} );
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {} );
    glfwSetScrollCallback(window, [](GLFWwindow* window, double dx, double dy) {} );

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    });

    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropic);
    glClearColor(0.4, 0.4, 0.4, 0);
    //glEnable(GL_DEPTH_TEST);
    
    /*u32 fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    u32 rbo;
    glGenRenderbuffers(1, & rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, 1280, 1024);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    const GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);*/

    u32 vbo;
    u32 vao;
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        struct Vert { vec3 pos; float tc; };
        const Vert verts[] = {
            {{-1, -1, 0}, 0},
            {{+1, -1, 0}, 1},
            {{+1, +1, 0}, 1},
            {{-1, +1, 0}, 0},
        };
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, tc));
    }

    auto initTex = [](GLenum target) {
        if (true) {
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else {
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(target);
    };
    const GLenum targets[2] = { GL_TEXTURE_1D, GL_TEXTURE_2D };
    u32 tex[2];
    glGenTextures(2, tex);
    glm::u8vec3 texels[128];
    for (size_t i = 0; i < std::size(texels); i++)
        texels[i] = glm::u8vec3(2*i);
    // 1D
    glBindTexture(GL_TEXTURE_1D, tex[0]);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, texels);
    initTex(GL_TEXTURE_1D);
    // 2D
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, texels);
    initTex(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    u32 prog[2];
    prog[0] = easyCreateShaderProg("1d", shader_srcs::vertShad, shader_srcs::fragShad_1d);
    prog[1] = easyCreateShaderProg("2d", shader_srcs::vertShad, shader_srcs::fragShad_2d);
    glUseProgram(prog[0]);
    glUniform1i(glGetUniformLocation(prog[0], "u_tex"), 0);
    glUseProgram(prog[1]);
    glUniform1i(glGetUniformLocation(prog[1], "u_tex"), 0);

    glViewport(0, 0, W, H);
    glBindVertexArray(vao);

    double time[2] = { 0, 0 };

    const int initMode = 0; // Toggle if we start with 1D or 2D first. It could happen that the one that executes first is faster because it starts with cooler temps
    const int numBatches = 40;
    const int numFramesPerBatch = 2'500;
    for (int batchInd = initMode; batchInd < numBatches + initMode; batchInd++) {
        const int mode = batchInd % 2;
        glUseProgram(prog[mode]);
        glBindTexture(targets[mode], tex[mode]);
        const double timeStart = glfwGetTime();
        for (int i = 0; i < numFramesPerBatch; i++) {
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
        glFinish(); // this is important so all the drawing commands have finished
        time[mode] += glfwGetTime() - timeStart;
    }
    
    printf("1D: %g\n2D: %g\n", time[0], time[1]);
}