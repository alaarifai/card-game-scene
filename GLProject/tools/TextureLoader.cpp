#include "TextureLoader.h"
#include <iostream>
#include "stb_image.h"

// Load several image files and return their OpenGL texture IDs in the same order.
std::vector<GLuint> loadTextures(
    const std::vector<std::string>& paths,
    GLenum wrapOption,
    GLenum filterOption
)
{
    std::vector<GLuint> textures;

    // OpenGL's texture origin is at the bottom-left, while image files usually start at the top-left.
    stbi_set_flip_vertically_on_load(true);

    for (const std::string& path : paths)
    {
        GLuint texture;

        // Reserve one texture ID for the current image.
        glGenTextures(1, &texture);

        int width, height, nrChannels;

        unsigned char* data = stbi_load(
            path.c_str(),
            &width,
            &height,
            &nrChannels,
            0
        );

        if (!data)
        {
            std::cout << "Failed to load texture: "
                      << path << std::endl;

            glDeleteTextures(1, &texture);
            continue;
        }

        // Select the matching OpenGL pixel format from the image channel count.
        GLenum format;

        if (nrChannels == 3)
        {
            format = GL_RGB;
        }
        else if (nrChannels == 4)
        {
            format = GL_RGBA;
        }
        else
        {
            std::cout << "Unsupported image format: "
                      << path << std::endl;

            // The GPU now has its copy, so release the temporary CPU image memory.
        stbi_image_free(data);
            glDeleteTextures(1, &texture);
            continue;
        }

        // Bind this texture so the following parameters and image data apply to it.
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            wrapOption
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            wrapOption
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            filterOption
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            filterOption
        );

        // Rows of RGB images are 3 bytes per pixel; widths not divisible by 4 (e.g. 1254) need alignment 1
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // Upload the pixel data to the GPU. Alignment 1 also supports RGB widths not divisible by 4.
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            format,
            width,
            height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            data
        );

        // Generate smaller copies used when the texture appears far away.
        glGenerateMipmap(GL_TEXTURE_2D);

        // Quality + speed: with GL_LINEAR the mipmaps above would never be used for far-away surfaces.
        // Trilinear mipmapping picks a smaller copy of the image when the surface is small on screen
        // (no shimmering, and the GPU reads far less memory).
        if (filterOption == GL_LINEAR)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        // Anisotropic filtering keeps the floor sharp when seen at a shallow angle (camera 2).
        // These two numbers come from the GL extension EXT_texture_filter_anisotropic.
        const GLenum MAX_ANISOTROPY = 0x84FE, MAX_ANISOTROPY_LIMIT = 0x84FF;
        GLfloat maxAniso = 1.0f;
        while (glGetError() != GL_NO_ERROR) {}          // clear old errors so only this query is checked
        glGetFloatv(MAX_ANISOTROPY_LIMIT, &maxAniso);
        if (glGetError() == GL_NO_ERROR && maxAniso > 1.0f)
            glTexParameterf(GL_TEXTURE_2D, MAX_ANISOTROPY, maxAniso > 8.0f ? 8.0f : maxAniso);

        // The GPU now has its copy, so release the temporary CPU image memory.
        stbi_image_free(data);

        textures.push_back(texture);
    }

    // Return all successfully loaded texture IDs to the caller.
    return textures;
}
