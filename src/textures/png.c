#include "png.h"
#include <png.h>

PngTexture texture_create(const char* restrict file_path) {
    PngTexture texture = {0};

    FILE* file_pointer = fopen(file_path, "rb");
    if (!file_pointer) {
        error(err_file);
        return texture;
    }

    png_byte header[8];
    fread(header, 1, 8, file_pointer);
    if (png_sig_cmp(header, 0, 8)) {
        #ifdef demidebug
        fatal(__LINE__, __FILE__, "file is not a PNG");
        #endif
        error(err_png);
        fclose(file_pointer);
        return texture;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        error(err_texture);
        fclose(file_pointer);
        return texture;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        error(err_texture);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(file_pointer);
        return texture;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        error(err_texture);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(file_pointer);
        return texture;
    }

    png_init_io(png_ptr, file_pointer);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    texture.width        = png_get_image_width(png_ptr, info_ptr);
    texture.height       = png_get_image_height(png_ptr, info_ptr);
    png_byte bit_depth   = png_get_bit_depth(png_ptr, info_ptr);
    png_byte color_type  = png_get_color_type(png_ptr, info_ptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);

    png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER); // Force RGBA
    png_set_gray_to_rgb(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    int bytes_per_pixel = 4;
    int stride = texture.width * bytes_per_pixel;
    png_bytep buffer = (uint8_t*)malloc(texture.height * stride);
    if (!buffer) {
        error(err_memory_allocation);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(file_pointer);
        return texture;
    }
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * texture.height);
    if (!row_pointers) {
        free(buffer);
        error(err_memory_allocation);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(file_pointer);
        return texture;
    }
    for (int y = 0; y < texture.height; y++) {
        row_pointers[y] = buffer + y * stride;
    }

    png_read_image(png_ptr, row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    free(row_pointers);
    fclose(file_pointer);

    glGenTextures(1, &texture.renderer_id);
    glBindTexture(GL_TEXTURE_2D, texture.renderer_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    free(buffer);

    texture.bits_per_pixel = bytes_per_pixel * 8;
    return texture;
}

void texture_delete(PngTexture* restrict texture) {
    glDeleteTextures(1, &texture->renderer_id);
    texture->renderer_id = 0;
}

void texture_bind(uint32_t slot, const PngTexture* restrict texture) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture->renderer_id);
}

void texture_unbind() { 
    glBindTexture(GL_TEXTURE_2D, 0); 
}