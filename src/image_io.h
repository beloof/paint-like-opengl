#pragma once
#include <string>
#include <vector>

// Reading and writing image files. Pixel data here is 8-bit RGB, tightly
// packed, rows ordered TOP to bottom (file order). Paths are UTF-8.
namespace imageio
{
    // Writes a PNG. Compression is a small built-in DEFLATE encoder (fixed
    // Huffman codes + LZ77), which handles the large flat areas of a drawing
    // well and needs no extra library.
    bool savePng(const std::string& path, int width, int height,
                 const std::vector<unsigned char>& rgb, std::string& error);

    // Loads PNG, JPEG, BMP, TGA, GIF (first frame), PSD, ... via stb_image.
    // Transparent pixels are composited over white.
    bool loadImage(const std::string& path, int& width, int& height,
                   std::vector<unsigned char>& rgb, std::string& error);

    // Scales an image to fit inside dstW x dstH (never upscaling), centres it on
    // a white background, and returns the dstW x dstH RGB result.
    std::vector<unsigned char> fitOnto(const std::vector<unsigned char>& rgb, int width, int height,
                                       int dstW, int dstH);
}
