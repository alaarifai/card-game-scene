#include "TextureLoader.h"
#include <iostream>
#include "stb_image.h"

std::vector<GLuint> loadTextures(
    const std::vector<std::string>& paths,
    GLenum wrapOption,
    GLenum filterOption)
{
    std::vector<GLuint> textures;

    
    stbi_set_flip_vertically_on_load(true);

    for (const std::string& path : paths)
    {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

        if (!data)
        {
            std::cout << "Failed to load texture: " << path << std::endl;
            continue;
        }

        
        GLenum format;
        if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else
        {
            std::cout << "Unsupported image format: " << path << std::endl;
            stbi_image_free(data);
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapOption);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapOption);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterOption);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterOption);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        
        glGenerateMipmap(GL_TEXTURE_2D);
        if (filterOption == GL_LINEAR)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        stbi_image_free(data);
        textures.push_back(texture);
    }

    return textures;
}
