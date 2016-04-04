#ifndef GLT_H_
#define GLT_H_

#include <exception> // For glt::parse_error()
#include <string>    // For std::string

#include <cstdio>  // For file reading
#include <cstdlib> // For malloc() and free()
#include <cstring> // For memcmp() and memset()

#include <GL/gl.h> // For gl_format().

#include "int.hpp" // Integer types

/** Cross-compiler NOEXCEPT support. */
#ifndef _MSC_VER
#  define _GLT_NOEXCEPT noexcept
#else
#  define _GLT_NOEXCEPT throw()
#endif

/* Define the pixel format values.
 *
 * These values correspond to OpenGl's
 * pixel formats. */
#define GLT_PIXEL_FORMAT_RGBA 0
#define GLT_PIXEL_FORMAT_BGRA 1

namespace glt{
    struct signature{
        u8 null;       // Null byte, helps prevent the file from being read in text mode.
        char magic[3]; // Magic string, encoded in ASCII ("GLT").

        // Major and minor version numbers
        u8 version_major;
        u8 version_minor;

        /** @brief Checks if the signature is valid. */
        bool is_valid(){
            /* For the signature to be valid, null must equal 0 and magic must equal "GLT".
             * Optionally, major and minor versions can be checked to assure compatibility. */

            // Compare values
            return this->null == 0x0 && memcmp(magic, "GLT", 3) == 0;
        }
    };

    struct texture_header{
        // Width and height of the texture.
        u64 width;
        u64 height;

        u64 format; // Pixel data format.

        // Returns the pixel format for OpenGL
        u32 gl_format(){
            switch(format){
                case GLT_PIXEL_FORMAT_RGBA:
                    return GL_RGBA;
                case GLT_PIXEL_FORMAT_BGRA:
                    return GL_BGRA;
                default:
                    return GL_RGBA;
            }
        }
    };

    /** @brief Thrown if a parse error ocurred. */
    class parse_error : public std::exception{
    private:
        std::string _what;
    public:
        parse_error(std::string what){
            this->_what = what;
        }
        ~parse_error() { }

        const char* what() const _GLT_NOEXCEPT override{
            return _what.c_str();
        }
    };

    class file{
    private:
        // File's signature and texture header.
        signature      _signature;
        texture_header _texture_header;

        // Pointer to the texture data, and its length.
        void   *_texture_data;
        size_t  _texture_data_length;

        size_t _pixel_length; // Length of each pixel
    public:
        file(const char*);
        ~file();

        /** @brief Flips the bytes in the texture data section. */
        void flip_bytes();

        /** @brief Frees all resources linked to this file. */
        void dispose();

        /** @brief Returns the file's texture data */
        void *get_texture_data(){ return this->_texture_data; }

        /** @breif Returns the length of the texture data */
        size_t get_texture_data_length(){ return this->_texture_data_length; }

        /** @brief Returns the length of each pixel element. */
        size_t get_pixel_length(){ return this->_pixel_length; }

        /** @brief Returns the file's signature. */
        signature get_signature(){ return this->_signature; }

        /** @brief Returns the file's texture header. */
        texture_header get_texture_header() { return this->_texture_header; }
    };
}

#endif // GLT_H_
