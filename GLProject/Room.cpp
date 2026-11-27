#include "Room.h"
#include "tools/TextureLoader.h"
#include "tools/Box.h"
#include <learnopengl/model.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace glm;




const float ROOM = 15.0f;
const float FLOOR_Y = -3.0f;
const float CEIL_Y = 9.0f;




static std::vector<GLuint> repeatTextures;   
const int TEX_FLOOR = 0, TEX_WALL = 1, TEX_FELT = 2, TEX_WOOD = 3, TEX_WHITE = 4;

static std::vector<GLuint> clampTextures;    
const int TEX_POSTER = 0, TEX_LEAF = 1, TEX_CHESSBOARD = 2, TEX_CHEST = 3;




static std::vector<PolygonWithTexture> flatParts;     
static std::vector<PolygonWithTexture> ceilingParts;  
static std::vector<Box> boxParts;                     
static std::vector<Box> glowParts;                    
static std::vector<Model> models;                     
static mat4 modelMatrix(1.0f);

const vec3 BRASS(0.55f, 0.45f, 0.30f);




static std::vector<vec2> fullTexCoords()
{
    return { vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f), vec2(0.0f, 1.0f) };
}


static void addFlat(std::vector<vec3> corners, std::vector<vec2> texCoords, GLuint texture, vec3 color)
{
    PolygonWithTexture polygon(corners, texCoords, color);
    polygon.setTexture(texture);
    flatParts.push_back(polygon);
}


static void addBox(vec3 center, vec3 size, GLuint texture, vec3 color = vec3(1.0f))
{
    boxParts.push_back(Box(center, size, texture, color));
}


static void addGlowBox(vec3 center, vec3 size, vec3 color)
{
    glowParts.push_back(Box(center, size, repeatTextures[TEX_WHITE], color));
}






static void addWall(vec3 bottomLeft, vec3 bottomRight)
{
    vec3 up(0.0f, CEIL_Y - FLOOR_Y, 0.0f);
    std::vector<vec3> corners = { bottomLeft + up, bottomLeft, bottomRight, bottomRight + up };
    std::vector<vec2> texCoords = { vec2(0.0f, 2.0f), vec2(0.0f, 0.0f), vec2(5.0f, 0.0f), vec2(5.0f, 2.0f) };
    addFlat(corners, texCoords, repeatTextures[TEX_WALL], vec3(1.0f));
}

static void buildFloorAndWalls()
{
    
    std::vector<vec3> floorCorners = {
        vec3(-ROOM, FLOOR_Y, -ROOM), vec3(-ROOM, FLOOR_Y, ROOM),
        vec3(ROOM, FLOOR_Y, ROOM), vec3(ROOM, FLOOR_Y, -ROOM) };
    std::vector<vec2> floorTexCoords = { vec2(0.0f, 10.0f), vec2(0.0f, 0.0f), vec2(10.0f, 0.0f), vec2(10.0f, 10.0f) };
    addFlat(floorCorners, floorTexCoords, repeatTextures[TEX_FLOOR], vec3(1.0f));

    
    addWall(vec3(-ROOM, FLOOR_Y, -ROOM), vec3(ROOM, FLOOR_Y, -ROOM));   
    addWall(vec3(ROOM, FLOOR_Y, ROOM), vec3(-ROOM, FLOOR_Y, ROOM));     
    addWall(vec3(-ROOM, FLOOR_Y, ROOM), vec3(-ROOM, FLOOR_Y, -ROOM));   
    addWall(vec3(ROOM, FLOOR_Y, -ROOM), vec3(ROOM, FLOOR_Y, ROOM));     

    
    std::vector<vec3> ceilingCorners = {
        vec3(-ROOM, CEIL_Y, -ROOM), vec3(ROOM, CEIL_Y, -ROOM),
        vec3(ROOM, CEIL_Y, ROOM), vec3(-ROOM, CEIL_Y, ROOM) };
    std::vector<vec2> ceilingTexCoords = { vec2(0.0f, 5.0f), vec2(5.0f, 5.0f), vec2(5.0f, 0.0f), vec2(0.0f, 0.0f) };
    PolygonWithTexture ceiling(ceilingCorners, ceilingTexCoords);
    ceiling.setTexture(repeatTextures[TEX_WALL]);
    ceilingParts.push_back(ceiling);
}




