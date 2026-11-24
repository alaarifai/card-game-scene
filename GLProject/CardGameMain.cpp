#include "libs/glad_loader_docs.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <iterator>
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


static std::string findAssetPath(const std::string& relativePath)
{
    std::vector<std::filesystem::path> candidates;
    std::error_code pathError;
    std::filesystem::path workingFolder = std::filesystem::current_path(pathError);

    if (!pathError)
    {

        candidates.push_back(workingFolder / "GLProject" / relativePath);
        candidates.push_back(workingFolder.parent_path() / "GLProject" / relativePath);
        candidates.push_back(workingFolder / relativePath);
    }

    char executablePath[MAX_PATH] = {};
    DWORD length = GetModuleFileNameA(NULL, executablePath, MAX_PATH);
    if (length > 0 && length < MAX_PATH)
    {
        std::filesystem::path exeFolder = std::filesystem::path(executablePath).parent_path();
        candidates.push_back(exeFolder / relativePath);
    }

    std::error_code existsError;
    for (const std::filesystem::path& candidate : candidates)
    {
        if (std::filesystem::exists(candidate, existsError) && !existsError)
            return candidate.string();
        existsError.clear();
    }

    if (length > 0 && length < MAX_PATH)
        return (std::filesystem::path(executablePath).parent_path() / relativePath).string();
    return relativePath;
}

static unsigned short readU16(const std::vector<unsigned char>& bytes, size_t offset)
{
    return static_cast<unsigned short>(bytes[offset]) |
           (static_cast<unsigned short>(bytes[offset + 1]) << 8);
}

static unsigned int readU32(const std::vector<unsigned char>& bytes, size_t offset)
{
    return static_cast<unsigned int>(bytes[offset]) |
           (static_cast<unsigned int>(bytes[offset + 1]) << 8) |
           (static_cast<unsigned int>(bytes[offset + 2]) << 16) |
           (static_cast<unsigned int>(bytes[offset + 3]) << 24);
}

static void writeU16(std::vector<unsigned char>& bytes, size_t offset, unsigned short value)
{
    bytes[offset] = static_cast<unsigned char>(value & 0xFF);
    bytes[offset + 1] = static_cast<unsigned char>((value >> 8) & 0xFF);
}



