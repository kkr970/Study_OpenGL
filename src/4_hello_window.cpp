#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 버전 3 사용
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Core Profile 사용
    //glfwWindowHInt(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Mac일경우 사용

    //800, 600의 크기, 이름은 "Study_OpenGL"의 window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Study_OpenGL", NULL, NULL);
    // window생성 오류
    if(window == NULL)
    {
        std::cout << "Fail to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //현재 쓰레드의 context를 window로 설정
    glfwMakeContextCurrent(window);
    //GLAD초기화 오류
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    //frame이 바뀌면 콜백, viewport가 유동적으로 바뀜
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //창이 종료 명령이 있을때 까지 while
    while(!glfwWindowShouldClose(window))
    {   
        //키 입력
        processInput(window);

        //색은 R, G, B, A 순
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //viewport설정
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}