static void buildTable()
{
    GLuint felt = repeatTextures[TEX_FELT];
    GLuint wood = repeatTextures[TEX_WOOD];

    
    addBox(vec3(0.0f, -0.15f, 0.0f), vec3(12.0f, 0.3f, 8.0f), felt);

    
    addBox(vec3(0.0f, -0.12f, 4.2f), vec3(12.8f, 0.36f, 0.4f), wood);
    addBox(vec3(0.0f, -0.12f, -4.2f), vec3(12.8f, 0.36f, 0.4f), wood);
    addBox(vec3(6.2f, -0.12f, 0.0f), vec3(0.4f, 0.36f, 8.0f), wood);
    addBox(vec3(-6.2f, -0.12f, 0.0f), vec3(0.4f, 0.36f, 8.0f), wood);

    
    addBox(vec3(0.0f, -0.6f, 4.0f), vec3(11.5f, 0.6f, 0.25f), wood);
    addBox(vec3(0.0f, -0.6f, -4.0f), vec3(11.5f, 0.6f, 0.25f), wood);
    addBox(vec3(6.0f, -0.6f, 0.0f), vec3(0.25f, 0.6f, 7.5f), wood);
    addBox(vec3(-6.0f, -0.6f, 0.0f), vec3(0.25f, 0.6f, 7.5f), wood);

    
    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sz = -1; sz <= 1; sz += 2)
        {
            float x = sx * 6.0f;
            float z = sz * 4.0f;
            addBox(vec3(x, -1.65f, z), vec3(0.5f, 2.7f, 0.5f), wood);
            addBox(vec3(x, -0.85f, z), vec3(0.7f, 0.3f, 0.7f), wood);
            addBox(vec3(x, -2.85f, z), vec3(0.7f, 0.3f, 0.7f), wood);
        }
    }
}




const float LINE_Y = 0.005f;   


static void addBar(float x0, float x1, float z0, float z1, float y, vec3 color)
{
    std::vector<vec3> corners = { vec3(x0, y, z1), vec3(x1, y, z1), vec3(x1, y, z0), vec3(x0, y, z0) };
    addFlat(corners, fullTexCoords(), repeatTextures[TEX_WHITE], color);
}


static void addOutline(float xMin, float xMax, float zMin, float zMax, float thickness, vec3 color)
{
    addBar(xMin, xMax, zMax - thickness, zMax, LINE_Y, color);
    addBar(xMin, xMax, zMin, zMin + thickness, LINE_Y, color);
    addBar(xMin, xMin + thickness, zMin + thickness, zMax - thickness, LINE_Y, color);
    addBar(xMax - thickness, xMax, zMin + thickness, zMax - thickness, LINE_Y, color);
}


static void addZone(float xMin, float xMax, float zMin, float zMax, vec3 color)
{
    addOutline(xMin, xMax, zMin, zMax, 0.04f, color);
    addOutline(xMin + 0.08f, xMax - 0.08f, zMin + 0.08f, zMax - 0.08f, 0.015f, color);

    float c = 0.16f;
    float y = LINE_Y + 0.0015f;
    addBar(xMin, xMin + c, zMin, zMin + c, y, color);
    addBar(xMax - c, xMax, zMin, zMin + c, y, color);
    addBar(xMin, xMin + c, zMax - c, zMax, y, color);
    addBar(xMax - c, xMax, zMax - c, zMax, y, color);
}


static void addSlots(float centerZ, vec3 color)
{
    for (int i = 0; i < 5; i++)
    {
        float cx = -2.6f + 1.3f * i;
        addOutline(cx - 0.55f, cx + 0.55f, centerZ - 0.8f, centerZ + 0.8f, 0.02f, color);
    }
}

