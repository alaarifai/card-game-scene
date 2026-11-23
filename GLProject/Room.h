#pragma once

#include "tools/Polygon.h"

// Builds all room parts once (textures + polygons)
void buildRoom();

// Draws the whole room every frame
void drawRoom(Shader& shader);

// Frees buffers and textures
void destroyRoom();