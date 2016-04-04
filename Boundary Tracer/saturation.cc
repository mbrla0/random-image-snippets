#include "effect.hh"

int main(int /*argc*/, char** argv){
	// Load the texture into a buffer
	glt::file sourcef(argv[1]);
	
	// Create a bitmap representing that texture
	effect::Bitmap source;
	source.width  = sourcef.get_texture_header().width;
	source.height = sourcef.get_texture_header().height;
	source.data   = (effect::Pixel<u8>*) sourcef.get_texture_data();
	
	// Apply effects
	for(size_t x = 0; x < source.width; ++x){
		for(size_t y = 0; y < source.height; ++y){
			effect::Pixel<u8> *current = &source.data[y * source.width + x];
			
			effect::hsv data(current);
			
			*current = {static_cast<u8>(data.saturation), static_cast<u8>(data.saturation), static_cast<u8>(data.saturation)};
		}
	}
	
	// Write file
	write_bitmap(&source, std::string(argv[2]));
}
