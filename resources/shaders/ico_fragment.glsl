out vec4 frag_color;
in vec2 texture_coords;

uniform sampler2D icon_texture;

void main() {
    frag_color = texture(icon_texture, texture_coords);
}