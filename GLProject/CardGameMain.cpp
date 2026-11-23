#include "libs/glad_loader_docs.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader_m.h>
#include "tools/Polygon.h"
#include "tools/TextureLoader.h"
#include "Card.h"
using namespace glm;
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
int windowWidth = SCR_WIDTH;
int windowHeight = SCR_HEIGHT;
int cameraMode = 1;
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Card Game", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    Shader sceneShader("./shaders/vs/scene.vs", "./shaders/fs/scene.fs");

    std::vector<GLuint> cardTextures = loadTextures({"./textures/crate.jpg",
                                                     "./textures/crate2.jpg",
                                                     "./textures/ball.jpg",
                                                     "./textures/white.jpg",
                                                     "./textures/alrihla.jpg"});

    vec3 deckPosition = vec3(4.0f, 0.0f, 2.75f);

    std::vector<Card> deck;
    for (int i = 0; i < 5; i++)
    {
        deck.push_back(Card(i, cardTextures[i], deckPosition, 0.01f * i));
    }
    std::vector<int> remainingCards = {0, 1, 2, 3, 4};
    int playedCount = 0;
    bool gWasPressed = false;
    srand((unsigned int)time(nullptr));
    float lastFrame = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        bool gIsPressed = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;

        if (gIsPressed && !gWasPressed && !remainingCards.empty())
        {
            int randomIndex = rand() % (int)remainingCards.size();
            int cardId = remainingCards[randomIndex];
            remainingCards.erase(remainingCards.begin() + randomIndex);

            deck[cardId].moveTo(vec3(-2.6f + 1.3f * playedCount, 0.0f, 2.75f));
            playedCount++;
        }

        gWasPressed = gIsPressed;

        glClearColor(0.12f, 0.14f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sceneShader.use();

        float aspect = windowHeight > 0 ? (float)windowWidth / (float)windowHeight : 1.0f;
        mat4 projection = perspective(radians(45.0f), aspect, 0.1f, 100.0f);
        sceneShader.setMat4("projection", projection);

        mat4 view;
        if (cameraMode == 1)
            view = lookAt(vec3(0.0f, 11.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
        else
            view = lookAt(vec3(0.0f, 6.0f, 11.0f), vec3(0.0f, 0.5f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
        sceneShader.setMat4("view", view);

        for (Card &card : deck)
        {
            if (cameraMode == 2)
                card.stand();
            else
                card.lay();
            card.update(deltaTime);
            card.draw(sceneShader);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        cameraMode = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        cameraMode = 2;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    windowWidth = width;
    windowHeight = height;
}