static bool makeDistantRadioCopy(const std::string& sourcePath, const std::string& outputPath)
{
    std::ifstream input(sourcePath, std::ios::binary);
    if (!input)
        return false;

    std::vector<unsigned char> bytes(
        (std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

    if (bytes.size() < 44 || std::memcmp(bytes.data(), "RIFF", 4) != 0 ||
        std::memcmp(bytes.data() + 8, "WAVE", 4) != 0)
        return false;

    size_t formatOffset = 0;
    size_t formatSize = 0;
    size_t audioOffset = 0;
    size_t audioSize = 0;

    for (size_t offset = 12; offset + 8 <= bytes.size();)
    {
        unsigned int chunkSize = readU32(bytes, offset + 4);
        size_t chunkData = offset + 8;
        if (chunkData + chunkSize > bytes.size())
            return false;

        if (std::memcmp(bytes.data() + offset, "fmt ", 4) == 0)
        {
            formatOffset = chunkData;
            formatSize = chunkSize;
        }
        else if (std::memcmp(bytes.data() + offset, "data", 4) == 0)
        {
            audioOffset = chunkData;
            audioSize = chunkSize;
            break;
        }

        offset = chunkData + chunkSize + (chunkSize & 1U);
    }

    if (formatOffset == 0 || formatSize < 16 || audioOffset == 0 || audioSize == 0)
        return false;

    const unsigned short audioFormat = readU16(bytes, formatOffset);
    const unsigned short channels = readU16(bytes, formatOffset + 2);
    const unsigned int sampleRate = readU32(bytes, formatOffset + 4);
    const unsigned short bitsPerSample = readU16(bytes, formatOffset + 14);

    if (audioFormat != 1 || (bitsPerSample != 8 && bitsPerSample != 16) ||
        channels == 0 || channels > 2 || sampleRate == 0)
        return false;

    const size_t bytesPerSample = bitsPerSample / 8;
    const size_t frameSize = bytesPerSample * channels;
    const size_t frameCount = audioSize / frameSize;
    if (frameCount == 0)
        return false;

    const float cutoffHz = 2100.0f;
    const float alpha = 1.0f - std::exp(-2.0f * 3.14159265f * cutoffHz / sampleRate);
    const size_t delayOne = std::max<size_t>(1, static_cast<size_t>(sampleRate * 0.028f));
    const size_t delayTwo = std::max<size_t>(delayOne + 1, static_cast<size_t>(sampleRate * 0.058f));
    const size_t ringFrames = delayTwo + 1;

    std::vector<float> lowPass(channels, 0.0f);
    std::vector<float> history(ringFrames * channels, 0.0f);

    for (size_t frame = 0; frame < frameCount; ++frame)
    {
        const size_t currentSlot = frame % ringFrames;
        const size_t oldOneSlot = (frame + ringFrames - delayOne) % ringFrames;
        const size_t oldTwoSlot = (frame + ringFrames - delayTwo) % ringFrames;

        for (unsigned short channel = 0; channel < channels; ++channel)
        {
            const size_t sampleOffset = audioOffset + frame * frameSize + channel * bytesPerSample;
            float sample = 0.0f;

            if (bitsPerSample == 16)
            {
                short signedSample = static_cast<short>(readU16(bytes, sampleOffset));
                sample = static_cast<float>(signedSample) / 32768.0f;
            }
            else
            {
                sample = (static_cast<float>(bytes[sampleOffset]) - 128.0f) / 128.0f;
            }

            lowPass[channel] += alpha * (sample - lowPass[channel]);
            const float echoOne = history[oldOneSlot * channels + channel];
            const float echoTwo = history[oldTwoSlot * channels + channel];
            history[currentSlot * channels + channel] = lowPass[channel];


            const float distantSample = (0.80f * lowPass[channel] +
                                         0.13f * echoOne + 0.07f * echoTwo) * 0.32f;
            const float clipped = std::max(-1.0f, std::min(1.0f, distantSample));

            if (bitsPerSample == 16)
            {
                short outputSample = static_cast<short>(clipped * 32767.0f);
                writeU16(bytes, sampleOffset, static_cast<unsigned short>(outputSample));
            }
            else
            {
                int outputSample = static_cast<int>(clipped * 127.0f + 128.0f);
                bytes[sampleOffset] = static_cast<unsigned char>(std::max(0, std::min(255, outputSample)));
            }
        }
    }

    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output)
        return false;
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return output.good();
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


    const float CARD_Y = 0.03f;
    const vec3 deckPosition(4.0f, CARD_Y, 2.75f);

    std::vector<Card> deck;
    for (int i = 0; i < 5; i++)
        deck.push_back(Card(i, cardTextures[i], deckPosition, 0.01f * i));

    std::vector<int> remainingCards = {0, 1, 2, 3, 4};
    int playedCount = 0;


    std::vector<Card> opponentCards;
    for (int i = 0; i < 5; i++)
    {
        vec3 opponentPosition(-2.6f + 1.3f * i, CARD_Y, -1.25f);
        opponentCards.push_back(Card(100 + i, backTextures[0], opponentPosition, 0.0f));
    }


    std::vector<Card> sideCards;
    for (int i = 0; i < 3; i++)
    {
        float z = -2.0f + 2.0f * i;
        sideCards.push_back(Card(200 + i, backTextures[0], vec3(-5.15f, CARD_Y, z), 0.0f));
        sideCards.push_back(Card(210 + i, backTextures[0], vec3( 5.15f, CARD_Y, z), 0.0f));
    }


    const std::string radioPath = findAssetPath("audio/radio.wav");

    const std::string radioDistantPath =
        (std::filesystem::path(radioPath).parent_path() / "radio_distant.wav").string();
    const std::string cardSoundPath = findAssetPath("audio/card_slide_better.wav");

    bool radioMciOpen = false;
    bool radioMciPlaying = false;
    bool distantRadioCreated = false;

    std::error_code fileError;
    if (std::filesystem::exists(radioPath, fileError))
    {

        distantRadioCreated = makeDistantRadioCopy(radioPath, radioDistantPath);
        const std::string playbackPath = distantRadioCreated ? radioDistantPath : radioPath;

        if (!distantRadioCreated)
            std::cerr << "Could not make a distant copy (expected PCM 8/16-bit WAV). Playing original WAV." << std::endl;

        const std::string openRadioCommand =
            "open \"" + playbackPath + "\" type waveaudio alias radioTrack wait";
        MCIERROR radioError = mciSendStringA(openRadioCommand.c_str(), NULL, 0, NULL);

        if (radioError == 0)
        {
            radioMciOpen = true;
            std::cout << "Radio ready: " << playbackPath << std::endl;
            std::cout << "Press R to play/stop the distant radio." << std::endl;
        }
        else
        {
            printMciError(radioError, "open");
            std::cerr << "Could not open radio.wav. Check that it is a normal WAV audio file." << std::endl;
        }
    }
    else
    {
        std::cerr << "Radio file not found: " << radioPath << std::endl;
        std::cerr << "Keep radio.wav in GLProject/audio." << std::endl;
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


        if (rIsPressed && !rWasPressed)
        {
            if (!radioMciOpen)
            {
                std::cerr << "Radio is not ready. Check GLProject/audio/radio.wav." << std::endl;
            }
            else if (radioMciPlaying)
            {
                MCIERROR stopError = mciSendStringA("stop radioTrack", NULL, 0, NULL);
                printMciError(stopError, "stop");
                mciSendStringA("seek radioTrack to start", NULL, 0, NULL);
                radioMciPlaying = false;
            }
            else
            {

                mciSendStringA("seek radioTrack to start", NULL, 0, NULL);
                MCIERROR playError = mciSendStringA("play radioTrack", NULL, 0, NULL);
                if (playError == 0)
                    radioMciPlaying = true;
                else
                {
                    radioMciPlaying = false;
                    printMciError(playError, "play");
                }
            }
        }
        rWasPressed = rIsPressed;


        if (radioMciPlaying)
        {
            char radioMode[32] = {};
            MCIERROR statusError = mciSendStringA(
                "status radioTrack mode", radioMode, sizeof(radioMode), NULL);

            if (statusError == 0 && std::strcmp(radioMode, "stopped") == 0)
            {
                mciSendStringA("seek radioTrack to start", NULL, 0, NULL);
                MCIERROR replayError = mciSendStringA("play radioTrack", NULL, 0, NULL);
                if (replayError != 0)
                {
                    radioMciPlaying = false;
                    printMciError(replayError, "replay");
                }
            }
        }



        if (gIsPressed && !gWasPressed && !remainingCards.empty())
        {
            int randomIndex = std::rand() % static_cast<int>(remainingCards.size());
            int cardId = remainingCards[randomIndex];
            remainingCards.erase(remainingCards.begin() + randomIndex);

            deck[cardId].moveTo(vec3(-2.6f + 1.3f * playedCount, CARD_Y, 2.75f));


            if (!PlaySoundA(cardSoundPath.c_str(), NULL,
                            SND_FILENAME | SND_ASYNC | SND_NODEFAULT))
            {
                std::cerr << "Could not play card sound: " << cardSoundPath << std::endl;
            }
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

            view = lookAt(vec3(0.0f, 11.0f, 0.0f),
                          vec3(0.0f, 0.0f, 0.0f),
                          vec3(0.0f, 0.0f, -1.0f));
        }
        else
        {

            view = lookAt(vec3(0.0f, 6.0f, 11.0f),
                          vec3(0.0f, 0.5f, -1.0f),
                          vec3(0.0f, 1.0f, 0.0f));
        }
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


        for (Card &card : opponentCards)
        {
            if (cameraMode == 2)
                card.stand();
            else
                card.lay();

            card.update(deltaTime);
            card.draw(sceneShader);
        }



        for (Card &card : sideCards)
        {
            card.lay();
            card.update(deltaTime);
            card.draw(sceneShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    if (radioMciOpen)
    {
        mciSendStringA("stop radioTrack", NULL, 0, NULL);
        mciSendStringA("close radioTrack", NULL, 0, NULL);
    }
    if (distantRadioCreated)
        std::filesystem::remove(radioDistantPath, fileError);
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
