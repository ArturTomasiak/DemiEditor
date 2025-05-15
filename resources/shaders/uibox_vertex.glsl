layout (location = 0) in vec2 vertex;

out VS_OUT {
    vec2 uv;
} vs_out;

uniform mat4 projection;
uniform mat4 model;

void main() {
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
    vs_out.uv = vertex.xy * 0.5 + 0.5;
}