#version 430 core

//You may need some other layouts.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aJoints;
layout (location = 3) in vec2 aJointWeights;
layout (location = 4) in float aVerticeID;

layout (std430, binding = 1) buffer storageBuffer1
{
    int relatedJoint1[];
};

layout (std430, binding = 2) buffer storageBuffer2
{
    int relatedJoint2[];
};

layout (std430, binding = 3) buffer storageBuffer3
{
    float weight1[];
};

layout (std430, binding = 4) buffer storageBuffer4
{
    float weight2[];
};

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
uniform vec3 Color;
uniform Joint jointList[50];
uniform int currentJoint;
uniform bool isPainting;
uniform mat4 tokenModel;
uniform bool predefinedMode;


void main()

{
    if (predefinedMode) {
        int id1 = int(aJoints[0]);
        int id2 = int(aJoints[1]);
        mat4 skinMat = aJointWeights[0] * jointList[id1].globalMat * jointList[id1].offsetMat + aJointWeights[1] * jointList[id2].globalMat * jointList[id2].offsetMat;
        vs_out.FragPos = vec3(model * skinMat * vec4(aPos,1.0));
        vs_out.Normal = mat3(transpose(inverse(model))) * vec3(skinMat * vec4(aNormal, 0.0));
        vs_out.Color = aJointWeights[0] * jointList[id1].color + aJointWeights[1] * jointList[id2].color;
    }
    else {
        int id = int(aVerticeID);
        if (relatedJoint1[id] == -1 && relatedJoint2[id] == -1) {
            vs_out.FragPos = vec3(model * vec4(aPos,1.0));
            vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
        }
        else {
            int id1 = relatedJoint1[id];
            int id2 = relatedJoint2[id];
            mat4 skinMat = weight1[id] * jointList[id1].globalMat * jointList[id1].offsetMat + weight2[id] * jointList[id2].globalMat * jointList[id2].offsetMat;
            vs_out.FragPos = vec3(model * skinMat * vec4(aPos,1.0));
            vs_out.Normal = mat3(transpose(inverse(model))) * vec3(skinMat * vec4(aNormal, 0.0));
        }
        
        vec3 tokenPos = vec3(tokenModel * vec4(0,0,0,1));
        float dist = (tokenPos.x - aPos.x) * (tokenPos.x - vs_out.FragPos.x) + (tokenPos.y - vs_out.FragPos.y) * (tokenPos.y - vs_out.FragPos.y) + (tokenPos.z - vs_out.FragPos.z) * (tokenPos.z - vs_out.FragPos.z);
        if (dist < 0.03 * 0.03 && isPainting) {
            if (relatedJoint1[id] == -1) {
                relatedJoint1[id] = currentJoint;
                weight1[id] = 1.0;
            }
            else {
                if (relatedJoint1[id] != currentJoint) {
                    if (relatedJoint2[id] == currentJoint) {
                        if (weight1[id] > 0 && weight2[id] < 1) {
                            weight1[id] = weight1[id] - 0.01;
                            weight2[id] = weight2[id] + 0.01;
                        }
                    }
                    if (relatedJoint2[id] == -1) {
                        relatedJoint2[id] = currentJoint;
                        weight2[id] = 0.1;
                        weight1[id] = 0.9;
                    }
                }
                else {
                    if (weight1[id] != 1.0) {
                        weight1[id] = weight1[id] + 0.01;
                        weight2[id] = weight2[id] - 0.01;
                    }
                }
            }
        }

        if (relatedJoint1[id] == -1 && relatedJoint2[id] == -1) {
            vs_out.Color = Color;
        }
        else {
            vs_out.Color = weight1[id] * jointList[relatedJoint1[id]].color + weight2[id] * jointList[relatedJoint2[id]].color;
        }
    }

    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}