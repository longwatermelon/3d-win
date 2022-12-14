#version 330 core
layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_tc;

out vec3 f_pos;
out vec2 f_tc;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    f_pos = vec3(model * vec4(i_pos, 1.));
    f_tc = i_tc;
    gl_Position = projection * view * vec4(f_pos, 1.);
}

