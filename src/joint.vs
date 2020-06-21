#version 330 core

//You may need some other layouts.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
//layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Color;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 Color;

void main()

{
	vs_out.FragPos = vec3(model * vec4(aPos,1.0));
	vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;  
    vs_out.Color = Color;

    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
	
	
}