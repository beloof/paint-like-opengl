#include "image_io.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>

#include "stb_image.h" // implementation lives in brush.cpp

namespace
{
// ---------- Checksums ----------

uint32_t crc32(const unsigned char* data, std::size_t len, uint32_t crc = 0)
{
    static uint32_t table[256];
    static bool ready = false;
    if (!ready)
    {
        for (uint32_t n = 0; n < 256; n++)
        {
            uint32_t c = n;
            for (int k = 0; k < 8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            table[n] = c;
        }
        ready = true;
    }
    crc = ~crc;
    for (std::size_t i = 0; i < len; i++) crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return ~crc;
}

uint32_t adler32(const std::vector<unsigned char>& data)
{
    uint32_t a = 1, b = 0;
    for (unsigned char byte : data)
    {
        a = (a + byte) % 65521u;
        b = (b + a) % 65521u;
    }
    return (b << 16) | a;
}

// ---------- DEFLATE (RFC 1951), single block, fixed Huffman codes ----------

class BitWriter
{
public:
    std::vector<unsigned char> out;

    // Plain values (block header, extra bits) go least-significant bit first.
    void bits(uint32_t value, int count)
    {
        for (int i = 0; i < count; i++)
        {
            acc |= ((value >> i) & 1u) << used;
            if (++used == 8) flushByte();
        }
    }
    // Huffman codes go most-significant bit first.
    void code(uint32_t code, int length)
    {
        for (int i = length - 1; i >= 0; i--) bits((code >> i) & 1u, 1);
    }
    void finish() { if (used > 0) flushByte(); }

private:
    void flushByte() { out.push_back((unsigned char)acc); acc = 0; used = 0; }
    uint32_t acc = 0;
    int used = 0;
};

const int lenBase[29]   = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258 };
const int lenExtra[29]  = { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0 };
const int distBase[30]  = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,
                            2049,3073,4097,6145,8193,12289,16385,24577 };
const int distExtra[30] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13 };

// Fixed literal/length code (RFC 1951, 3.2.6).
void putSymbol(BitWriter& w, int sym)
{
    if (sym < 144)      w.code(0x30 + sym, 8);
    else if (sym < 256) w.code(0x190 + (sym - 144), 9);
    else if (sym < 280) w.code(sym - 256, 7);
    else                w.code(0xC0 + (sym - 280), 8);
}

void putMatch(BitWriter& w, int length, int distance)
{
    int li = 28;
    while (lenBase[li] > length) li--;
    putSymbol(w, 257 + li);
    w.bits(length - lenBase[li], lenExtra[li]);

    int di = 29;
    while (distBase[di] > distance) di--;
    w.code(di, 5);
    w.bits(distance - distBase[di], distExtra[di]);
}

std::vector<unsigned char> deflate(const std::vector<unsigned char>& in)
{
    constexpr int window = 32768, minMatch = 3, maxMatch = 258, maxChain = 32;
    constexpr int hashBits = 15;

    BitWriter w;
    w.bits(1, 1); // BFINAL
    w.bits(1, 2); // BTYPE = fixed Huffman

    const std::size_t n = in.size();
    std::vector<int> head(1u << hashBits, -1);
    std::vector<int> prev(n, -1);
    auto hashAt = [&](std::size_t i) {
        return ((in[i] << 10) ^ (in[i + 1] << 5) ^ in[i + 2]) & ((1u << hashBits) - 1);
    };
    auto insert = [&](std::size_t i) {
        if (i + minMatch > n) return;
        const uint32_t h = hashAt(i);
        prev[i] = head[h];
        head[h] = (int)i;
    };

    std::size_t i = 0;
    while (i < n)
    {
        int bestLen = 0, bestDist = 0;
        if (i + minMatch <= n)
        {
            const int limit = (int)std::min<std::size_t>(maxMatch, n - i);
            int cand = head[hashAt(i)];
            for (int chain = 0; cand >= 0 && chain < maxChain; chain++, cand = prev[cand])
            {
                const int dist = (int)i - cand;
                if (dist > window) break;
                int len = 0;
                while (len < limit && in[cand + len] == in[i + len]) len++;
                if (len > bestLen) { bestLen = len; bestDist = dist; if (len == limit) break; }
            }
        }

        if (bestLen >= minMatch)
        {
            putMatch(w, bestLen, bestDist);
            for (int k = 0; k < bestLen; k++) insert(i + k);
            i += bestLen;
        }
        else
        {
            putSymbol(w, in[i]);
            insert(i);
            i++;
        }
    }
    putSymbol(w, 256); // end of block
    w.finish();
    return std::move(w.out);
}

// ---------- PNG container ----------

void put32(std::vector<unsigned char>& v, uint32_t x)
{
    v.push_back((unsigned char)(x >> 24));
    v.push_back((unsigned char)(x >> 16));
    v.push_back((unsigned char)(x >> 8));
    v.push_back((unsigned char)x);
}

