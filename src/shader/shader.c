#include "shader.h"

static uint32_t shader_compile(uint32_t type, const char* source);
static char* shader_content(const char* location, int32_t arr_limit);
static int32_t get_uniform_location(Shader* shader, const char* name);

Shader shader_create(const char* vertex_shader_path, const char* fragment_shader_path, int32_t arr_limit) {
    char* vertex_shader = shader_content(vertex_shader_path, arr_limit);
    char* fragment_shader = shader_content(fragment_shader_path, arr_limit);
    Shader shader = {0};
    shader.cache = NULL;
    shader.renderer_id = glCreateProgram();
    uint32_t vs = shader_compile(GL_VERTEX_SHADER, vertex_shader);
    uint32_t fs = shader_compile(GL_FRAGMENT_SHADER, fragment_shader);
    if (!vs || !fs)
        return shader;
    glAttachShader(shader.renderer_id, vs);
    glAttachShader(shader.renderer_id, fs);
    glLinkProgram(shader.renderer_id);
    glValidateProgram(shader.renderer_id);
    glDeleteShader(vs);
    glDeleteShader(fs);
    free(vertex_shader);
    free(fragment_shader);
    return shader;
}

void shader_delete(Shader* shader) {
    if (shader->cache != NULL) {
        free(shader->cache);
        shader->cache = NULL;
        shader->cache_length = 0;
    }
    glDeleteProgram(shader->renderer_id);
    shader->renderer_id = 0;
}

extern inline void shader_bind(const Shader* shader) {
    glUseProgram(shader->renderer_id);
}

extern inline void shader_unbind() {
    glUseProgram(0);
}

void shader_set_uniform1i(Shader* shader, const char* name, int32_t value) {
    glUniform1i(get_uniform_location(shader, name), value);
}

void shader_set_uniform1iv(Shader* shader, const char* name, int32_t* matrix, uint32_t length) {
    glUniform1iv(get_uniform_location(shader, name), length, matrix);
}

void shader_set_uniform1f(Shader* shader, const char* name, float value) {
    glUniform1f(get_uniform_location(shader, name), value);
}

void shader_set_uniform3f(Shader* shader, const char* name, float v0, float v1, float v2) {
    glUniform3f(get_uniform_location(shader, name), v0, v1, v2);
}

void shader_set_uniform4f(Shader* shader, const char* name, float v0, float v1, float v2, float v3) {
    glUniform4f(get_uniform_location(shader, name), v0, v1, v2, v3);
}

void shader_set_uniformmat4f(Shader* shader, const char* name, float* matrix, uint32_t length) {
    glUniformMatrix4fv(get_uniform_location(shader, name), length, GL_FALSE, matrix);
}

static uint32_t shader_compile(uint32_t type, const char* source) {
    uint32_t id = glCreateShader(type);
    glShaderSource(id, 1, &source, 0);
    glCompileShader(id);
    int32_t result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        #ifdef demidebug
        int32_t length; 
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = malloc(length * sizeof(char));
        if (!message){
            fatal_error(err_memory_allocation);
            return 0;
        }
        glGetShaderInfoLog(id, length, &length, message);
        glDeleteShader(id);
        printf("%s\n%s", "in file: ", source);
        fatal(__LINE__, __FILE__, message);
        free(message);
        #else
        glDeleteShader(id);
        #endif
        fatal_error(err_shader_compilation);
    }
    return id;
}

static char* shader_content(const char* location, int32_t arr_limit) {
    char define_str[100];
    sprintf(define_str, "#version 460 core\n#define arr_limit %d\n", arr_limit);
    size_t define_size = strlen(define_str);
    FILE *file_pointer = fopen(location, "rb");
    if (!file_pointer) {
        #ifdef demidebug
        char* err = "failed to open file at: ";
        char* full_err = malloc(strlen(err) + strlen(location) + 1);
        if (!full_err){
            fatal_error(err_memory_allocation);
            return "\0";
        }
        strcpy(full_err, err);
        strcat(full_err, location);
        fatal(__LINE__, __FILE__, full_err);
        free(full_err);
        #endif
        fatal_error(err_file);
        return "\0";
    }
    fseek(file_pointer, 0, SEEK_END);
    uint32_t file_size = ftell(file_pointer);
    rewind(file_pointer);
    char *buffer = malloc(file_size + define_size + 1);
    if (!buffer) {
        fclose(file_pointer);
        fatal_error(err_memory_allocation);
        return "\0";
    }
    strcpy(buffer, define_str);
    size_t read_size = fread(buffer + define_size, 1, file_size, file_pointer);
    buffer[define_size + read_size] = '\0';
    fclose(file_pointer);
    return buffer;
}

static int32_t get_uniform_location(Shader* shader, const char* name) {
    for (uint32_t i = 0; i < shader->cache_length; i++)
        if (strcmp(shader->cache[i].name, name) == 0)
            return shader->cache[i].location;

    int32_t location = glGetUniformLocation(shader->renderer_id, name);
    #ifdef demidebug
    if (location == -1)
        warning(__LINE__, __FILE__, "uniform does not exist");
    #endif
    shader->cache_length++;
    shader->cache = realloc(shader->cache, sizeof(shader_uniform_cache) * shader->cache_length);
    shader->cache[shader->cache_length - 1].name = name;
    shader->cache[shader->cache_length - 1].location = location;
    return location;
}