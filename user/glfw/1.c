#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

#define WIN_W   512
#define WIN_H   512

#define FPS     60
#define TEX_W   256
#define TEX_H   256

static GLFWwindow *window; 

static float vertices[6][2] = {
    {-1, -1}, {1, -1}, {1, 1},
    {-1, -1}, {1, 1}, {-1, 1},
};

static unsigned char buf[TEX_W * TEX_H * 4];
void init();
void update();

static void glfw_err_callback(int error, const char *desc)
{
    fprintf(stderr, "> <  GLFW: (%d) %s\n", error, desc);
}

static void glfw_fbsz_callback(GLFWwindow *window, int w, int h)
{
    glViewport(0, 0, w, h);

    float xs = w / TEX_W;
    float ys = h / TEX_H;
    float s = (xs < ys ? xs : ys);
    float x = s * TEX_W;
    float y = s * TEX_H;
    glBufferData(GL_ARRAY_BUFFER,
        12 * sizeof(float), vertices, GL_STREAM_DRAW);
}

static inline GLuint load_shader(GLenum type, const char *source)
{
    GLuint shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, &source, NULL);
    glCompileShader(shader_id);

    GLint status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    char msg_buf[1024];
    glGetShaderInfoLog(shader_id, sizeof(msg_buf) - 1, NULL, msg_buf);
    fprintf(stderr, "OvO  Compilation log for %s shader\n",
        (type == GL_VERTEX_SHADER ? "vertex" :
         type == GL_FRAGMENT_SHADER ? "fragment" : "unknown (!)"));
    fputs(msg_buf, stderr);
    fprintf(stderr, "=v=  End\n");
    if (status != GL_TRUE) {
        fprintf(stderr, "> <  Shader compilation failed\n");
        return 0;
    }

    return shader_id;
}

int main()
{
    // -- Initialization --

    glfwSetErrorCallback(glfw_err_callback);

    if (!glfwInit()) {
        fprintf(stderr, "> <  Cannot initialize GLFW\n");
        return 2;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    window = glfwCreateWindow(WIN_W, WIN_H, "MIKAN", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "> <  Cannot create GLFW window\n");
        return 2;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "> <  Cannot initialize GLEW\n");
        return 2;
    }

    // -- OpenGL setup --

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

#define GLSL(__source) "#version 120\n" #__source

    const char *vshader_source = GLSL(
        attribute vec2 ppp;
        varying vec2 pos;
        void main()
        {
            gl_Position = vec4(ppp, 0.0, 1.0);
            pos = ppp;
        }
    );

    const char *fshader_source = GLSL(
        varying vec2 pos;
        uniform sampler2D tex;
        void main()
        {
            gl_FragColor = texture2D(tex, pos);
        }
    );

    GLuint vshader = load_shader(GL_VERTEX_SHADER, vshader_source);
    GLuint fshader = load_shader(GL_FRAGMENT_SHADER, fshader_source);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vshader);
    glAttachShader(prog, fshader);
    glLinkProgram(prog);
    glUseProgram(prog);

    GLuint ppp_attrib_index = glGetAttribLocation(prog, "ppp");
    glEnableVertexAttribArray(ppp_attrib_index);
    glVertexAttribPointer(ppp_attrib_index, 2, GL_FLOAT, GL_FALSE,
        2 * sizeof(float), 0);

    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glUniform1i(glGetUniformLocation(prog, "tex"), 0);

    glBufferData(GL_ARRAY_BUFFER,
        12 * sizeof(float), vertices, GL_STREAM_DRAW);

    // -- Event/render loop --

    init();

    float last_time = glfwGetTime(), cur_time;

    glfwSetFramebufferSizeCallback(window, glfw_fbsz_callback);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;

        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        while ((cur_time = glfwGetTime()) < last_time + 1.0f / FPS) { }
        last_time = cur_time;

        update();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_W, TEX_H,
            0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

void init()
{
    puts("^ ^  Hello, world!");
}

void update()
{
    static int count = 0;
    count++;
    for (int i = 0; i < TEX_H; i++)
    for (int j = 0; j < TEX_W; j++) {
        buf[(i * TEX_W + j) * 4] = 
        buf[(i * TEX_W + j) * 4 + 1] =
        buf[(i * TEX_W + j) * 4 + 2] =
        buf[(i * TEX_W + j) * 4 + 3] = (((i ^ j ^ (count & 64)) & 64) ? 255 : 0);
    }
}