static void buildZones()
{
    vec3 gold(0.85f, 0.70f, 0.25f);
    vec3 dimGold(0.5f, 0.4f, 0.18f);
    vec3 slotColor(0.6f, 0.5f, 0.22f);

    addZone(-3.3f, 3.3f, 1.0f, 3.0f, gold);        
    addSlots(2.0f, slotColor);

    addZone(-3.3f, 3.3f, -0.85f, 0.85f, gold);     
    addSlots(0.0f, slotColor);

    addZone(-3.3f, 3.3f, -3.0f, -1.0f, dimGold);   
}





static void buildPoster()
{
    float bottom = -1.0f;
    float top = 4.8f;
    float halfWidth = (top - bottom) / 3.0f;   
    float z = -ROOM + 0.02f;   

    std::vector<vec3> corners = {
        vec3(-halfWidth, top, z), vec3(-halfWidth, bottom, z),
        vec3(halfWidth, bottom, z), vec3(halfWidth, top, z) };
    std::vector<vec2> texCoords = { vec2(0.0f, 1.0f), vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f) };
    addFlat(corners, texCoords, clampTextures[TEX_POSTER], vec3(1.0f));
}




static void buildChessTable()
{
    GLuint wood = repeatTextures[TEX_WOOD];
    float cx = -10.0f;
    float cz = -10.0f;
    float legHeight = 1.5f;
    float topY = FLOOR_Y + legHeight + 0.1f;

    
    addBox(vec3(cx, topY, cz), vec3(3.4f, 0.2f, 3.4f), wood);

    
    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sz = -1; sz <= 1; sz += 2)
            addBox(vec3(cx + sx * 1.5f, FLOOR_Y + legHeight / 2.0f, cz + sz * 1.5f), vec3(0.25f, legHeight, 0.25f), wood);
    }

    
    addBox(vec3(cx, FLOOR_Y + 0.45f, cz + 2.6f), vec3(1.0f, 0.9f, 1.0f), wood);
    addBox(vec3(cx, FLOOR_Y + 0.45f, cz - 2.6f), vec3(1.0f, 0.9f, 1.0f), wood);

    
    float y = topY + 0.1f + 0.005f;
    float b = 1.5f;
    std::vector<vec3> corners = {
        vec3(cx - b, y, cz + b), vec3(cx + b, y, cz + b),
        vec3(cx + b, y, cz - b), vec3(cx - b, y, cz - b) };
    addFlat(corners, fullTexCoords(), clampTextures[TEX_CHESSBOARD], vec3(1.0f));
}




static void buildChest()
{
    GLuint chest = clampTextures[TEX_CHEST];
    addBox(vec3(10.0f, FLOOR_Y + 0.5f, -10.0f), vec3(2.4f, 1.0f, 1.6f), chest);
    addBox(vec3(10.0f, FLOOR_Y + 1.25f, -10.0f), vec3(2.5f, 0.5f, 1.7f), chest);
}




static void buildModel()
{
    models.push_back(Model("models/alrihla.glb"));
    if (models[0].meshes.empty())
    {
        std::cerr << "Room: 3D model failed to load" << std::endl;
        models.clear();
        return;
    }

    
    vec3 minCorner(1000000.0f);
    vec3 maxCorner(-1000000.0f);
    for (const Mesh& mesh : models[0].meshes)
    {
        for (const Vertex& vertex : mesh.vertices)
        {
            minCorner = min(minCorner, vertex.Position);
            maxCorner = max(maxCorner, vertex.Position);
        }
    }

    vec3 size = maxCorner - minCorner;
    float biggest = glm::max(size.x, glm::max(size.y, size.z));
    float scaleFactor = 3.0f / biggest;
    vec3 bottomCenter((minCorner.x + maxCorner.x) / 2.0f, minCorner.y, (minCorner.z + maxCorner.z) / 2.0f);

    
    modelMatrix = translate(mat4(1.0f), vec3(-12.0f, FLOOR_Y, 8.0f));
    modelMatrix = scale(modelMatrix, vec3(scaleFactor));
    modelMatrix = translate(modelMatrix, -bottomCenter);
}





