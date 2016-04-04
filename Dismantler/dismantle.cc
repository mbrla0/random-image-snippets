#include "fragment.hh"
#include <iostream>
				
void swap(fragment::pixel_block& block, size_t ox1, size_t oy1, size_t ox2, size_t oy2){ 
	
	fragment::pixel_block block1 = block.subblock(ox1, oy1);
	fragment::pixel_block block2 = block.subblock(ox2, oy2);
	
	fragment::pixel_block tmp = {
		0, 0,
		block2.width, block2.height,
		
		block2.width, block2.height,
		(effect::Pixel<u8>*) malloc(block2.width * block2.height * sizeof(effect::Pixel<u8>))
	};
	
	fragment::pixel_block::copy(tmp,    block2);
	fragment::pixel_block::copy(block2, block1);
	fragment::pixel_block::copy(block1, tmp);
	
	free(tmp.data);
}

template<typename Generator>
void color_shift(fragment::pixel_block& block, size_t ox1, size_t oy1, size_t ox2, size_t oy2){
	fragment::pixel_block block1 = block.subblock(ox1, oy1);
	fragment::pixel_block block2 = block.subblock(ox2, oy2);
	
	size_t width  = std::min(block1.width,  block2.width);
	size_t height = std::min(block1.height, block2.height);
	
	Generator rnd;
	fragment::distribution color_dist(0x0, 0xFF);
	fragment::distribution direction_dist(0, 1);
	
	//printf("Applying color shift between subblocks (%zu, %zu) and (%zu, %zu) of block {%zu, %zu, %zu, %zu}\n", ox1, oy1, ox2, oy2, block.x, block.y, block.width, block.height);
	
	#pragma omp parallel for collapse(2)
	for(size_t x = 0; x < width; ++x){
		for(size_t y = 0; y < height; ++y){
			size_t salt = (width * height) * ((x + 1) * (y + 1)) + ox1 - oy2 + oy1 + ox2;
			
			effect::Pixel<u8> shift;
			
			rnd.seed(salt);
			size_t direction = direction_dist(rnd);
			if(direction){
				// Seed: Block2
				// Dest: Block1
				effect::Pixel<u8> *dest = block1.at(x, y);
				#define s(c) \
					rnd.seed(salt * (block2.at(x, y)->c ? block2.at(x, y)->c : 1)); \
					dest->c += color_dist(rnd);
				
				s(red);
				s(green);
				s(blue);
				s(alpha);
				
				#undef s
				
			}else{
				// Seed: Block1
				// Dest: Block2
				effect::Pixel<u8> *dest = block2.at(x, y);
				#define s(c) \
					rnd.seed(salt * (block1.at(x, y)->c ? block1.at(x, y)->c : 1)); \
					dest->c += color_dist(rnd);
				
				s(red);
				s(green);
				s(blue);
				s(alpha);
				
				#undef s
			}
		}
	}
}

template <typename G1, typename G2 = G1>
void apply_effect(fragment::key<G1>& key, effect::Bitmap& source){
	// Divide the image into multiple sizes of 2 x 2 blocks, and calculate
	// the operations in that formatq
	fragment::pixel_block block = {
		0, 0, 
		source.width, source.height, 
		
		source.width, source.height,
		source.data
	};
	
	std::vector<fragment::operation> operations = fragment::block_operations<G1>(key, block);
	
	// Run operations
	for(fragment::operation op : operations){
		for(size_t mangled_opcode : op.code){
			// Get current opcode
			size_t opcode = op.opcode_table[mangled_opcode];
			
			switch(opcode){
				// Position swap
				case 0x0: swap(op.block, 0, 0, 1, 0); break; // Top-left    <=> Top-right
				case 0x1: swap(op.block, 0, 1, 1, 1); break; // Bottom-left <=> Bottom-right
				case 0x2: swap(op.block, 0, 0, 0, 1); break; // Top-left    <=> Bottom-left
				case 0x3: swap(op.block, 1, 0, 1, 1); break; // Top-right   <=> Bottom-right
				case 0x4: swap(op.block, 0, 0, 1, 1); break; // Top-left    <=> Bottom-right
				case 0x5: swap(op.block, 0, 1, 1, 0); break; // Bottom-left <=> Top-right
				
				// Color shift
				case 0x6: color_shift<G2>(op.block, 0, 0, 1, 0); break; // Top-left    <=> Top-right
				case 0x7: color_shift<G2>(op.block, 0, 1, 1, 1); break; // Bottom-left <=> Bottom-right
				case 0x8: color_shift<G2>(op.block, 0, 0, 0, 1); break; // Top-left    <=> Bottom-left
				case 0x9: color_shift<G2>(op.block, 1, 0, 1, 1); break; // Top-right   <=> Bottom-right
				case 0xA: color_shift<G2>(op.block, 0, 0, 1, 1); break; // Top-left    <=> Bottom-right
				case 0xB: color_shift<G2>(op.block, 0, 1, 1, 0); break; // Bottom-left <=> Top-right
			}
		}
	}
}

int main(int argc, char** argv){
	struct{
		std::string source = "";
		std::string key    = "";
		std::string output = "";
		
		bool incomplete() { return source.empty() || key.empty() || output.empty(); }
		
		size_t complexity = 1;
	} flags;
	
	for(size_t i = 1; i < argc; ++i){
		// Parse flags
		if(std::string(argv[i]) == "--fast" || std::string(argv[i]) == "-f")
			flags.complexity = 0;
		else if(std::string(argv[i]) == "--complex" || std::string(argv[i]) == "-c")
			flags.complexity = 2;
		else{
			// Parse default arguments
			if(flags.source.empty())
				flags.source = argv[i];
			else{
				if(flags.key.empty())
					flags.key = argv[i];
				else if(flags.output.empty())
					flags.output = argv[i];
			}
			
		}
	}
	
	if(flags.incomplete()){
		fprintf(stderr, "Usage: %s <Input> <Key> <Output>\n", argv[0]);
		return 3;
	}
	
	#define run(generator1, generator2)\
		glt::file source_image(flags.source.c_str()); \
		fragment::key<generator1> key(flags.key); \
		\
		effect::Bitmap source; \
		source.width  = source_image.get_texture_header().width; \
		source.height = source_image.get_texture_header().height; \
		source.data   = (effect::Pixel<u8>*) source_image.get_texture_data(); \
		\
		apply_effect<generator1, generator2>(key, source); \
		\
		effect::write_bitmap(&source, flags.output)
	
	if(flags.complexity == 0){
		run(fragment::light_random_generator, fragment::light_random_generator);
	}else if(flags.complexity == 1){
		run(fragment::heavy_random_generator, fragment::light_random_generator);
	}else if(flags.complexity == 2){
		run(fragment::heavy_random_generator, fragment::heavy_random_generator);
	}
	
	#undef run
}
