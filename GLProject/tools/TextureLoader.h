#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <vector>
#include <string>
#include "libs/glad_loader_docs.h"

std::vector<GLuint> loadTextures(
    const std::vector<std::string>& paths,
    GLenum wrapOption = GL_REPEAT,
    GLenum filterOption = GL_LINEAR
);

#endif