#include "glt.hpp"

namespace glt{
    file::file(const char* path){
        /* In case of fail, this constructor will
         * throw an instance of glt::parse_error() */

        /* Try to open the file specifyed in path,
         * in binary read mode. */
        FILE *file = fopen(path, "rb");

        if(file == NULL)
            throw parse_error("File \"" + std::string(path) + "\" could not be open.");

        /* Retrieve the file's signature,
         * and check if it is valid. */
        char signature_buffer[sizeof(signature)];
        fread(signature_buffer, sizeof(signature), 1, file);

        this->_signature = *((signature *) signature_buffer);
        if(!this->_signature.is_valid())
            throw parse_error("Signature for file \"" + std::string(path) + "\" is not valid.");

        /* Retrieve the file's texture header. */
        char texture_header_buffer[sizeof(texture_header)];
        fread(texture_header_buffer, sizeof(texture_header), 1, file);

        this->_texture_header = *((texture_header *) texture_header_buffer);

        /* Flip endianess for values in _texture_header,
         * in case the system is not little-endian. */
        if(!_LITTLE_ENDIAN()){
            _FLIP_ENDIAN<u64>(&_texture_header.width);
            _FLIP_ENDIAN<u64>(&_texture_header.height);

            _FLIP_ENDIAN<u64>(&_texture_header.format);
        }

        /* Calculate the length of the "Texture data" segment.
         *
         * Note: The GLT specification does not require overflow protection for
         *       this value, thus, none will be implemented here.
         */
        this->_texture_data_length = _texture_header.width * _texture_header.height;

        // Determine the length of each pixel.
        switch(_texture_header.format){
            case GLT_PIXEL_FORMAT_RGBA:
                this->_pixel_length = 4 * sizeof(u8);
                break;
            case GLT_PIXEL_FORMAT_BGRA:
                this->_pixel_length = 4 * sizeof(u8);
                break;
            default:
                this->_pixel_length = 4 * sizeof(u8);
        }

        // Multiply the number of pixels by the pixel length.
        _texture_data_length *= _pixel_length;

        /* Allocate a buffer for the texture data and fill it with zeros,
         * then, read the remaining of the file (Corresponding to the
         * file's third section) into it. */
         this->_texture_data = malloc(_texture_data_length);
         if(this->_texture_data == NULL)
            throw parse_error("Could not allocate memory for the texture data.");

        memset(_texture_data, 0, _texture_data_length);
        fread(_texture_data, _texture_data_length, 1, file);

        /* Close the file. */
        fclose(file);
    }

    file::~file(){
        // Dispose allocated data
        this->dispose();
    }

    void file::flip_bytes(){
        /* To some degree, the specification implies endian-safety,
         * so, in order to use some libraries (such as SDL 2) you
         * should be able to flip the byte values. */
        for(size_t pi = 0; pi < _texture_data_length / _pixel_length; ++pi){
            // Get the current pixel
            u8 *pixel = &(((u8 *) _texture_data)[pi * _pixel_length]);

            // Flip all bytes, according to _pixel_length
            // Do that by swapping the values for opposite bytes.
            for(size_t bi = 0; bi < _pixel_length / 2; ++bi){
                u8 one = pixel[bi];
                u8 two = pixel[_pixel_length - 1 - bi];

                pixel[bi] = two;
                pixel[_pixel_length - 1 - bi] = one;
            }
        }
    }

    void file::dispose(){
        /* Free the memory allocated for the texture data
         * and set its pointer to NULL. The remaining
         * resources will be freed on destruction */

        free(this->_texture_data);
        _texture_data = NULL;
    }
}
