#include<glad/glad.h>
#include<glfw/glfw3.h>
#include<stdio.h>
#include "vector.h"

const char* vertexShaderSource = 
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos, 1.0);\n"
"}\n";

const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(1.0, 0.5, 0.3, 1.0);\n"
"}\n";


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

vector(float) vertices;
vector(unsigned int) indices;

int main(int argc, char const *argv[])
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800,600,"hell world",NULL,NULL);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1); // for vsync


    // vertices
    vector_init(vertices);
    vector_push_back_args(vertices, -0.5, -0.5,0.0); // x0, y0
    vector_push_back_args(vertices, 0.5, -0.5,0.0); // x1, y1
    vector_push_back_args(vertices, 0.5, 0.5,0.0); // x2, y2
    vector_push_back_args(vertices,-0.5, 0.5,0.0); // x3, y3
    
    // indices
    vector_init(indices);
    vector_push_back_args(indices, 0,1,2,2,3,0);

    unsigned int VBO,EBO,VAO;
    glGenVertexArrays(1,&VAO);
    
    glBindVertexArray(VAO);
    
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER, vector_size(vertices) * sizeof(float), vertices.data,GL_STATIC_DRAW);

    glGenBuffers(1,&EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,vector_size(indices) * sizeof(unsigned int), indices.data,GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);


    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,&vertexShaderSource,NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&fragmentShaderSource,NULL);
    glCompileShader(fragmentShader);

    unsigned int program = glCreateProgram();
    glAttachShader(program,vertexShader);
    glAttachShader(program,fragmentShader);
    glLinkProgram(program);
	glUseProgram(program);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
    
        glfwSwapBuffers(window);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteBuffers(1,&VBO);
    glDeleteBuffers(1,&EBO);
    glDeleteVertexArrays(1,&VAO);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    vector_destroy(vertices);
    vector_destroy(indices);
    glfwTerminate();
    printf("Destroyed all memory blocks\n");

    
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);  // Viewport'u yeni pencere boyutuna göre ayarla
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {  // Sadece tuşa basıldığında işlem yap
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);  // Pencereyi kapat
        }
    }
}
