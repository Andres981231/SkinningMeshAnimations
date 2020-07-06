#version 330 core

//You may need some other layouts.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aID;
layout (location = 3) in vec2 aWeight;
layout (location = 4) in vec2 aBone;
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

//uniform vec3 lightPos[4];
// for transformation
//uniform vec3 dirLightDir2VS;
//uniform vec3 viewPos2VS;

void main()

{
	vs_out.FragPos = vec3(model * vec4(aPos,1.0));
	vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;  
    vs_out.Color = Color;

    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
	
	//vs_out.TexCoords = aTexCoords;
    //vec3 T = normalize(mat3(transpose(inverse(model))) * aTangent);
    //vec3 N = normalize(vs_out.Normal);
    //vec3 B = normalize(mat3(transpose(inverse(model))) * aBitangent);
   
    //mat3 TBN = transpose(mat3(T,B,N));
    // do transformation
    //vs_out.TangentViewPos = TBN * viewPos2VS;
    //vs_out.TangentFragPos = TBN * vs_out.FragPos;
    //for(int i = 0; i < 4; i++){
    //    TangentLightPos[i] = TBN * lightPos[i];
    //}
    //TangentdirLightDir = TBN * dirLightDir2VS;
}