void putChunk(std::vector<unsigned char>& png, const char type[4], const std::vector<unsigned char>& data)
{
    put32(png, (uint32_t)data.size());
    const std::size_t typeAt = png.size();
    png.insert(png.end(), type, type + 4);
    png.insert(png.end(), data.begin(), data.end());
    put32(png, crc32(&png[typeAt], 4 + data.size()));
}
} // namespace

namespace imageio
{
bool savePng(const std::string& path, int width, int height,
             const std::vector<unsigned char>& rgb, std::string& error)
{
    if (width <= 0 || height <= 0 || rgb.size() != (std::size_t)width * height * 3)
    {
        error = "Invalid image size.";
        return false;
    }

    // Raw scanlines, each prefixed with filter type 0 (none).
    const std::size_t stride = (std::size_t)width * 3;
    std::vector<unsigned char> raw;
    raw.reserve((stride + 1) * height);
    for (int y = 0; y < height; y++)
    {
        raw.push_back(0);
        raw.insert(raw.end(), rgb.begin() + y * stride, rgb.begin() + (y + 1) * stride);
    }

    std::vector<unsigned char> zlib = { 0x78, 0x01 }; // deflate, 32K window
    const std::vector<unsigned char> body = deflate(raw);
    zlib.insert(zlib.end(), body.begin(), body.end());
    put32(zlib, adler32(raw));

    std::vector<unsigned char> ihdr;
    put32(ihdr, (uint32_t)width);
    put32(ihdr, (uint32_t)height);
    ihdr.insert(ihdr.end(), { 8, 2, 0, 0, 0 }); // 8-bit, truecolour RGB

    std::vector<unsigned char> png = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
    putChunk(png, "IHDR", ihdr);
    putChunk(png, "IDAT", zlib);
    putChunk(png, "IEND", {});

    std::ofstream f(std::filesystem::u8path(path), std::ios::binary | std::ios::trunc);
    if (!f)
    {
        error = "Could not open the file for writing.";
        return false;
    }
    f.write((const char*)png.data(), (std::streamsize)png.size());
    if (!f)
    {
        error = "Could not write the file.";
        return false;
    }
    return true;
}

bool loadImage(const std::string& path, int& width, int& height,
               std::vector<unsigned char>& rgb, std::string& error)
{
    // Read the bytes ourselves so non-ASCII (UTF-8) paths work on Windows too.
    std::ifstream f(std::filesystem::u8path(path), std::ios::binary);
    if (!f)
    {
        error = "Could not open the file.";
        return false;
    }
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)),
                                           std::istreambuf_iterator<char>());
    if (bytes.empty())
    {
        error = "The file is empty.";
        return false;
    }

    int channels = 0;
    unsigned char* data = stbi_load_from_memory(bytes.data(), (int)bytes.size(),
                                                &width, &height, &channels, 4);
    if (!data)
    {
        error = std::string("Not a supported image (") + stbi_failure_reason() + ").";
        return false;
    }

    // RGBA -> RGB over white.
    const std::size_t count = (std::size_t)width * height;
    rgb.resize(count * 3);
    for (std::size_t p = 0; p < count; p++)
    {
        const unsigned a = data[p * 4 + 3];
        for (int c = 0; c < 3; c++)
            rgb[p * 3 + c] = (unsigned char)((data[p * 4 + c] * a + 255 * (255 - a) + 127) / 255);
    }
    stbi_image_free(data);
    return true;
}

std::vector<unsigned char> fitOnto(const std::vector<unsigned char>& rgb, int width, int height,
                                   int dstW, int dstH)
{
    std::vector<unsigned char> out((std::size_t)dstW * dstH * 3, 255);

    const double scale = std::min({ 1.0, (double)dstW / width, (double)dstH / height });
    const int outW = std::max(1, (int)(width * scale + 0.5));
    const int outH = std::max(1, (int)(height * scale + 0.5));
    const int offX = (dstW - outW) / 2;
    const int offY = (dstH - outH) / 2;

    // Box filter: each output pixel averages the source pixels it covers.
    for (int y = 0; y < outH; y++)
    {
        const int sy0 = (int)((long long)y * height / outH);
        const int sy1 = std::max(sy0 + 1, (int)((long long)(y + 1) * height / outH));
        for (int x = 0; x < outW; x++)
        {
            const int sx0 = (int)((long long)x * width / outW);
            const int sx1 = std::max(sx0 + 1, (int)((long long)(x + 1) * width / outW));
            unsigned long sum[3] = { 0, 0, 0 };
            for (int sy = sy0; sy < sy1; sy++)
                for (int sx = sx0; sx < sx1; sx++)
                    for (int c = 0; c < 3; c++)
                        sum[c] += rgb[((std::size_t)sy * width + sx) * 3 + c];
            const unsigned long area = (unsigned long)(sy1 - sy0) * (sx1 - sx0);
            unsigned char* dst = &out[((std::size_t)(offY + y) * dstW + offX + x) * 3];
            for (int c = 0; c < 3; c++) dst[c] = (unsigned char)((sum[c] + area / 2) / area);
        }
    }
    return out;
}
} // namespace imageio
