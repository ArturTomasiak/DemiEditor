out vec4 frag_color;
in vec2 texture_coords;

uniform sampler2D texture_input;
uniform vec3 color;

void main() {
    frag_color = texture(texture_input, texture_coords);
}