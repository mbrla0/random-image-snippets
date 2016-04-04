#include "effect.hh"

void trace_boundaries(effect::Bitmap* input, effect::Pixel<u8> line_color = {0xFF, 0xFF, 0xFF, 0xFF}){
	struct boundary{
		float             difference;
		effect::Pixel<u8>  color;
	};
	
	boundary **tmap = (boundary**) malloc(input->width * sizeof(boundary*));
	for(size_t i = 0; i < input->width; ++i){
		tmap[i] = (boundary*) malloc(input->height * sizeof(boundary));
	}
	
	// Scan horizontally for boundaries
	for(size_t y = 0; y < input->height; ++y){
		for(size_t x = 0; x < input->width; ++x){
			// Get current pixel
			effect::Pixel<u8> *current = &input->data[y * input->width + x];
			
			// Ignore if already at line_color
			if(*current == line_color){
				tmap[x][y] = {0, line_color};
				continue;
			}
			
			// Get hsv data for the previous, current and next Pixel<u8>s
			#define LEFT   input->data[y * input->width + (x == 0 ? x : x - 1)]
			#define RIGHT  input->data[y * input->width + (x == input->width - 1 ? x : x + 1)]
			#define TOP    input->data[(y == 0 ? y : y - 1)                 * input->width + x]
			#define BOTTOM input->data[(y == input->height - 1 ? y : y + 1) * input->width + x]
			#define TL     input->data[(y == 0 || x == 0 ? y : y - 1) * input->width \
			                         + (y == 0 || x == 0 ? x : x - 1)]
			#define BR     input->data[(y == input->height - 1 || x == input->width - 1 ? y : y + 1) * input->width \
			                         + (y == input->height - 1 || x == input->width - 1 ? x : x + 1)]
			#define TR     input->data[(y == 0 || x == input->width - 1 ? y : y - 1) * input->width \
			                         + (y == 0 || x == input->width - 1 ? x : x + 1)]
			#define BL     input->data[(y == input->height - 1 || x == 0 ? y : y + 1) * input->width \
			                         + (y == input->height - 1 || x == 0 ? x : x - 1)]
			
			#define DIFFERENCE(s1, s2) \
				std::max( \
					(s1 == line_color || s1.alpha == 0 ? \
					effect::hsv(current) : effect::hsv(&s1)).diff(effect::hsv(current)).peak(), \
					(s2 == line_color || s2.alpha == 0 ? \
					effect::hsv(current) : effect::hsv(&s2)).diff(effect::hsv(current)).peak()  \
				)
			
			boundary b;
			// Pick the hiest of the three differences to represent this pixel
			b.difference = (DIFFERENCE(LEFT, RIGHT) + DIFFERENCE(BOTTOM, TOP) + DIFFERENCE(TL, BR) + DIFFERENCE(TR, BL)) / 4;
			
			// Average the current pixel's color along with the other pixels'
			b.color = (((*current).cast<size_t>()
			             + LEFT.cast<size_t>()   + RIGHT.cast<size_t>() 
			             + BOTTOM.cast<size_t>() + TOP.cast<size_t>() 
			             + TL.cast<size_t>()     + BR.cast<size_t>() 
			             + TR.cast<size_t>()     + BL.cast<size_t>()) / 9).cast<u8>();
			
			tmap[x][y] = b;
			
			
			#undef LEFT
			#undef RIGHT
			#undef UP
			#undef DOWN
			#undef TL
			#undef BR
			#undef TR
			#undef BL
			
			#undef DIFFERENCE
		}
	}
	
	// Get the hihest value
	float highest_diff = 0;
	size_t highest_x = 0;
	size_t highest_y = 0;
	for(size_t x = 0; x < input->width; ++x){
		for(size_t y = 0; y < input->height; ++y){
			size_t i = tmap[x][y].difference;
			if(i > highest_diff){
				highest_x = x;
				highest_y = y;
				
				highest_diff = i;
			}
		}
	}
	
	printf("Highest diff: %f (%zu, %zu)\n", highest_diff, highest_x, highest_y);
	
	// Get the alpha value for every 1 of difference
	float alpha_per_diff = 0xFF / (highest_diff == 0 ? 1 : highest_diff);
	
	// Write map onto the image
	for(size_t x = 0; x < input->width; ++x){
		for(size_t y = 0; y < input->height; ++y){
			input->data[y * input->width + x] = {
				tmap[x][y].color.red,
				tmap[x][y].color.green,
				tmap[x][y].color.blue,
				static_cast<u8>(tmap[x][y].difference * alpha_per_diff)
			};
		}
	}
	
	free(tmap);
	
}

int main(int /*argc*/, char** argv){
	// Load the texture into a buffer
	glt::file sourcef(argv[1]);
	
	// Create a bitmap representing that texture
	effect::Bitmap source;
	source.width  = sourcef.get_texture_header().width;
	source.height = sourcef.get_texture_header().height;
	source.data   = (effect::Pixel<u8>*) sourcef.get_texture_data();
	
	// Apply effects
	trace_boundaries(&source);
	
	// Write file
	effect::write_bitmap(&source, std::string(argv[2]));
}
