// Writes a minimal QuickTime movie, adds xmp:Rating with XMPFiles, then prints
// the start of the track-level 'meta' atom before and after.
// Run with any argument to add the 4-byte version/flags field (MP4 layout) as a control.
#include <cstdio>
#include <string>
#include <vector>
#define TXMP_STRING_TYPE std::string
#define XMP_INCLUDE_XMPFILES 1
#include "XMP.incl_cpp"
#include "XMP.hpp"

// ftyp 'qt  ', then moov > trak > meta > hdlr, keys, ilst.
// QuickTime layout: 'hdlr' follows the 'meta' header directly, with no version/flags.
static const unsigned char kMovie[] = {
    0, 0, 0, 20, 'f', 't', 'y', 'p', 'q', 't', ' ', ' ', 0, 0, 0, 0, 'q', 't', ' ', ' ',
    0, 0, 0, 82, 'm', 'o', 'o', 'v',
    0, 0, 0, 74, 't', 'r', 'a', 'k',
    0, 0, 0, 66, 'm', 'e', 't', 'a',
    0, 0, 0, 34, 'h', 'd', 'l', 'r', 0, 0, 0, 0, 0, 0, 0, 0, 'm', 'd', 't', 'a', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 16, 'k', 'e', 'y', 's', 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 8, 'i', 'l', 's', 't',
};

static void dump(const char* label, const char* path) {
    unsigned char b[20];
    FILE* f = std::fopen(path, "rb");
    std::fseek(f, 36, SEEK_SET);
    size_t n = std::fread(b, 1, sizeof b, f);
    std::fclose(f);
    std::printf("%s", label);
    for (size_t i = 0; i < n; ++i) std::printf(" %02x", b[i]);
    std::printf("  ");
    for (size_t i = 0; i < n; ++i) std::putchar(b[i] >= 32 && b[i] < 127 ? b[i] : '.');
    std::printf("\n");
}

int main(int argc, char**) {
    std::vector<unsigned char> movie(kMovie, kMovie + sizeof kMovie);
    if (argc > 1) {
        movie.insert(movie.begin() + 44, 4, 0);
        movie[23] += 4, movie[31] += 4, movie[39] += 4;
    }
    const char* path = "test.mov";
    FILE* f = std::fopen(path, "wb");
    std::fwrite(movie.data(), 1, movie.size(), f);
    std::fclose(f);
    dump("before:", path);

    SXMPMeta::Initialize();
    SXMPFiles::Initialize(kXMPFiles_IgnoreLocalText);
    SXMPFiles file(path, kXMP_UnknownFile, kXMPFiles_OpenForUpdate | kXMPFiles_OpenUseSmartHandler);
    SXMPMeta meta;
    file.GetXMP(&meta);
    meta.SetProperty(kXMP_NS_XMP, "Rating", "5");
    file.PutXMP(meta);
    file.CloseFile();
    std::puts("PutXMP and CloseFile returned without error");

    dump("after: ", path);
    return 0;
}
