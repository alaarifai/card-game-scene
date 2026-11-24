#include "libs/glad_loader_docs.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <filesystem>
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

// Build asset paths from the executable's folder, not from the terminal's working folder.
static std::string getAssetPath(const std::string& relativePath)
{
    char executablePath[MAX_PATH] = {};
    DWORD length = GetModuleFileNameA(NULL, executablePath, MAX_PATH);

    if (length == 0 || length >= MAX_PATH)
        return relativePath;

    return (std::filesystem::path(executablePath).parent_path() / relativePath).string();
}

static void printMciError(MCIERROR error, const char* action)
{
    if (error == 0)
        return;

    char message[256] = {};
    mciGetErrorStringA(error, message, sizeof(message));
    std::cerr << "Radio " << action << " failed: " << message << std::endl;
}

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
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader sceneShader("./shaders/vs/scene.vs", "./shaders/fs/scene.fs");
    buildRoom();

    // Load the five face textures and one shared back texture.
    std::vector<GLuint> cardTextures = loadTextures({
        "./textures/card0.jpg",
        "./textures/card1.jpg",
        "./textures/card2.jpg",
        "./textures/card3.jpg",
        "./textures/card4.jpg"
    });

    std::vector<GLuint> backTextures = loadTextures(
        {"./textures/card_back.png"}, GL_CLAMP_TO_EDGE, GL_LINEAR);

    if (cardTextures.size() != 5 || backTextures.size() != 1)
    {
        std::cerr << "Missing card textures. Need card0.jpg ... card4.jpg and card_back.png" << std::endl;
        destroyRoom();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // The felt surface is y = 0. A small positive offset keeps cards above it.
    const float CARD_Y = 0.03f;
    const vec3 deckPosition(4.0f, CARD_Y, 2.75f);

    std::vector<Card> deck;
    for (int i = 0; i < 5; i++)
        deck.push_back(Card(i, cardTextures[i], deckPosition, 0.01f * i));

    std::vector<int> remainingCards = {0, 1, 2, 3, 4};
    int playedCount = 0;

    // The opponent's five cards are face-down from the beginning.
    std::vector<Card> opponentCards;
    for (int i = 0; i < 5; i++)
    {
        vec3 opponentPosition(-2.6f + 1.3f * i, CARD_Y, -1.25f);
        opponentCards.push_back(Card(100 + i, backTextures[0], opponentPosition, 0.0f));
    }

    // Extra decorative cards stay fixed on the left and right sides.
    std::vector<Card> sideCards;
    for (int i = 0; i < 3; i++)
    {
        float z = -2.0f + 2.0f * i;
        sideCards.push_back(Card(200 + i, backTextures[0], vec3(-5.15f, CARD_Y, z), 0.0f));
        sideCards.push_back(Card(210 + i, backTextures[0], vec3( 5.15f, CARD_Y, z), 0.0f));
    }

    // Use paths beside GLProject.exe so playback works from VS Code and from a shortcut.
    const std::string radioPath = getAssetPath("audio/radio.wav");
    const std::string cardSoundPath = getAssetPath("audio/card_slide_better.wav");

    bool radioMciOpen = false;
    bool radioMciPlaying = false;
    bool radioFallbackPlaying = false;

    std::error_code fileError;
    if (std::filesystem::exists(radioPath, fileError))
    {
        // MCI plays the music independently, so card effects do not interrupt it.
        const std::string openRadioCommand =
            "open \"" + radioPath + "\" type waveaudio alias radioTrack wait";
        MCIERROR radioError = mciSendStringA(openRadioCommand.c_str(), NULL, 0, NULL);

        if (radioError == 0)
        {
            radioMciOpen = true;
            // MCI volume range is 0..1000. Keep the radio at a comfortable level.
            mciSendStringA("setaudio radioTrack volume to 700", NULL, 0, NULL);
            std::cout << "Radio file found: " << radioPath << std::endl;
            std::cout << "Press R to play/stop the radio." << std::endl;
        }
        else
        {
            printMciError(radioError, "open");
            std::cerr << "Will try the Windows WAV playback fallback when R is pressed." << std::endl;
        }
    }
    else
    {
        std::cerr << "Radio file not found: " << radioPath << std::endl;
        std::cerr << "Keep radio.wav in the audio folder beside GLProject.exe." << std::endl;
    }

    if (!std::filesystem::exists(cardSoundPath, fileError))
        std::cerr << "Card sound file not found: " << cardSoundPath << std::endl;

    bool gWasPressed = false;
    bool rWasPressed = false;
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

        // R toggles the radio. MCI is preferred because it can play alongside card sounds.
        if (rIsPressed && !rWasPressed)
        {
            if (radioMciPlaying)
            {
                MCIERROR stopError = mciSendStringA("stop radioTrack", NULL, 0, NULL);
                printMciError(stopError, "stop");
                mciSendStringA("seek radioTrack to start", NULL, 0, NULL);
                radioMciPlaying = false;
            }
            else if (radioFallbackPlaying)
            {
                PlaySoundA(NULL, NULL, 0);
                radioFallbackPlaying = false;
            }
            else
            {
                bool started = false;

                if (radioMciOpen)
                {
                    MCIERROR playError = mciSendStringA("play radioTrack repeat", NULL, 0, NULL);
                    if (playError == 0)
                    {
                        radioMciPlaying = true;
                        started = true;
                    }
                    else
                    {
                        printMciError(playError, "play");
                    }
                }

                // Fallback for WAV files/devices that MCI cannot open on this computer.
                if (!started)
                {
                    BOOL soundStarted = PlaySoundA(
                        radioPath.c_str(), NULL,
                        SND_FILENAME | SND_ASYNC | SND_LOOP | SND_NODEFAULT);

                    if (soundStarted)
                    {
                        radioFallbackPlaying = true;
                        std::cout << "Radio playing with WAV fallback." << std::endl;
                    }
                    else
                    {
                        std::cerr << "Could not start radio. Check that audio/radio.wav is a standard WAV file." << std::endl;
                    }
                }
            }
        }
        rWasPressed = rIsPressed;

        // Each G press deals one random remaining card into the player's row.
        // After all five cards have been dealt, further presses do nothing.
        if (gIsPressed && !gWasPressed && !remainingCards.empty())
        {
            int randomIndex = std::rand() % static_cast<int>(remainingCards.size());
            int cardId = remainingCards[randomIndex];
            remainingCards.erase(remainingCards.begin() + randomIndex);

            deck[cardId].moveTo(vec3(-2.6f + 1.3f * playedCount, CARD_Y, 2.75f));

            // Paper-slide sound is separate from the looping radio music.
            PlaySoundA(cardSoundPath.c_str(), NULL,
                       SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            playedCount++;
        }
        gWasPressed = gIsPressed;

        glClearColor(0.12f, 0.14f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sceneShader.use();

        float aspect = windowHeight > 0
            ? static_cast<float>(windowWidth) / static_cast<float>(windowHeight)
            : 1.0f;

        mat4 projection = perspective(radians(45.0f), aspect, 0.1f, 100.0f);
        sceneShader.setMat4("projection", projection);

        mat4 view;
        if (cameraMode == 1)
        {
            // Camera 1: directly above the table; cards lie flat.
            view = lookAt(vec3(0.0f, 11.0f, 0.0f),
                          vec3(0.0f, 0.0f, 0.0f),
                          vec3(0.0f, 0.0f, -1.0f));
        }
        else
        {
            // Camera 2: player's view; cards stand upright.
            view = lookAt(vec3(0.0f, 6.0f, 11.0f),
                          vec3(0.0f, 0.5f, -1.0f),
                          vec3(0.0f, 1.0f, 0.0f));
        }
        sceneShader.setMat4("view", view);

        drawRoom(sceneShader);

        // Player deck / cards already dealt.
        for (Card &card : deck)
        {
            if (cameraMode == 2)
                card.stand();
            else
                card.lay();

            card.update(deltaTime);
            card.draw(sceneShader);
        }

        // Opponent cards remain face-down in both camera modes.
        for (Card &card : opponentCards)
        {
            if (cameraMode == 2)
                card.stand();
            else
                card.lay();

            card.update(deltaTime);
            card.draw(sceneShader);
        }

        // Decorative side cards also follow the two camera positions.
        for (Card &card : sideCards)
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

    if (radioMciPlaying || radioMciOpen)
        mciSendStringA("stop radioTrack", NULL, 0, NULL);
    if (radioMciOpen)
        mciSendStringA("close radioTrack", NULL, 0, NULL);

    if (radioFallbackPlaying)
        PlaySoundA(NULL, NULL, 0);
    PlaySoundA(NULL, NULL, 0);
    destroyRoom();
    glDeleteTextures(static_cast<GLsizei>(cardTextures.size()), cardTextures.data());
    glDeleteTextures(static_cast<GLsizei>(backTextures.size()), backTextures.data());

    glfwDestroyWindow(window);
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
