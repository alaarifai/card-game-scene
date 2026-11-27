#include "libs/glad_loader_docs.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader_m.h>
#include "tools/Polygon.h"
#include "tools/TextureLoader.h"
#include "Room.h"
#include "tools/Box.h"
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
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Card Game", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    Shader sceneShader("./shaders/vs/scene.vs", "./shaders/fs/scene.fs");
    buildRoom();

    
    std::vector<GLuint> cardTextures = loadTextures({
        "./textures/card0.jpg",
        "./textures/card1.jpg",
        "./textures/card2.jpg",
        "./textures/card3.jpg",
        "./textures/card4.jpg"});

    if (cardTextures.size() != 5)
    {
        std::cerr << "Missing card textures (card0.jpg ... card4.jpg)" << std::endl;
        glfwTerminate();
        return -1;
    }

    
    const float CARD_Y = 0.03f;   
    const float DECK_Z = 0.75f;   
    const float PLAY_Z = 2.75f;   

    std::vector<Card> deck;
    for (int i = 0; i < 5; i++)
    {
        vec3 position(-2.6f + 1.3f * i, CARD_Y, DECK_Z);
        deck.push_back(Card(i, cardTextures[i], position, 0.0f));
    }

    std::vector<int> remainingCards = {0, 1, 2, 3, 4}; 
    int playedCount = 0;                               

    
    bool radioPlaying = false;
    MCIERROR radioError = mciSendStringA("open \"./audio/radio.wav\" type waveaudio alias radio", NULL, 0, NULL);
    bool radioReady = (radioError == 0);
    if (radioReady)
        std::cout << "Press R to play / stop the radio" << std::endl;
    else
        std::cerr << "Could not open audio/radio.wav" << std::endl;

    
    bool gWasPressed = false;
    bool rWasPressed = false;
    bool tWasPressed = false;
    int inspectedCard = -1;   
    vec3 inspectedHome(0.0f); 

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    float lastFrame = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        bool gIsPressed = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
        bool rIsPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        bool tIsPressed = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;

        
        if (rIsPressed && !rWasPressed && radioReady)
        {
            if (radioPlaying)
                mciSendStringA("stop radio", NULL, 0, NULL);
            else
            {
                mciSendStringA("seek radio to start", NULL, 0, NULL);
                mciSendStringA("play radio", NULL, 0, NULL);
            }
            radioPlaying = !radioPlaying;
        }
        rWasPressed = rIsPressed;

        
        if (radioPlaying)
        {
            char mode[32] = {};
            mciSendStringA("status radio mode", mode, sizeof(mode), NULL);
            if (std::strcmp(mode, "stopped") == 0)
            {
                mciSendStringA("seek radio to start", NULL, 0, NULL);
                mciSendStringA("play radio", NULL, 0, NULL);
            }
        }

        
        if (gIsPressed && !gWasPressed && !remainingCards.empty())
        {
            int randomIndex = std::rand() % static_cast<int>(remainingCards.size());
            int cardId = remainingCards[randomIndex];
            remainingCards.erase(remainingCards.begin() + randomIndex);

            deck[cardId].moveTo(vec3(-2.6f + 1.3f * playedCount, CARD_Y, PLAY_Z));
            playedCount++;

            PlaySoundA("./audio/card_slide_better.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        gWasPressed = gIsPressed;

        
        if (cameraMode == 1 && inspectedCard != -1)
        {
            deck[inspectedCard].moveTo(inspectedHome);
            inspectedCard = -1;
        }
        if (tIsPressed && !tWasPressed && cameraMode == 2)
        {
            if (inspectedCard != -1)
            {
                deck[inspectedCard].moveTo(inspectedHome);
                inspectedCard = -1;
            }
            else
            {
                std::vector<int> candidates;
                for (int i = 0; i < 5; i++)
                {
                    bool inDeck = std::find(remainingCards.begin(), remainingCards.end(), i) != remainingCards.end();
                    if (!inDeck && !deck[i].moving)
                        candidates.push_back(i);
                }
                if (!candidates.empty())
                {
                    int id = candidates[std::rand() % static_cast<int>(candidates.size())];
                    inspectedCard = id;
                    inspectedHome = deck[id].targetPosition;
                    deck[id].moveTo(vec3(0.0f, 3.7f, 7.9f));
                }
            }
        }
        tWasPressed = tIsPressed;

        
        glClearColor(0.12f, 0.14f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sceneShader.use();

        float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
        mat4 projection = perspective(radians(45.0f), aspect, 0.1f, 100.0f);
        sceneShader.setMat4("projection", projection);

        mat4 view;
        if (cameraMode == 1)
            view = lookAt(vec3(0.0f, 11.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
        else
            view = lookAt(vec3(0.0f, 6.5f, 12.5f), vec3(0.0f, 0.5f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
        sceneShader.setMat4("view", view);

        drawRoom(sceneShader);

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

    if (radioReady)
    {
        mciSendStringA("stop radio", NULL, 0, NULL);
        mciSendStringA("close radio", NULL, 0, NULL);
    }
    PlaySoundA(NULL, NULL, 0);
    for (Card &card : deck)
        card.deleteBuffers();
    destroyRoom();
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
    windowHeight = height > 0 ? height : 1;
}
