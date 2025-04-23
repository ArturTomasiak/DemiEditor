out vec4 color;

in VS_OUT{
    vec2 coordinates;
    flat int index;
}fs_in;

uniform sampler2DArray text;
uniform int letter_map[arr_limit];
uniform vec3 text_color;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, vec3(fs_in.coordinates.xy, letter_map[fs_in.index])).r);
    color = vec4(text_color, 1.0) * sampled;
}