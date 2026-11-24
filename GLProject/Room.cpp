#include "Room.h"
#include "tools/TextureLoader.h"
#include "tools/Box.h"
#include <learnopengl/model.h>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

// Room data lives here (file-level), so main only calls buildRoom/drawRoom
static std::vector<GLuint> roomTextures;            // [0] = floor, [1] = wallpaper
static std::vector<PolygonWithTexture*> roomParts;  // floor, 4 walls, ceiling

// The table: one top slab + 4 legs (all built from Box), with its own textures
static std::vector<GLuint> tableTextures;           // [0] = green felt (top), [1] = dark wood (legs)
static std::vector<Box*> tableParts;                // top, then 4 legs

// The 3 zone frames drawn on the felt (deck / played / opponent), made of thin flat quads
static std::vector<GLuint> zoneTexture;             // [0] = plain white (tinted by each frame's color)
static std::vector<PolygonWithTexture*> zoneParts;

// The torn poster on the back wall (a PNG with real transparency, see scripts/make_poster.ps1)
static std::vector<GLuint> posterTexture;
static PolygonWithTexture* posterPart = nullptr;
static PolygonWithTexture* posterShadow = nullptr;   // same torn shape, dark and shifted: fake drop shadow

// Decor pieces (built from boxes). Piece 1 = Damascene chess table at (-10, -3, -10)
static std::vector<GLuint> decorTextures;            // [0] = chessboard picture
static std::vector<Box*> decorParts;                 // table top, legs, stools
static PolygonWithTexture* chessBoard = nullptr;     // the board picture, lying on the table top
static std::vector<GLuint> chestTexture;             // decor piece 2: treasure chest picture (the box faces show it)
static Model* decorModel = nullptr;                  // decor piece 3: the ONE 3D model (loaded with Assimp)
static glm::mat4 decorModelMatrix(1.0f);             // places it at (-12, -3, 8) with a sensible size

// Carpet + chairs + hanging lanterns (the "make it look professional" pass)
static std::vector<GLuint> rugTexture;               // [0] = the carpet picture
static PolygonWithTexture* rugPart = nullptr;        // the carpet, a flat quad on the floor
static std::vector<Box*> chairParts;                 // 6 chairs, every piece is a Box
static std::vector<Box*> radioParts;                  // radio, shelf and small desk fan
static std::vector<Box*> lanternParts;               // lantern metal: chain, cap, posts, base
static std::vector<Box*> glowParts;                  // lantern glass: drawn with emissive = 1

// Potted plant in the back-left corner: brass pot + trunk are Box, the fronds are flat quads
static std::vector<GLuint> leafTexture;              // [0] = palm frond with alpha
static std::vector<Box*> plantParts;
static std::vector<PolygonWithTexture*> plantLeaves;

// Room limits (same numbers as the contract): x,z in [-15, 15], floor y = -3, ceiling y = 9
static const float ROOM = 15.0f;
static const float FLOOR_Y = -3.0f;
static const float CEIL_Y = 9.0f;

// Helper: builds one textured quad from 4 corners and 4 UVs, and gives it a texture
static void addQuad(const std::vector<glm::vec3>& corners, const std::vector<glm::vec2>& uvs, GLuint& texture)
{
    PolygonWithTexture* quad = new PolygonWithTexture(corners, uvs);
    quad->setTexture(texture);
    roomParts.push_back(quad);
}