static void addChair(float x, float side)
{
    GLuint wood = repeatTextures[TEX_WOOD];
    float seatY = FLOOR_Y + 1.8f;   

    
    addBox(vec3(x, seatY, 0.0f), vec3(1.4f, 0.2f, 1.4f), wood);

    
    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sz = -1; sz <= 1; sz += 2)
            addBox(vec3(x + sx * 0.55f, FLOOR_Y + 0.85f, sz * 0.55f), vec3(0.15f, 1.7f, 0.15f), wood);
    }

    
    addBox(vec3(x + side * 0.65f, seatY + 1.0f, 0.0f), vec3(0.15f, 1.8f, 1.4f), wood);
}

static void buildChairs()
{
    addChair(-7.7f, -1.0f);
    addChair(7.7f, 1.0f);
}





static void addHangingLantern(float x)
{
    GLuint white = repeatTextures[TEX_WHITE];

    addBox(vec3(x, 6.60f, 0.0f), vec3(0.06f, 4.80f, 0.06f), white, BRASS);   
    addBox(vec3(x, 4.15f, 0.0f), vec3(0.86f, 0.18f, 0.86f), white, BRASS);   
    addBox(vec3(x, 3.05f, 0.0f), vec3(0.80f, 0.14f, 0.80f), white, BRASS);   

    
    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sz = -1; sz <= 1; sz += 2)
            addBox(vec3(x + sx * 0.30f, 3.60f, sz * 0.30f), vec3(0.09f, 1.00f, 0.09f), white, BRASS);
    }

    
    addGlowBox(vec3(x, 3.60f, 0.0f), vec3(0.60f, 0.90f, 0.60f), vec3(1.0f, 0.80f, 0.48f));
}

static void addWallLantern(float x)
{
    GLuint white = repeatTextures[TEX_WHITE];
    float z = -ROOM + 0.55f;

    addBox(vec3(x, 2.30f, -ROOM + 0.20f), vec3(0.18f, 0.22f, 0.60f), white, BRASS);   
    addBox(vec3(x, 2.82f, z), vec3(0.88f, 0.16f, 0.88f), white, BRASS);               
    addBox(vec3(x, 1.90f, z), vec3(0.80f, 0.14f, 0.80f), white, BRASS);               

    
    for (int px = -1; px <= 1; px += 2)
    {
        for (int pz = -1; pz <= 1; pz += 2)
            addBox(vec3(x + px * 0.33f, 2.35f, z + pz * 0.33f), vec3(0.08f, 0.80f, 0.08f), white, BRASS);
    }

    
    addGlowBox(vec3(x, 2.35f, z), vec3(0.60f, 0.72f, 0.60f), vec3(1.0f, 0.86f, 0.58f));
}

static void buildLanterns()
{
    addHangingLantern(-4.0f);
    addHangingLantern(4.0f);
    addWallLantern(-5.6f);
    addWallLantern(5.6f);
}




static void addLeaf(vec3 crown, float angleDeg, float tiltDeg, float length, float halfWidth)
{
    float a = radians(angleDeg);
    float t = radians(tiltDeg);

    
    vec3 direction(std::cos(a) * std::cos(t), std::sin(t), std::sin(a) * std::cos(t));
    vec3 side = normalize(cross(direction, vec3(0.0f, 1.0f, 0.0f)));

    vec3 base = crown + direction * 0.06f;
    vec3 tip = base + direction * length;
    tip.y -= length * 0.25f;   

    std::vector<vec3> corners = {
        base - side * halfWidth, base + side * halfWidth,
        tip + side * halfWidth * 0.8f, tip - side * halfWidth * 0.8f };
    std::vector<vec2> texCoords = { vec2(0.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f), vec2(1.0f, 0.0f) };
    addFlat(corners, texCoords, clampTextures[TEX_LEAF], vec3(1.35f, 1.55f, 1.25f));
}

