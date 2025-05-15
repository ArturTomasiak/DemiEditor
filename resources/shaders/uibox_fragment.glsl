out vec4 color;

in VS_OUT {
    vec2 uv;
} fs_in;

uniform vec3 box_color;
uniform float corner_brightness;
uniform float corner_size;

void main() {
    vec2 uv = fs_in.uv;
    float corner = smoothstep(0.0, corner_size, min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y)));
    float brightness = mix(corner_brightness, 1.0, corner);
    color = vec4(box_color * brightness, 1.0);
}
