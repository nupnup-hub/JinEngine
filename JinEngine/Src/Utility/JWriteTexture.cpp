#include"JWriteTexture.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include"../../Lib/stb-master/stb_image_write.h"

namespace JinEngine
{
	void WritePngFormatTexture(char const* filename, int w, int h, int comp, const void* data, const int pixelPerByte)
	{ 
		//stbi_write_jpg(filename, w, h, comp, data, 1);
		stbi_write_png(filename, w, h, comp, data, pixelPerByte * w);
	}
}