/** glt-make: Program to convert image files into GLT */

#include <cstdio> // For C IO
#include <Magick++.h> // For image decoding

#include "glt/glt.hpp" // For everything GLT

int main(int argc, char** argv){
    if(argc <= 1){
        fprintf(stderr, "Usage: %s <file> [options]\n", argv[0]);
        return 3;
    }

    // Intialize ImageMagick
    Magick::InitializeMagick(*argv);

    // Open the image.
    Magick::Image image(argv[1]);

    // Convert it to RGBA
    u64 format = GLT_PIXEL_FORMAT_RGBA;
    image.magick("RGBA");

    // Get the image's data
    Magick::Blob blob;
    image.write(&blob);

    /** Create the GLT file. */
    // Signature
    glt::signature signature;

    signature.null = 0;

    signature.magic[0] = 'G';
    signature.magic[1] = 'L';
    signature.magic[2] = 'T';

    signature.version_major = 1;
    signature.version_minor = 0;

    // Texture header
    glt::texture_header header;

    header.width  = image.columns();
    header.height = image.rows();

    header.format = format;

    /** Flip the bytes, in case of a big-endian system */
    if(!_LITTLE_ENDIAN()){
        _FLIP_ENDIAN<u64>(&header.width);
        _FLIP_ENDIAN<u64>(&header.height);

        _FLIP_ENDIAN<u64>(&header.format);
    }

    /** Write to the GLT file. */
    FILE *file = fopen((std::string(argv[1]) + ".glt").c_str(), "wb");

    if(file == NULL){
        fprintf(stderr, "Could not open output file.\n");
        return 1;
    }

    fwrite(&signature, sizeof(glt::signature),      1, file);
    fwrite(&header,    sizeof(glt::texture_header), 1, file);

    // Write texture bytes
    fwrite(blob.data(), sizeof(u8), blob.length(), file);

    printf("File: %s\n\nWidth: %d\nHeight: %d\n\nFormat: %d\n\nLength: %d\n",
           argv[1], image.columns(), image.rows(), format, blob.length());

    fclose(file);

    return 0;
}
