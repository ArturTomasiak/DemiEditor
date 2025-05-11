layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tex_coords;

out vec2 texture_coords;

uniform mat4 projection;
uniform mat4 model;

void main() {
    gl_Position = projection * model * vec4(position.xy, 0.0, 1.0);
    texture_coords = tex_coords;
}