static void buildPlant()
{
    GLuint white = repeatTextures[TEX_WHITE];
    GLuint wood = repeatTextures[TEX_WOOD];
    float px = -11.5f;
    float pz = -6.6f;

    
    addBox(vec3(px, FLOOR_Y + 0.06f, pz), vec3(0.62f, 0.12f, 0.62f), white, BRASS);
    addBox(vec3(px, FLOOR_Y + 0.40f, pz), vec3(0.80f, 0.55f, 0.80f), white, BRASS);
    addBox(vec3(px, FLOOR_Y + 0.73f, pz), vec3(0.92f, 0.12f, 0.92f), white, BRASS);
    addBox(vec3(px, FLOOR_Y + 0.79f, pz), vec3(0.78f, 0.06f, 0.78f), white, vec3(0.24f, 0.18f, 0.12f));   

    
    addBox(vec3(px, FLOOR_Y + 1.14f, pz), vec3(0.18f, 0.70f, 0.18f), wood);

    
    vec3 crown(px, FLOOR_Y + 1.50f, pz);
    for (int i = 0; i < 12; i++)
        addLeaf(crown, 30.0f * i, 18.0f + (i % 3) * 14.0f, 1.80f + (i % 4) * 0.16f, 0.46f);
    for (int i = 0; i < 5; i++)
        addLeaf(crown, 72.0f * i, 58.0f, 1.30f, 0.36f);
}





const float RADIO_SHIFT = 7.5f;     
const float RADIO_SHIFT_Z = -6.3f;  

static void addRadioPart(vec3 center, vec3 size, GLuint texture, vec3 color)
{
    center.x += RADIO_SHIFT;
    center.z += RADIO_SHIFT_Z;
    addBox(center, size, texture, color);
}

static void addRadioGlow(vec3 center, vec3 size, vec3 color)
{
    center.x += RADIO_SHIFT;
    center.z += RADIO_SHIFT_Z;
    addGlowBox(center, size, color);
}

static void buildRadio()
{
    GLuint white = repeatTextures[TEX_WHITE];
    GLuint wood = repeatTextures[TEX_WOOD];
    vec3 radioWood(0.62f, 0.46f, 0.27f);
    vec3 radioDark(0.12f, 0.10f, 0.075f);
    vec3 radioBrass(0.72f, 0.50f, 0.22f);

    
    addRadioPart(vec3(5.00f, -1.43f, -7.50f), vec3(3.50f, 0.18f, 1.90f), wood, radioWood);
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            float x = (i == 0) ? 3.45f : 6.55f;
            float z = (j == 0) ? -8.22f : -6.78f;
            addRadioPart(vec3(x, -2.18f, z), vec3(0.14f, 1.55f, 0.14f), wood, radioWood);
        }
    }

    
    addRadioPart(vec3(4.25f, -0.78f, -7.53f), vec3(1.70f, 1.10f, 0.90f), wood, radioWood);
    addRadioPart(vec3(4.25f, -0.78f, -7.065f), vec3(1.54f, 0.94f, 0.045f), white, radioDark);

    
    for (int i = 0; i < 6; i++)
        addRadioPart(vec3(3.88f, -1.10f + 0.12f * i, -7.035f), vec3(0.40f, 0.022f, 0.025f), white, radioBrass);

    
    addRadioPart(vec3(4.62f, -0.72f, -7.035f), vec3(0.42f, 0.34f, 0.025f), white, radioDark);
    addRadioGlow(vec3(4.62f, -0.72f, -7.015f), vec3(0.34f, 0.25f, 0.018f), vec3(0.38f, 0.18f, 0.045f));
    addRadioPart(vec3(4.90f, -1.10f, -7.025f), vec3(0.12f, 0.12f, 0.07f), white, radioBrass);
    addRadioPart(vec3(4.90f, -0.42f, -7.025f), vec3(0.12f, 0.12f, 0.07f), white, radioBrass);

    
    addRadioPart(vec3(4.82f, -0.03f, -7.80f), vec3(0.025f, 0.62f, 0.025f), white, radioBrass);

    
    addRadioPart(vec3(3.15f, -1.27f, -6.78f), vec3(0.34f, 0.07f, 0.34f), wood, radioWood);
    addRadioPart(vec3(3.15f, -1.03f, -6.78f), vec3(0.055f, 0.42f, 0.055f), white, radioBrass);
    addRadioPart(vec3(3.15f, -0.78f, -6.78f), vec3(0.28f, 0.25f, 0.28f), white, radioBrass);
    addRadioGlow(vec3(3.15f, -0.78f, -6.74f), vec3(0.18f, 0.15f, 0.18f), vec3(0.32f, 0.14f, 0.035f));

    
    float fanZ = -7.03f;
    addRadioPart(vec3(6.05f, -1.24f, -7.50f), vec3(0.66f, 0.14f, 0.62f), wood, radioWood);
    addRadioPart(vec3(6.05f, -0.87f, -7.50f), vec3(0.12f, 0.62f, 0.12f), white, radioBrass);
    addRadioPart(vec3(6.05f, -0.43f, fanZ), vec3(0.82f, 0.075f, 0.07f), white, radioBrass);
    addRadioPart(vec3(6.05f, -1.15f, fanZ), vec3(0.82f, 0.075f, 0.07f), white, radioBrass);
    addRadioPart(vec3(5.69f, -0.79f, fanZ), vec3(0.075f, 0.72f, 0.07f), white, radioBrass);
    addRadioPart(vec3(6.41f, -0.79f, fanZ), vec3(0.075f, 0.72f, 0.07f), white, radioBrass);
    addRadioPart(vec3(6.05f, -0.79f, fanZ + 0.085f), vec3(0.15f, 0.15f, 0.07f), white, radioBrass);
}




