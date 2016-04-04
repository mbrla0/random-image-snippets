/** glt-get: Program to convert GLT files into PNG */

#include <cstdio> // For C IO
#include <Magick++.h> // For image decoding

#include "glt/glt.hpp" // For everything GLT

int main(int argc, char** argv){
    if(argc <= 1){
        fprintf(stderr, "Usage: %s <in> <out>\n", argv[0]);
        return 3;
    }

    // Intialize ImageMagick
    Magick::InitializeMagick(*argv);

    // Open the image.
    glt::file file(argv[1]);

    // Get a blob to it
    Magick::Blob blob(file.get_texture_data(), file.get_texture_header().width * file.get_texture_header().height * 4);

    // Convert it using to PNG
    Magick::Image png;
    png.size(std::to_string(file.get_texture_header().width) + "x" + std::to_string(file.get_texture_header().height));
    png.depth(8);
	png.magick("RGBA");
	png.read(blob);
	
	png.magick("PNG");
	png.write(argv[2]);
	
    return 0;
}
