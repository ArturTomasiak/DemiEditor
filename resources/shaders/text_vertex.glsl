layout (location = 0) in vec2 vertex;

out VS_OUT{
    vec2 coordinates;
    flat int index;
}vs_out;

uniform mat4 transforms[arr_limit];
uniform mat4 projection;

void main() {
    gl_Position = projection * transforms[gl_InstanceID] * vec4(vertex.xy, 0.0, 1.0);
    vs_out.index = gl_InstanceID;
    vs_out.coordinates = vertex.xy;
    vs_out.coordinates.y = 1.0f - vs_out.coordinates.y;
}