#include <iostream>
#include <ndtf/ndtf.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"

#include <string>
#include <format>
#include <filesystem>

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf(	"Usage: %s <input ndtf> <output image>\n"
				"Converts an NDTF (N-Dimensional Texture Format) to an image (jpeg/png/tga/bmp).\n"
				"The channel format is always 8-bit.\n"
			, argv[0]);
		return 1;
	}

	const char* input = argv[1];
	const char* output = argv[2];
	if (!std::filesystem::exists(input))
	{
		printf("Unable to load \"%s\" (file doesn't exist)\n", input);
		return 2;
	}
	std::string outExt = std::filesystem::path(output).extension().string();
	int outExtI = 0;
	if (outExt == ".png")
		outExtI = 0;
	else if (outExt == ".bmp")
		outExtI = 1;
	else if (outExt == ".tga")
		outExtI = 2;
	else if (outExt == ".jpeg")
		outExtI = 3;
	else if (outExt == ".jpg")
		outExtI = 3;
	else
	{
		printf("Extension \"%s\" is not supported for output!\n", outExt.c_str());
		return 3;
	}
	
	NDTF_File file = ndtf_file_load(input, nullptr, NDTF_TEXELFORMAT_NONE);
	if (!ndtf_file_isValid(&file))
	{
		printf("Unable to load \"%s\" (invalid/unsupported NDTF)\n", input);
		return 4;
	}
	switch (file.header.texelFormat)
	{
	case NDTF_TEXELFORMAT_RGBA16161616: { ndtf_file_reformat(&file, NDTF_TEXELFORMAT_RGBA8888); } break;
	case NDTF_TEXELFORMAT_RGB161616: { ndtf_file_reformat(&file, NDTF_TEXELFORMAT_RGB888); } break;
	case NDTF_TEXELFORMAT_R16: { ndtf_file_reformat(&file, NDTF_TEXELFORMAT_R8); } break;
	case NDTF_TEXELFORMAT_RGBA32323232F: { ndtf_file_reformat(&file, NDTF_TEXELFORMAT_RGBA8888); } break;
	case NDTF_TEXELFORMAT_RGB323232F: { ndtf_file_reformat(&file, NDTF_TEXELFORMAT_RGB888); } break;
	case NDTF_TEXELFORMAT_R32F: { ndtf_file_reformat(&file, NDTF_TEXELFORMAT_R8); } break;
	case NDTF_TEXELFORMAT_RGBA32323232: { ndtf_file_reformat(&file, NDTF_TEXELFORMAT_RGBA8888); } break;
	case NDTF_TEXELFORMAT_RGB323232: { ndtf_file_reformat(&file, NDTF_TEXELFORMAT_RGB888); } break;
	case NDTF_TEXELFORMAT_R32: { ndtf_file_reformat(&file, NDTF_TEXELFORMAT_R8); } break;
	}

	int width = file.header.width * file.header.ind;
	int height = file.header.height * file.header.depth * file.header.ind2;
	int channels = ndtf_getChannelCount((NDTF_TexelFormat)file.header.texelFormat);

	uint8_t* result = (uint8_t*)malloc(width * height * channels);

	NDTF_Coord coord{ 0 };

	for (coord.x = 0; coord.x < file.header.width; ++coord.x)
	{
		for (coord.y = 0; coord.y < file.header.height; ++coord.y)
		{
			for (coord.z = 0; coord.z < file.header.depth; ++coord.z)
			{
				for (coord.w = 0; coord.w < file.header.ind; ++coord.w)
				{
					for (coord.v = 0; coord.v < file.header.ind2; ++coord.v)
					{
						size_t x = (coord.x + coord.w * file.header.width) % width;
						size_t y = (coord.y + coord.z * file.header.height + coord.v * file.header.depth) % height;

						size_t iInd = (x + y * width) * channels;

						uint8_t* color = (uint8_t*)ndtf_file_getTexel(&file, &coord);

						for (uint8_t c = 0; c < channels; ++c)
						{
							result[iInd + c] = color[c];
						}
					}
				}
			}
		}
	}

	bool status = false;

	switch (outExtI)
	{
	case 0: { status = stbi_write_png(output, width, height, channels, result, width * channels); } break;
	case 1: { status = stbi_write_bmp(output, width, height, channels, result); } break;
	case 2: { status = stbi_write_tga(output, width, height, channels, result); } break;
	case 3: { status = stbi_write_jpg(output, width, height, channels, result, 100); } break;
	}

	if (!status)
	{
		printf("Failed to save the result image to \"%s\"\n", output);
	}
	else
	{
		printf("Saved the result image at \"%s\"\n", output);
	}

	free(result);
	ndtf_file_free(&file);

	return 0;
}