void buildRoom()
{
    std::vector<std::string> repeatPaths = {
        "textures/floor.jpg", "textures/wall.jpg", "textures/table_felt.jpg",
        "textures/table_wood.jpg", "textures/white.jpg" };
    repeatTextures = loadTextures(repeatPaths, GL_REPEAT, GL_LINEAR);

    std::vector<std::string> clampPaths = {
        "textures/poster.png", "textures/leaf.png",
        "textures/decor_chessboard.png", "textures/decor_chest.png" };
    clampTextures = loadTextures(clampPaths, GL_CLAMP_TO_EDGE, GL_LINEAR);

    if (repeatTextures.size() != repeatPaths.size() || clampTextures.size() != clampPaths.size())
    {
        std::cerr << "Room: some textures failed to load" << std::endl;
        return;
    }

    buildFloorAndWalls();
    buildTable();
    buildZones();
    buildPoster();
    buildChessTable();
    buildChest();
    buildModel();
    buildChairs();
    buildLanterns();
    buildPlant();
    buildRadio();
}

void drawRoom(Shader& shader)
{
    shader.setInt("textureSample", 0);
    glActiveTexture(GL_TEXTURE0);

    for (PolygonWithTexture& part : flatParts)
        part.draw(shader);

    for (Box& part : boxParts)
        part.draw(shader);

    
    if (!models.empty())
    {
        shader.setVec3("objectColor", vec3(1.0f));
        shader.setMat4("model", modelMatrix);
        models[0].Draw(shader);
        glActiveTexture(GL_TEXTURE0);
    }

    
    shader.setBool("glow", true);
    for (Box& part : glowParts)
        part.draw(shader);
    shader.setBool("glow", false);

    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    for (PolygonWithTexture& part : ceilingParts)
        part.draw(shader);
    glDisable(GL_CULL_FACE);
}

void destroyRoom()
{
    for (PolygonWithTexture& part : flatParts)
        part.deleteBuffers();
    flatParts.clear();

    for (PolygonWithTexture& part : ceilingParts)
        part.deleteBuffers();
    ceilingParts.clear();

    for (Box& part : boxParts)
        part.destroy();
    boxParts.clear();

    for (Box& part : glowParts)
        part.destroy();
    glowParts.clear();

    models.clear();

    glDeleteTextures(static_cast<GLsizei>(repeatTextures.size()), repeatTextures.data());
    glDeleteTextures(static_cast<GLsizei>(clampTextures.size()), clampTextures.data());
}
