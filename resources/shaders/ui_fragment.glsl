out vec4 frag_color;
in vec2 texture_coords;

uniform sampler2D texture_input;
uniform vec3 color;
uniform bool texture_passed;

void main() {
    if(texture_passed)
        frag_color = texture(texture_input, texture_coords);
    else
        frag_color = vec4(color, 1.0);
}