void buildRoom()
{
    // 1. Load textures with GL_REPEAT so they can tile
    std::vector<std::string> paths = { "textures/floor.jpg", "textures/wall.jpg" };
    roomTextures = loadTextures(paths, GL_REPEAT, GL_LINEAR);
    if (roomTextures.size() != paths.size())
    {
        std::cerr << "Room: expected " << paths.size() << " textures, got " << roomTextures.size() << std::endl;
        return;
    }

    // 2. Floor: UV up to 10 so the wood repeats 10 x 10 times instead of stretching
    addQuad(
        {
            glm::vec3(-ROOM, FLOOR_Y, -ROOM), glm::vec3(-ROOM, FLOOR_Y,  ROOM),
            glm::vec3( ROOM, FLOOR_Y,  ROOM), glm::vec3( ROOM, FLOOR_Y, -ROOM)
        },
        { glm::vec2(0.0f, 10.0f), glm::vec2(0.0f, 0.0f), glm::vec2(10.0f, 0.0f), glm::vec2(10.0f, 10.0f) },
        roomTextures[0]);

    // 3. Walls: 30 wide x 12 tall. One wallpaper tile is about 6 x 6 units,
    //    so U goes 0 -> 5 along the width and V goes 0 -> 2 along the height (no stretching).
    std::vector<glm::vec2> wallUV =
        { glm::vec2(0.0f, 2.0f), glm::vec2(0.0f, 0.0f), glm::vec2(5.0f, 0.0f), glm::vec2(5.0f, 2.0f) };

    // corners order: top-left, bottom-left, bottom-right, top-right (as seen from INSIDE the room)
    addQuad( // back wall (z = -15)
        { glm::vec3(-ROOM, CEIL_Y, -ROOM), glm::vec3(-ROOM, FLOOR_Y, -ROOM),
          glm::vec3( ROOM, FLOOR_Y, -ROOM), glm::vec3( ROOM, CEIL_Y, -ROOM) }, wallUV, roomTextures[1]);
    addQuad( // front wall (z = +15), behind the player
        { glm::vec3( ROOM, CEIL_Y,  ROOM), glm::vec3( ROOM, FLOOR_Y,  ROOM),
          glm::vec3(-ROOM, FLOOR_Y,  ROOM), glm::vec3(-ROOM, CEIL_Y,  ROOM) }, wallUV, roomTextures[1]);
    addQuad( // left wall (x = -15)
        { glm::vec3(-ROOM, CEIL_Y,  ROOM), glm::vec3(-ROOM, FLOOR_Y,  ROOM),
          glm::vec3(-ROOM, FLOOR_Y, -ROOM), glm::vec3(-ROOM, CEIL_Y, -ROOM) }, wallUV, roomTextures[1]);
    addQuad( // right wall (x = +15)
        { glm::vec3( ROOM, CEIL_Y, -ROOM), glm::vec3( ROOM, FLOOR_Y, -ROOM),
          glm::vec3( ROOM, FLOOR_Y,  ROOM), glm::vec3( ROOM, CEIL_Y,  ROOM) }, wallUV, roomTextures[1]);

    // 4. Ceiling at y = 9, same wallpaper, tiled 5 x 5.
    //    Corners are ordered counter-clockwise as seen from BELOW, so its front face points into the room.
    //    This lets drawRoom hide it when the top camera (y = 11) looks down from above.
    addQuad(
        {
            glm::vec3(-ROOM, CEIL_Y, -ROOM), glm::vec3( ROOM, CEIL_Y, -ROOM),
            glm::vec3( ROOM, CEIL_Y,  ROOM), glm::vec3(-ROOM, CEIL_Y,  ROOM)
        },
        { glm::vec2(0.0f, 5.0f), glm::vec2(5.0f, 5.0f), glm::vec2(5.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
        roomTextures[1]);

    // 5. The table (contract: 12 x 8, top surface at y = 0).
    //    Top slab is 0.3 thick, so its center is at y = -0.15 and its size is (12, 0.3, 8).
    std::vector<std::string> tablePaths = { "textures/table_felt.jpg", "textures/table_wood.jpg" };
    tableTextures = loadTextures(tablePaths, GL_REPEAT, GL_LINEAR);
    if (tableTextures.size() != tablePaths.size())
    {
        std::cerr << "Room: table textures failed to load" << std::endl;
        return;
    }
    // Small helper so each table piece is one short line: (center, size, texture index)
    auto addBox = [](glm::vec3 center, glm::vec3 size, int tex)
    {
        tableParts.push_back(new Box(center, size, tableTextures[tex]));
    };
    const int FELT = 0, WOOD = 1;

    // Felt: the playing surface. Its top is exactly y = 0 (the contract), size 12 x 8.
    addBox(glm::vec3(0.0f, -0.15f, 0.0f), glm::vec3(12.0f, 0.3f, 8.0f), FELT);

    // Wooden frame around the felt (4 bars, 0.4 wide). Slightly higher (+0.06) so the felt looks sunk inside it.
    addBox(glm::vec3(0.0f, -0.12f,  4.2f), glm::vec3(12.8f, 0.36f, 0.4f), WOOD);   // near the player
    addBox(glm::vec3(0.0f, -0.12f, -4.2f), glm::vec3(12.8f, 0.36f, 0.4f), WOOD);   // far side
    addBox(glm::vec3( 6.2f, -0.12f, 0.0f), glm::vec3(0.4f, 0.36f, 8.0f), WOOD);    // right
    addBox(glm::vec3(-6.2f, -0.12f, 0.0f), glm::vec3(0.4f, 0.36f, 8.0f), WOOD);    // left

    // Wooden skirt (apron) under the top, between the legs
    addBox(glm::vec3(0.0f, -0.6f,  4.0f), glm::vec3(11.5f, 0.6f, 0.25f), WOOD);
    addBox(glm::vec3(0.0f, -0.6f, -4.0f), glm::vec3(11.5f, 0.6f, 0.25f), WOOD);
    addBox(glm::vec3( 6.0f, -0.6f, 0.0f), glm::vec3(0.25f, 0.6f, 7.5f), WOOD);
    addBox(glm::vec3(-6.0f, -0.6f, 0.0f), glm::vec3(0.25f, 0.6f, 7.5f), WOOD);

    // 4 legs from under the slab (y = -0.3) down to the floor (y = -3): height 2.7, center y = -1.65.
    // Each leg also gets a wider block at the top and a wider foot at the bottom, like the reference picture.
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sz = -1; sz <= 1; sz += 2)
        {
            float x = sx * 6.0f, z = sz * 4.0f;
            addBox(glm::vec3(x, -1.65f, z),   glm::vec3(0.5f, 2.7f, 0.5f),   WOOD);  // leg
            addBox(glm::vec3(x, -0.85f, z),   glm::vec3(0.7f, 0.3f, 0.7f),   WOOD);  // top block
            addBox(glm::vec3(x, -2.85f, z),   glm::vec3(0.7f, 0.3f, 0.7f),   WOOD);  // foot
        }

    // 6. Zone decoration on the felt: a gold "inlay" look (outer line + thin inner line + corner squares),
    //    plus a faint outline for each of the 5 card slots. Everything is 0.005 above the felt (no z-fighting).
    //    A lying card is 1 wide and 1.5 deep (z from 1.25 to 2.75 for the player's row).
    zoneTexture = loadTextures({ "textures/white.jpg" }, GL_REPEAT, GL_LINEAR);
    if (zoneTexture.empty())
    {
        std::cerr << "Room: white texture failed to load" << std::endl;
        return;
    }
    const float LINE_Y = 0.005f;

    // One flat bar (a quad lying on the table) between x0..x1 and z0..z1 at height y
    auto addBar = [&](float x0, float x1, float z0, float z1, float y, glm::vec3 color)
    {
        PolygonWithTexture* bar = new PolygonWithTexture(
            { glm::vec3(x0, y, z1), glm::vec3(x1, y, z1), glm::vec3(x1, y, z0), glm::vec3(x0, y, z0) },
            { glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
            color);
        bar->setTexture(zoneTexture[0]);
        zoneParts.push_back(bar);
    };

    // A rectangle outline = 4 bars of thickness t (left/right bars stop short of the corners so none overlap)
    auto addOutline = [&](float xMin, float xMax, float zMin, float zMax, float t, glm::vec3 color)
    {
        addBar(xMin, xMax, zMax - t, zMax, LINE_Y, color);              // near the player
        addBar(xMin, xMax, zMin, zMin + t, LINE_Y, color);              // far side
        addBar(xMin, xMin + t, zMin + t, zMax - t, LINE_Y, color);      // left
        addBar(xMax - t, xMax, zMin + t, zMax - t, LINE_Y, color);      // right
    };

    // One decorated zone: thick outer line, thin inner line, and a small square on each corner
    auto addZone = [&](float xMin, float xMax, float zMin, float zMax, glm::vec3 color)
    {
        addOutline(xMin, xMax, zMin, zMax, 0.04f, color);
        addOutline(xMin + 0.08f, xMax - 0.08f, zMin + 0.08f, zMax - 0.08f, 0.015f, color);
        const float c = 0.16f, y = LINE_Y + 0.0015f;                    // corner squares sit a bit higher
        addBar(xMin, xMin + c, zMin, zMin + c, y, color);
        addBar(xMax - c, xMax, zMin, zMin + c, y, color);
        addBar(xMin, xMin + c, zMax - c, zMax, y, color);
        addBar(xMax - c, xMax, zMax - c, zMax, y, color);
    };

    // Five card slots in a row (same x as the cards: -2.6, -1.3, 0, 1.3, 2.6), slightly bigger than a card
    auto addSlots = [&](float centerZ, glm::vec3 color)
    {
        for (int i = 0; i < 5; i++)
        {
            float cx = -2.6f + 1.3f * i;
            addOutline(cx - 0.55f, cx + 0.55f, centerZ - 0.8f, centerZ + 0.8f, 0.02f, color);
        }
    };

    const glm::vec3 GOLD(0.85f, 0.70f, 0.25f);      // active zones
    const glm::vec3 DIM_GOLD(0.5f, 0.4f, 0.18f);    // opponent zone (empty for now)
    const glm::vec3 SLOT(0.6f, 0.5f, 0.22f);

    addZone(-3.3f,  3.3f,  1.0f,  3.0f, GOLD);      // played zone (center 0, +2)
    addSlots(2.0f, SLOT);
    addZone( 3.42f, 4.58f, 1.0f,  3.0f, GOLD);      // deck zone   (center +4, +2)
    addZone(-3.3f,  3.3f, -3.0f, -1.0f, DIM_GOLD);  // opponent    (center 0, -2)
    addSlots(-2.0f, DIM_GOLD * 0.8f);

    // 6b. Carpet, chairs and the two hanging lanterns (the "make the scene look real" pass).
    //     The carpet is one flat quad on the floor, everything else is a Box: no new geometry code.
    rugTexture = loadTextures({ "textures/rug.jpg" }, GL_CLAMP_TO_EDGE, GL_LINEAR);
    if (rugTexture.empty())
    {
        std::cerr << "Room: rug texture failed to load" << std::endl;
        return;
    }
    const float RUG_Y = FLOOR_Y + 0.005f;    // 5 mm above the floor, so the two never fight over depth
    rugPart = new PolygonWithTexture(
        { glm::vec3(-10.0f, RUG_Y,  6.5f), glm::vec3(10.0f, RUG_Y,  6.5f),
          glm::vec3( 10.0f, RUG_Y, -6.5f), glm::vec3(-10.0f, RUG_Y, -6.5f) },
        { glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f) });
    rugPart->setTexture(rugTexture[0]);

    // One chair = 8 boxes, always modelled facing +z (backrest on the -z side).
    // To turn a chair we rotate its OFFSETS around the chair origin; the boxes themselves stay
    // axis-aligned because we only use 0/90/180/270, so their width and depth simply swap.
    auto addChair = [&](glm::vec3 origin, float rotY, int wood, int cloth)
    {
        const float c = glm::cos(glm::radians(rotY));
        const float s = glm::sin(glm::radians(rotY));
        auto place = [&](glm::vec3 offset, glm::vec3 size, int tex)
        {
            glm::vec3 center = origin + glm::vec3(offset.x * c + offset.z * s, offset.y,
                                                  -offset.x * s + offset.z * c);
            glm::vec3 worldSize(std::fabs(c) * size.x + std::fabs(s) * size.z, size.y,
                                std::fabs(s) * size.x + std::fabs(c) * size.z);
            chairParts.push_back(new Box(center, worldSize, tableTextures[tex]));
        };
        place(glm::vec3(-0.55f, 0.75f,  0.55f), glm::vec3(0.14f, 1.50f, 0.14f), wood);   // 4 legs
        place(glm::vec3( 0.55f, 0.75f,  0.55f), glm::vec3(0.14f, 1.50f, 0.14f), wood);
        place(glm::vec3(-0.55f, 0.75f, -0.55f), glm::vec3(0.14f, 1.50f, 0.14f), wood);
        place(glm::vec3( 0.55f, 0.75f, -0.55f), glm::vec3(0.14f, 1.50f, 0.14f), wood);
        place(glm::vec3(0.0f, 1.58f,  0.0f), glm::vec3(1.35f, 0.16f, 1.35f), wood);       // seat
        place(glm::vec3(0.0f, 1.70f,  0.0f), glm::vec3(1.05f, 0.10f, 1.00f), cloth);      // cushion
        // Open, framed backrest: it looks lighter than one large solid wooden block.
        place(glm::vec3(-0.52f, 1.95f, -0.60f), glm::vec3(0.13f, 1.05f, 0.16f), wood);
        place(glm::vec3( 0.52f, 1.95f, -0.60f), glm::vec3(0.13f, 1.05f, 0.16f), wood);
        place(glm::vec3(0.0f, 1.48f, -0.60f), glm::vec3(1.15f, 0.13f, 0.17f), wood);
        place(glm::vec3(0.0f, 2.46f, -0.60f), glm::vec3(1.38f, 0.15f, 0.19f), wood);
        for (int i = -1; i <= 1; i++)
            place(glm::vec3(0.30f * i, 1.95f, -0.60f), glm::vec3(0.07f, 0.78f, 0.12f), wood);

        // Two armrests with short supports, matching the reference chair silhouette.
        for (int side = -1; side <= 1; side += 2)
        {
            place(glm::vec3(0.68f * side, 1.91f, 0.38f), glm::vec3(0.12f, 0.58f, 0.12f), wood);
            place(glm::vec3(0.68f * side, 1.91f, -0.42f), glm::vec3(0.12f, 0.58f, 0.12f), wood);
            place(glm::vec3(0.68f * side, 2.22f, -0.02f), glm::vec3(0.16f, 0.12f, 1.02f), wood);
        }
    };

    // 6 chairs: two on each long side, one on each end. rotY turns every chair towards the table.
    addChair(glm::vec3(-2.9f, FLOOR_Y,  5.7f), 180.0f, WOOD, FELT);   // near side, faces -z
    addChair(glm::vec3( 2.9f, FLOOR_Y,  5.7f), 180.0f, WOOD, FELT);
    addChair(glm::vec3(-2.9f, FLOOR_Y, -5.7f),   0.0f, WOOD, FELT);   // far side, faces +z
    addChair(glm::vec3( 2.9f, FLOOR_Y, -5.7f),   0.0f, WOOD, FELT);
    addChair(glm::vec3(-7.7f, FLOOR_Y,  0.0f),  90.0f, WOOD, FELT);   // left end, faces +x
    addChair(glm::vec3( 7.7f, FLOOR_Y,  0.0f), -90.0f, WOOD, FELT);   // right end, faces -x

    // 6c. Vintage radio and compact fan on a side shelf at the back-right of the table.
    // Keep the set close enough to the table so it remains visible from camera 2.
    auto addRadioPart = [&](glm::vec3 center, glm::vec3 size, GLuint texture, glm::vec3 tint)
    {
        radioParts.push_back(new Box(center, size, texture, tint));
    };

    const glm::vec3 RADIO_WOOD(0.62f, 0.46f, 0.27f);
    const glm::vec3 RADIO_DARK(0.12f, 0.10f, 0.075f);
    const glm::vec3 RADIO_BRASS(0.72f, 0.50f, 0.22f);

    // Side shelf (right-back corner, just behind the playing surface).
    addRadioPart(glm::vec3(5.00f, -1.43f, -7.50f),
                 glm::vec3(3.50f, 0.18f, 1.90f), tableTextures[WOOD], RADIO_WOOD);

    // Four short wooden legs.
    for (float x : {3.45f, 6.55f})
        for (float z : {-8.22f, -6.78f})
            addRadioPart(glm::vec3(x, -2.18f, z),
                         glm::vec3(0.14f, 1.55f, 0.14f), tableTextures[WOOD], RADIO_WOOD);

    // Radio case; its front faces +Z (toward camera 2).
    addRadioPart(glm::vec3(4.25f, -0.78f, -7.53f),
                 glm::vec3(1.70f, 1.10f, 0.90f), tableTextures[WOOD], RADIO_WOOD);

    // Dark front panel.
    addRadioPart(glm::vec3(4.25f, -0.78f, -7.065f),
                 glm::vec3(1.54f, 0.94f, 0.045f), zoneTexture[0], RADIO_DARK);

    // Brass speaker grille.
    for (int i = 0; i < 6; i++)
    {
        float y = -1.10f + 0.12f * i;
        addRadioPart(glm::vec3(3.88f, y, -7.035f),
                     glm::vec3(0.40f, 0.022f, 0.025f), zoneTexture[0], RADIO_BRASS);
    }

    // Dim amber tuning display.
    addRadioPart(glm::vec3(4.62f, -0.72f, -7.035f),
                 glm::vec3(0.42f, 0.34f, 0.025f), zoneTexture[0], RADIO_DARK);
    glowParts.push_back(new Box(glm::vec3(4.62f, -0.72f, -7.015f),
                                glm::vec3(0.34f, 0.25f, 0.018f), zoneTexture[0],
                                glm::vec3(0.38f, 0.18f, 0.045f)));

    // Brass tuning knobs and short antenna.
    addRadioPart(glm::vec3(4.90f, -1.10f, -7.025f),
                 glm::vec3(0.12f, 0.12f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(4.90f, -0.42f, -7.025f),
                 glm::vec3(0.12f, 0.12f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(4.82f, -0.03f, -7.80f),
                 glm::vec3(0.025f, 0.62f, 0.025f), zoneTexture[0], RADIO_BRASS);

    // Small shaded amber lamp in front-left of the radio.
    addRadioPart(glm::vec3(3.15f, -1.27f, -6.78f),
                 glm::vec3(0.34f, 0.07f, 0.34f), tableTextures[WOOD], RADIO_WOOD);
    addRadioPart(glm::vec3(3.15f, -1.03f, -6.78f),
                 glm::vec3(0.055f, 0.42f, 0.055f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(3.15f, -0.78f, -6.78f),
                 glm::vec3(0.28f, 0.25f, 0.28f), zoneTexture[0], RADIO_BRASS);
    glowParts.push_back(new Box(glm::vec3(3.15f, -0.78f, -6.74f),
                                glm::vec3(0.18f, 0.15f, 0.18f), zoneTexture[0],
                                glm::vec3(0.32f, 0.14f, 0.035f)));

    // Compact desk fan immediately beside the radio.
    addRadioPart(glm::vec3(6.05f, -1.24f, -7.50f),
                 glm::vec3(0.66f, 0.14f, 0.62f), tableTextures[WOOD], RADIO_WOOD);
    addRadioPart(glm::vec3(6.05f, -0.87f, -7.50f),
                 glm::vec3(0.12f, 0.62f, 0.12f), zoneTexture[0], RADIO_BRASS);

    const float FAN_FRONT_Z = -7.03f;
    // Fan's square brass guard.
    addRadioPart(glm::vec3(6.05f, -0.43f, FAN_FRONT_Z),
                 glm::vec3(0.82f, 0.075f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(6.05f, -1.15f, FAN_FRONT_Z),
                 glm::vec3(0.82f, 0.075f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(5.69f, -0.79f, FAN_FRONT_Z),
                 glm::vec3(0.075f, 0.72f, 0.07f), zoneTexture[0], RADIO_BRASS);
    addRadioPart(glm::vec3(6.41f, -0.79f, FAN_FRONT_Z),
                 glm::vec3(0.075f, 0.72f, 0.07f), zoneTexture[0], RADIO_BRASS);

    // Two simple crossed blades and a brass hub.
    addRadioPart(glm::vec3(6.05f, -0.79f, FAN_FRONT_Z + 0.045f),
                 glm::vec3(0.11f, 0.52f, 0.055f), zoneTexture[0], RADIO_DARK);
    addRadioPart(glm::vec3(6.05f, -0.79f, FAN_FRONT_Z + 0.050f),
                 glm::vec3(0.52f, 0.11f, 0.055f), zoneTexture[0], RADIO_DARK);
    addRadioPart(glm::vec3(6.05f, -0.79f, FAN_FRONT_Z + 0.085f),
                 glm::vec3(0.15f, 0.15f, 0.07f), zoneTexture[0], RADIO_BRASS);

    // Two lanterns hanging over the table. The glass box glows (emissive), and its position is
    // exactly one of the two warm lights hard-coded in scene.fs, so the light really comes from it.
    const glm::vec3 BRASS(0.55f, 0.45f, 0.30f);
    for (float lx : { -4.0f, 4.0f })
    {
        lanternParts.push_back(new Box(glm::vec3(lx, 6.60f, 0.0f), glm::vec3(0.06f, 4.80f, 0.06f), zoneTexture[0], BRASS)); // chain
        lanternParts.push_back(new Box(glm::vec3(lx, 4.15f, 0.0f), glm::vec3(0.86f, 0.18f, 0.86f), zoneTexture[0], BRASS)); // cap
        for (int sx = -1; sx <= 1; sx += 2)
            for (int sz = -1; sz <= 1; sz += 2)
                lanternParts.push_back(new Box(glm::vec3(lx + sx * 0.30f, 3.60f, sz * 0.30f),
                                              glm::vec3(0.09f, 1.00f, 0.09f), zoneTexture[0], BRASS));                    // 4 posts
        lanternParts.push_back(new Box(glm::vec3(lx, 3.05f, 0.0f), glm::vec3(0.80f, 0.14f, 0.80f), zoneTexture[0], BRASS)); // base
        glowParts.push_back(new Box(glm::vec3(lx, 3.60f, 0.0f), glm::vec3(0.60f, 0.90f, 0.60f), zoneTexture[0],
                                    glm::vec3(1.0f, 0.80f, 0.48f)));                                                         // glass
    }

    // Two wall sconces on the back wall, left and right of the poster: same recipe as the lanterns,
    // but bolted on with a bracket and a bit smaller.
    for (float sx : { -5.6f, 5.6f })
    {
        const float SZ = -ROOM + 0.55f;    // how far the sconce sticks out of the back wall
        lanternParts.push_back(new Box(glm::vec3(sx, 2.30f, -ROOM + 0.20f), glm::vec3(0.18f, 0.22f, 0.60f), zoneTexture[0], BRASS)); // bracket
        lanternParts.push_back(new Box(glm::vec3(sx, 2.82f, SZ), glm::vec3(0.88f, 0.16f, 0.88f), zoneTexture[0], BRASS));           // cap
        lanternParts.push_back(new Box(glm::vec3(sx, 1.90f, SZ), glm::vec3(0.80f, 0.14f, 0.80f), zoneTexture[0], BRASS));           // base
        for (int px = -1; px <= 1; px += 2)
            for (int pz = -1; pz <= 1; pz += 2)
                lanternParts.push_back(new Box(glm::vec3(sx + px * 0.33f, 2.35f, SZ + pz * 0.33f),
                                              glm::vec3(0.08f, 0.80f, 0.08f), zoneTexture[0], BRASS));                            // 4 posts
        glowParts.push_back(new Box(glm::vec3(sx, 2.35f, SZ), glm::vec3(0.60f, 0.72f, 0.60f), zoneTexture[0],
                                    glm::vec3(1.0f, 0.86f, 0.58f)));                                                             // glass
    }

    // Potted plant in the back-left corner. Pot + trunk are Boxes; every frond is one flat quad with
    // a frond picture that has real transparency, so "discard" cuts the leaf shape out of the quad.
    leafTexture = loadTextures({ "textures/leaf.png" }, GL_CLAMP_TO_EDGE, GL_LINEAR);
    if (leafTexture.empty())
    {
        std::cerr << "Room: leaf texture failed to load" << std::endl;
        return;
    }
    const float PLX = -11.5f, PLZ = -6.6f;
    plantParts.push_back(new Box(glm::vec3(PLX, FLOOR_Y + 0.06f, PLZ), glm::vec3(0.62f, 0.12f, 0.62f), zoneTexture[0], BRASS)); // pot foot
    plantParts.push_back(new Box(glm::vec3(PLX, FLOOR_Y + 0.40f, PLZ), glm::vec3(0.80f, 0.55f, 0.80f), zoneTexture[0], BRASS)); // pot body
    plantParts.push_back(new Box(glm::vec3(PLX, FLOOR_Y + 0.73f, PLZ), glm::vec3(0.92f, 0.12f, 0.92f), zoneTexture[0], BRASS)); // pot rim
    plantParts.push_back(new Box(glm::vec3(PLX, FLOOR_Y + 0.79f, PLZ), glm::vec3(0.78f, 0.06f, 0.78f), zoneTexture[0],
                                 glm::vec3(0.24f, 0.18f, 0.12f)));                                                             // soil
    plantParts.push_back(new Box(glm::vec3(PLX, FLOOR_Y + 1.14f, PLZ), glm::vec3(0.18f, 0.70f, 0.18f), tableTextures[WOOD]));    // trunk

    const glm::vec3 CROWN(PLX, FLOOR_Y + 1.50f, PLZ);
    auto addFrond = [&](float angleDeg, float tiltDeg, float len, float halfWidth)
    {
        const float a = glm::radians(angleDeg), t = glm::radians(tiltDeg);
        glm::vec3 dir(std::cos(a) * std::cos(t), std::sin(t), std::sin(a) * std::cos(t));
        glm::vec3 side = glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f));
        if (glm::length(side) < 0.01f)
            side = glm::vec3(1.0f, 0.0f, 0.0f);
        side = glm::normalize(side);
        glm::vec3 base = CROWN + dir * 0.06f;
        glm::vec3 tip = base + dir * len;
        tip.y -= len * 0.25f;            // the frond droops, so it doesn't look like a straight blade
        plantLeaves.push_back(new PolygonWithTexture(
            // the quad is the frond's bounding box; the texture's alpha decides the real leaf shape
            { base - side * halfWidth, base + side * halfWidth,
              tip + side * halfWidth * 0.80f, tip - side * halfWidth * 0.80f },
            { glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
            glm::vec3(1.35f, 1.55f, 1.25f)));
        plantLeaves.back()->setTexture(leafTexture[0]);
    };
    for (int i = 0; i < 12; i++)
        addFrond(30.0f * i, 18.0f + (i % 3) * 14.0f, 1.80f + (i % 4) * 0.16f, 0.46f);
    for (int i = 0; i < 5; i++)
        addFrond(72.0f * i, 58.0f, 1.30f, 0.36f);

    // 7. Poster on the back wall (z = -15). The picture is 1024 x 1536 (ratio 2:3), so 8 tall -> 5.333 wide.
    //    It floats 0.02 in front of the wall so the two surfaces don't fight over depth (same idea as the 0.005 on the table).
    posterTexture = loadTextures({ "textures/poster.png" }, GL_CLAMP_TO_EDGE, GL_LINEAR);
    if (posterTexture.empty())
    {
        std::cerr << "Room: poster texture failed to load" << std::endl;
        return;
    }
    const float PW = 8.0f * 2.0f / 3.0f / 2.0f;     // half width
    const float PB = -1.0f, PT = 7.0f;              // bottom and top height of the poster (lowered by 1.5)
    const float PZ = -ROOM + 0.02f;
    posterPart = new PolygonWithTexture(
        { glm::vec3(-PW, PT, PZ), glm::vec3(-PW, PB, PZ), glm::vec3(PW, PB, PZ), glm::vec3(PW, PT, PZ) },
        { glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f) });
    posterPart->setTexture(posterTexture[0]);

    // Shadow: same poster picture, tinted almost black, moved right and down, and 0.01 closer to the wall.
    // It uses the same alpha holes, so the shadow follows the torn edge too.
    posterShadow = new PolygonWithTexture(
        { glm::vec3(-PW + 0.18f, PT - 0.18f, PZ - 0.01f), glm::vec3(-PW + 0.18f, PB - 0.18f, PZ - 0.01f),
          glm::vec3( PW + 0.18f, PB - 0.18f, PZ - 0.01f), glm::vec3( PW + 0.18f, PT - 0.18f, PZ - 0.01f) },
        { glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
        glm::vec3(0.12f, 0.09f, 0.06f));
    posterShadow->setTexture(posterTexture[0]);

    // 8. Decor piece 1: Damascene chess table, centered at (-10, -3, -10) (contract position 1).
    //    Everything is a Box: scale first then translate, so each box is placed by its CENTER.
    decorTextures = loadTextures({ "textures/decor_chessboard.png" }, GL_CLAMP_TO_EDGE, GL_LINEAR);
    if (decorTextures.empty())
    {
        std::cerr << "Room: chessboard texture failed to load" << std::endl;
        return;
    }
    const float CX = -10.0f, CZ = -10.0f;
    const float LEG_H = 1.5f;                                   // legs go from the floor (-3) up to -1.5
    const float TOP_Y = FLOOR_Y + LEG_H + 0.1f;                 // center of the 0.2 thick slab = -1.4
    auto addDecor = [&](glm::vec3 center, glm::vec3 size)
    {
        decorParts.push_back(new Box(center, size, tableTextures[1]));   // dark wood, same as the game table
    };
    addDecor(glm::vec3(CX, TOP_Y, CZ), glm::vec3(3.4f, 0.2f, 3.4f));     // table top
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sz = -1; sz <= 1; sz += 2)
            addDecor(glm::vec3(CX + sx * 1.5f, FLOOR_Y + LEG_H / 2.0f, CZ + sz * 1.5f), glm::vec3(0.25f, LEG_H, 0.25f));
    addDecor(glm::vec3(CX, FLOOR_Y + 0.45f, CZ + 2.6f), glm::vec3(1.0f, 0.9f, 1.0f));   // stool, player side
    addDecor(glm::vec3(CX, FLOOR_Y + 0.45f, CZ - 2.6f), glm::vec3(1.0f, 0.9f, 1.0f));   // stool, wall side

    // The board picture lies on the slab, 0.005 above its top face (same z-fighting trick as the zone frames)
    const float BY = TOP_Y + 0.1f + 0.005f, B = 1.5f;           // half size of the board = 1.5
    chessBoard = new PolygonWithTexture(
        { glm::vec3(CX - B, BY, CZ + B), glm::vec3(CX + B, BY, CZ + B), glm::vec3(CX + B, BY, CZ - B), glm::vec3(CX - B, BY, CZ - B) },
        { glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f) });
    chessBoard->setTexture(decorTextures[0]);

    // 9. Decor piece 2: treasure chest at (10, -3, -10) (contract position 2): a body box + a slightly wider lid box.
    //    The picture is a carved chest front, so putting it on every face of both boxes looks like one ornate chest.
    chestTexture = loadTextures({ "textures/decor_chest.png" }, GL_CLAMP_TO_EDGE, GL_LINEAR);
    if (chestTexture.empty())
    {
        std::cerr << "Room: chest texture failed to load" << std::endl;
        return;
    }
    // body: 1.0 tall sitting on the floor (center y = -3 + 0.5); lid: 0.5 tall on top (center y = -2 + 0.25)
    decorParts.push_back(new Box(glm::vec3(10.0f, FLOOR_Y + 0.5f, -10.0f), glm::vec3(2.4f, 1.0f, 1.6f), chestTexture[0]));
    decorParts.push_back(new Box(glm::vec3(10.0f, FLOOR_Y + 1.25f, -10.0f), glm::vec3(2.5f, 0.5f, 1.7f), chestTexture[0]));

    // 10. Decor piece 3: the single 3D model, at (-12, -3, 8) (contract position 3).
    //     We don't know its size in advance, so we measure its bounding box (min/max of all vertices) and then:
    //     move it so its bottom center is the origin, scale so its biggest side is 3 units, move to the contract spot.
    //     Matrix = T(place) * S(scale) * T(-bottom center): the LAST one written is applied FIRST.
    decorModel = new Model("models/alrihla.glb");
    if (decorModel->meshes.empty())
    {
        std::cerr << "Room: 3D model failed to load" << std::endl;
        delete decorModel;
        decorModel = nullptr;
        return;
    }
    glm::vec3 mn(1e9f), mx(-1e9f);
    for (const Mesh& mesh : decorModel->meshes)
        for (const Vertex& v : mesh.vertices)
        {
            mn = glm::min(mn, v.Position);
            mx = glm::max(mx, v.Position);
        }
    glm::vec3 size = mx - mn;
    float s = 3.0f / glm::max(size.x, glm::max(size.y, size.z));
    glm::vec3 bottomCenter((mn.x + mx.x) * 0.5f, mn.y, (mn.z + mx.z) * 0.5f);
    decorModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-12.0f, FLOOR_Y, 8.0f))
                     * glm::scale(glm::mat4(1.0f), glm::vec3(s))
                     * glm::translate(glm::mat4(1.0f), -bottomCenter);
}

void drawRoom(Shader& shader)
{
    shader.setInt("textureSample", 0);
    glActiveTexture(GL_TEXTURE0);
    // Floor and walls first (the ceiling is always the LAST part)
    for (size_t i = 0; i + 1 < roomParts.size(); i++)
        roomParts[i]->draw(shader);

    // The carpet, lying on the floor under the table
    if (rugPart)
        rugPart->draw(shader);

    // The table (top + 4 legs), drawn before the ceiling
    for (Box* part : tableParts)
        part->draw(shader);

    // The 6 chairs around the table
    for (Box* part : chairParts)
        part->draw(shader);

    // Radio shelf, vintage radio and small fan on the right side of the room.
    for (Box* part : radioParts)
        part->draw(shader);

    // The potted plant in the back-left corner
    for (Box* part : plantParts)
        part->draw(shader);
    for (PolygonWithTexture* leaf : plantLeaves)
        leaf->draw(shader);

    // The 3 zone frames, on top of the felt
    for (PolygonWithTexture* bar : zoneParts)
        bar->draw(shader);

    // Decor: chess table (wood boxes) and the board picture on top
    for (Box* part : decorParts)
        part->draw(shader);
    if (chessBoard)
        chessBoard->draw(shader);

    // Decor piece 3: the 3D model (its own meshes bind their own textures; white color = no tint)
    if (decorModel)
    {
        shader.setVec3("objectColor", glm::vec3(1.0f));
        shader.setMat4("model", decorModelMatrix);
        decorModel->Draw(shader);
        glActiveTexture(GL_TEXTURE0);
    }

    // The poster on the back wall (shadow first, then the poster over it)
    if (posterShadow)
        posterShadow->draw(shader);
    if (posterPart)
        posterPart->draw(shader);

    // The two lanterns: metal parts are lit normally, the glass is drawn with emissive = 1 (it glows),
    // and emissive goes back to 0 right after, so the cards and the ceiling are lit as usual.
    for (Box* part : lanternParts)
        part->draw(shader);
    shader.setFloat("emissive", 1.0f);
    for (Box* part : glowParts)
        part->draw(shader);
    shader.setFloat("emissive", 0.0f);

    // Ceiling: culled when seen from above, so the top camera can see the table through it
    if (!roomParts.empty())
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        roomParts.back()->draw(shader);
        glDisable(GL_CULL_FACE);
    }
}

