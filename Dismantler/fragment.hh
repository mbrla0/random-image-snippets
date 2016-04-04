#ifndef __FRAGMENT_H__
#define __FRAGMENT_H__

#include "effect.hh"

#include <vector>

namespace fragment{
	// Typenames for default random generator and distribution
	typedef std::minstd_rand                      light_random_generator;
	typedef std::mt19937_64                       heavy_random_generator;
	typedef std::uniform_int_distribution<size_t> distribution;
	
	template<typename Generator = heavy_random_generator, typename Distribution = distribution>
	class key {
	private:
		std::vector<size_t> _values;
		size_t _base_salt = 1;
		size_t _rehashes  = 0;
		
	public:
		key(std::string key){
			// Calculate the base salt
			_base_salt = std::hash<std::string>()(key);
		
			// Hash and copy the key
			std::hash<char> hasher;
			for(char c : key){
				_values.push_back(hasher(c));
			}
		}
		
		void rehash(){
			++_rehashes;
			
			Generator generator;
			generator.seed(_base_salt);
			
			Distribution value_dist(0, _values.size() - 1);
			Distribution power_dist(-10, 10);
			Distribution salt_dist (-10, 10);
			
			// For every node, determine another node, which will be used
			// to generate the number that will be used as factor. 
			// 
			// Then, hash the result into the value that will be saved
			for(size_t i = 0; i < _values.size(); ++i){
				size_t salt_rnd = salt_dist(generator);
				size_t salt = (((i + 1) * _base_salt) / _rehashes) * (salt_rnd == 0 ? -1 : salt_rnd);
				
				generator.seed((_values[i] + 1) * salt);
				size_t j = value_dist(generator);
				
				generator.seed((_values[j] + 1) * salt);
				size_t factor = power_dist(generator);
				factor = factor == 0 ? -1 : factor;
				
				_values[i] = std::hash<size_t>()((_values[i] + 1) * factor);
			}
		}
		
		const std::vector<size_t> values() const{
			return _values;
		}
		
		const size_t average() const{
			size_t value = 0;
			
			for(size_t node : _values){
				value += node;
			}
			value /= _values.size();
			
			return value;
		}
		
		const size_t average_hash() const{
			return std::hash<size_t>()(this->average());
		}
	};
	
	struct pixel_block{
		static void copy(pixel_block& dest, pixel_block &src){
			size_t width  = std::min(dest.width,  src.width);
			size_t height = std::min(dest.height, src.height);
			
			for(size_t x = 0; x < width; ++x){
				for(size_t y = 0; y < height; ++y){
					//printf("\tCopying %zu, %zu\n", x, y);
					*dest.at(x, y) = *src.at(x, y);
				}
			}
		}
		
		size_t x, y;
		size_t width, height;
		
		size_t data_width;
		size_t data_height;
		effect::Pixel<u8> *data;
		
		pixel_block subblock(u8 ox, u8 oy){
			size_t new_x = (ox ? (width  / 2) : 0);
			size_t new_y = (oy ? (height / 2) : 0);
			
			if(new_x + (width  / 2) >= data_width)
				new_x = (data_width  - (width  / 2) - 1);
			if(new_y + (height / 2) >= data_height)
				new_y = (data_height - (height / 2) - 1);
			
			pixel_block tmp = { 
				new_x, 
				new_y, 
				width / 2, 
				height / 2,
				data_width,
				data_height,
				&data[new_y * data_width + new_x]
			};
			
			return tmp;
		}
		
		//pixel_block 
		
		
		effect::Pixel<u8> *at(size_t x, size_t y){
			return &data[y * data_width + x];
		}
	};
	
	struct operation{
		size_t opcode_table[0xC];
		std::vector<size_t> code;
			
		pixel_block block;
	};
	
	template<typename Generator = heavy_random_generator>
	std::vector<operation> block_operations(key<Generator>& key, pixel_block& block){
		std::vector<operation> tmp;
		
		for(size_t block_width = block.width, block_height = block.height;
			block_width >= 2 && block_height >= 2; 
			block_width /= 2, block_height /= 2){
			
			printf("New block dimentions: %zux%zu...", block_width, block_height);
			#pragma omp parallel for ordered collapse(2)
			for(size_t x = 0; x < block.width; x += block_width / 2){
				for(size_t y = 0; y < block.height; y += block_height / 2){
					// Redo the hashing
					key.rehash();
			
					// Set up the block operation
					operation op;
					op.block = {
						x, y, 
						x + block_width  > block.width  ? block.width  - x - 1 : block_width, 
						y + block_height > block.height ? block.height - y - 1 : block_height, 
					
						block.data_width, block.data_height, 
						&block.data[y * block.data_width + x]
					};
					
			
					// Setup opcode table and opcodes
					Generator rnd;
					fragment::distribution swap_dist(0, 0xB);
			
					#define so(slot) \
						rnd.seed(key.average() * (slot + 1)); \
						op.opcode_table[slot] = swap_dist(rnd);
					
					so(0x0); so(0x1);
					so(0x2); so(0x3);
					so(0x4); so(0x5);
					so(0x6); so(0x7);
					so(0x8); so(0x9);
					so(0xA); so(0xB);
					
					#undef so
					for(size_t node : key.values()){
						rnd.seed(node);
						op.code.push_back(swap_dist(rnd));
					}
					
					#pragma omp critical
					tmp.push_back(op);
				}
			}
			
			printf("Done!\n");
		}
		return tmp;
	}
}

#endif // __FRAGMENT_H__
