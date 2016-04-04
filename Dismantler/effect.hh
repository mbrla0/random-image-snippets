#include "glt/glt.hpp" // For texture handling
#include <memory.h>    // For memory-related operations
#include <string>      // For C++ string management
#include <algorithm>   // For std::max() and std::min()
#include <cmath>       // For std::atan2()

namespace effect{
	size_t diff(size_t x, size_t y){
		return x > y ? x - y : y - x;
	}

	template<typename component_type=u8>
	struct Pixel{ // Asume RGBA
		component_type red   = 0x00;
		component_type green = 0x00;
		component_type blue  = 0x00;
		component_type alpha = 0xFF;
	
		const bool operator ==(Pixel<component_type> const& comparing) const{
			return comparing.red   == red   &&
		 	       comparing.green == green &&
		 	       comparing.blue  == blue  && 
		 	       comparing.alpha == alpha;
		}
	
		const Pixel<component_type> diff(Pixel<component_type> const& comparing) const{
			return { 
				effect::diff(red,   comparing.red),
				effect::diff(green, comparing.green),
				effect::diff(blue,  comparing.blue),
				effect::diff(alpha, comparing.alpha)
			};
		}
	
		Pixel<size_t> operator+(Pixel<component_type> const& b){
			return {
				static_cast<size_t>(red   + b.red),
				static_cast<size_t>(green + b.green),
				static_cast<size_t>(blue  + b.blue),
				static_cast<size_t>(alpha + b.alpha)
			};
		}
	
		Pixel<size_t> operator/(size_t const& b){
			return {
				red   / b,
				green / b,
				blue  / b,
				alpha / b
			};
		}
	
	
		template<typename T> 
		Pixel<T> cast(){
			return{
				(T) red,
				(T) green,
				(T) blue,
				(T) alpha
			};
		}
	};

	struct Bitmap{
		size_t width;
		size_t height;
	
		Pixel<u8>* data;
	
		const size_t length() const{
			return width * height;
		}
	
		Bitmap copy() const{
			Bitmap copy;
			copy.width  = width;
			copy.height = height;
		
			copy.data = (Pixel<u8>*) malloc(width * height * sizeof(Pixel<u8>));
			memcpy(copy.data, data, width * height * sizeof(Pixel<u8>));
		
			return copy;
		}
	};

	struct hsv{
		size_t hue;
		size_t saturation;
		size_t luminosity;
		
		hsv() { }
		
		hsv(Pixel<u8> *color){
			// Luminosity is the hightest value between the three
			luminosity = static_cast<size_t>((std::max(color->red, std::max(color->green, color->blue))) * (color->alpha / 0xFF));
	
			// Saturation is 0xFF - The lowest value between the three
			saturation = static_cast<size_t>((0xFF - std::min(color->red, std::min(color->green, color->blue))) * (color->alpha / 0xFF));
		}
		
		const bool operator ==(hsv const& comparing) const{
			return comparing.hue        == hue        &&
		 	       comparing.saturation == saturation &&
		 	       comparing.luminosity == luminosity;
		}
	
		const hsv diff(hsv const& comparing) const{
			hsv result;
			result.hue        = effect::diff(hue, comparing.hue) >= 320 ? 
			                    effect::diff(hue, comparing.hue) - 320 : effect::diff(hue, comparing.hue);
			result.saturation = effect::diff(saturation, comparing.saturation);
			result.luminosity = effect::diff(luminosity, comparing.luminosity);
			
			return result;
		}
	
		const size_t peak() const{
			return std::max(hue, std::max(saturation, luminosity));
		}
	
		const size_t average() const{
			return (hue + saturation + luminosity) / 3;
		}
	};

	void write_bitmap(Bitmap* bmap, const std::string& output){
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

		header.width  = bmap->width;
		header.height = bmap->height;

		header.format = GLT_PIXEL_FORMAT_RGBA;

		/** Write to the GLT file. */
		FILE *file = fopen(output.c_str(), "wb");

		if(file == NULL){
		    fprintf(stderr, "Could not open output file.\n");
		    return;
		}

		fwrite(&signature, sizeof(glt::signature),      1, file);
		fwrite(&header,    sizeof(glt::texture_header), 1, file);

		// Write texture bytes
		fwrite(bmap->data, sizeof(Pixel<u8>), bmap->length(), file);

		fclose(file);
	}
}
