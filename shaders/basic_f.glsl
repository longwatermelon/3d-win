#version 330 core
out vec4 FragColor;

in vec3 f_pos;
in vec2 f_tc;

uniform sampler2D tex;

void main()
{
    FragColor = texture(tex, f_tc);
}

