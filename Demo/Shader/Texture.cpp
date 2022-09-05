
#include "Texture.h"
#include <gl/glew.h>
#include <cassert>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "../3rdParty/stb_image/stb_image.h"

bool Texture2D::InitializedIMG{ false };

Texture2D::Texture2D(const std::filesystem::path& path)
{
	stbi_set_flip_vertically_on_load(true);

	int nrChannels;
	unsigned char* data = stbi_load(path.string().c_str(), &m_Width, &m_Height, &nrChannels, 0);
	
	glGenTextures(1, &m_Id);
	glBindTexture(GL_TEXTURE_2D, m_Id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (data)
	{
		// Get Pixel Format
		GLenum pixelFormat{ GL_RGB };
		switch (nrChannels)
		{
		case 3:
			pixelFormat = GL_RGB;
		case 4:
			pixelFormat = GL_RGBA;
			break;
		default:
			throw std::runtime_error("Texture::CreateFromSurface, unknown pixel format");
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, pixelFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//if (!InitializedIMG)
	//{
	//	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
	//	{
	//		throw std::runtime_error(std::string("Failed to load support for png's: ") + SDL_GetError());
	//	}

	//	if ((IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) != IMG_INIT_JPG)
	//	{
	//		throw std::runtime_error(std::string("Failed to load support for jpg's: ") + SDL_GetError());
	//	}

	//	InitializedIMG = true;
	//}

	//SDL_Surface* pSurface{};
	//{
	//	pSurface = IMG_Load(path.string().c_str());
	//	if (!pSurface)
	//	{
	//		//throw std::runtime_error(std::string("Failed to load texture: ") + SDL_GetError());
	//		// TODO log error
	//	}
	//	std::cout << SDL_GetError();
	//}

	////glGenTextures(1, &m_Id);
	////glBindTexture(GL_TEXTURE_2D, m_Id);

	//// Get Image Size
	////int width = pSurface->w;
	////int height = pSurface->h;

	//// Get Pixel Format
	//GLenum pixelFormat{ GL_RGB };
	//switch (pSurface->format->BytesPerPixel)
	//{
	//case 3:
	//	if (pSurface->format->Rmask == 0x000000ff)
	//		pixelFormat = GL_RGB;
	//	else
	//		pixelFormat = GL_BGR_EXT;
	//	break;
	//case 4:
	//	if (pSurface->format->Rmask == 0x000000ff)
	//		pixelFormat = GL_RGBA;
	//	else
	//		pixelFormat = GL_BGRA_EXT;
	//	break;
	//default:
	//	throw std::runtime_error("Texture::CreateFromSurface, unknown pixel format");
	//}

	////Generate an array of textures.  We only want one texture (one element array), so trick
	////it by treating "texture" as array of length one.
	//glGenTextures(1, &m_Id);

	//// Select(bind) the texture we just generated as the current 2D texture OpenGL is using / modifying.
	////All subsequent changes to OpenGL's texturing state for 2D textures will affect this texture.
	//glBindTexture(GL_TEXTURE_2D, m_Id);
	//// check for errors. Can happen if a texture is created while a static pointer is being initialized, even before the call to the main function.
	//assert(!glGetError());

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//// Specify the texture's data.  
	//// This function is a bit tricky, and it's hard to find helpful documentation. 
	//// A summary:
	////    GL_TEXTURE_2D:    The currently bound 2D texture (i.e. the one we just made)
	////                0:    The mipmap level.  0, since we want to update the base level mipmap image (i.e., the image itself,
	////                         not cached smaller copies)
	////          GL_RGBA:    Specifies the number of color components in the texture.
	////                     This is how OpenGL will store the texture internally (kinda)--
	////                     It's essentially the texture's type.
	////       surface->w:    The width of the texture
	////       surface->h:    The height of the texture
	////                0:    The border.  Don't worry about this if you're just starting.
	////      pixelFormat:    The format that the *data* is in--NOT the texture! 
	//// GL_UNSIGNED_BYTE:    The type the data is in.  In SDL, the data is stored as an array of bytes, with each channel
	////                         getting one byte.  This is fairly typical--it means that the image can store, for each channel,
	////                         any value that fits in one byte (so 0 through 255).  These values are to be interpreted as
	////                         *unsigned* values (since 0x00 should be dark and 0xFF should be bright).
	////  surface->pixels:    The actual data.  As above, SDL's array of bytes.
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pSurface->w, pSurface->h, 0, pixelFormat, GL_UNSIGNED_BYTE, pSurface->pixels);

	//assert(!glGetError());

	//// Set the minification and magnification filters.  In this case, when the texture is minified (i.e., the texture's pixels (texels) are
	//// *smaller* than the screen pixels you're seeing them on, linearly filter them (i.e. blend them together).  This blends four texels for
	//// each sample--which is not very much.  Mipmapping can give better results.  Find a texturing tutorial that discusses these issues
	//// further.  Conversely, when the texture is magnified (i.e., the texture's texels are *larger* than the screen pixels you're seeing
	//// them on), linearly filter them.  Qualitatively, this causes "blown up" (overmagnified) textures to look blurry instead of blocky.
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//// Unbind texture
	//glBindTexture(GL_TEXTURE_2D, 0);

	//assert(!glGetError());

	//SDL_FreeSurface(pSurface);
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &m_Id);
}

