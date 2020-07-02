#version 330 core

//You may need some other layouts.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aJoints;
layout (location = 3) in vec2 aJointWeights;
layout (location = 4) in vec2 aBoneWeights;
//layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Color;
} vs_out;

struct Joint {
    vec3 color;
    mat4 globalMat;
    mat4 offsetMat;
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//uniform vec3 Color;
//uniform vec3 JointColor[29];
uniform Joint jointList[29];

//uniform vec3 lightPos[4];
// for transformation
//uniform vec3 dirLightDir2VS;
//uniform vec3 viewPos2VS;

void main()

{
    int id1 = int(aJoints[0]);
    int id2 = int(aJoints[1]);
    mat4 skinMat = aJointWeights[0] * jointList[id1].globalMat * jointList[id1].offsetMat + aJointWeights[1] * jointList[id2].globalMat * jointList[id2].offsetMat;
    vs_out.FragPos = vec3(model * skinMat * vec4(aPos,1.0));
	vs_out.Normal = mat3(transpose(inverse(model))) * vec3(skinMat * vec4(aNormal, 0.0));
    //vs_out.Color = Color;
    vs_out.Color = jointList[id1].color * aJointWeights[0] + jointList[id2].color * aJointWeights[1];

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