void destroyRoom()
{
    for (PolygonWithTexture* part : roomParts)
    {
        part->deleteBuffers();
        delete part;
    }
    roomParts.clear();
    for (Box* part : tableParts)
    {
        part->destroy();
        delete part;
    }
    tableParts.clear();
    for (PolygonWithTexture* bar : zoneParts)
    {
        bar->deleteBuffers();
        delete bar;
    }
    zoneParts.clear();
    for (Box* part : decorParts)
    {
        part->destroy();
        delete part;
    }
    decorParts.clear();
    if (chessBoard)
    {
        chessBoard->deleteBuffers();
        delete chessBoard;
        chessBoard = nullptr;
    }
    glDeleteTextures(static_cast<GLsizei>(decorTextures.size()), decorTextures.data());
    glDeleteTextures(static_cast<GLsizei>(chestTexture.size()), chestTexture.data());
    glDeleteTextures(static_cast<GLsizei>(rugTexture.size()), rugTexture.data());
    glDeleteTextures(static_cast<GLsizei>(leafTexture.size()), leafTexture.data());
    for (PolygonWithTexture* leaf : plantLeaves)
    {
        leaf->deleteBuffers();
        delete leaf;
    }
    plantLeaves.clear();
    for (Box* part : plantParts)
    {
        part->destroy();
        delete part;
    }
    plantParts.clear();
    if (rugPart)
    {
        rugPart->deleteBuffers();
        delete rugPart;
        rugPart = nullptr;
    }
    for (Box* part : chairParts)
    {
        part->destroy();
        delete part;
    }
    chairParts.clear();
    for (Box* part : radioParts)
    {
        part->destroy();
        delete part;
    }
    radioParts.clear();
    for (Box* part : lanternParts)
    {
        part->destroy();
        delete part;
    }
    lanternParts.clear();
    for (Box* part : glowParts)
    {
        part->destroy();
        delete part;
    }
    glowParts.clear();
    delete decorModel;                       // Model owns its meshes; deleting it frees them
    decorModel = nullptr;
    if (posterShadow)
    {
        posterShadow->deleteBuffers();
        delete posterShadow;
        posterShadow = nullptr;
    }
    if (posterPart)
    {
        posterPart->deleteBuffers();
        delete posterPart;
        posterPart = nullptr;
    }
    glDeleteTextures(static_cast<GLsizei>(posterTexture.size()), posterTexture.data());
    glDeleteTextures(static_cast<GLsizei>(zoneTexture.size()), zoneTexture.data());
    glDeleteTextures(static_cast<GLsizei>(tableTextures.size()), tableTextures.data());
    glDeleteTextures(static_cast<GLsizei>(roomTextures.size()), roomTextures.data());
}
