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
    vec3 position;
    vec3 color;
    mat4 globalMat;
    mat4 offsetMat;
    float radius;
    int sonID;
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//uniform vec3 Color;
//uniform vec3 JointColor[29];
uniform Joint jointList[29];

bool isInSphere(int i);
bool isInFrustum(int i);
float distance2(vec3 a, vec3 b);

void main()

{
    int relatedJoints[2];relatedJoints[0] = -1;relatedJoints[1] = -1;
    float distToJoints[2];
    for(int i = 0;i < 6;i++){
        if(isInSphere(i) || isInFrustum(i)){
            if(relatedJoints[0] == -1){
                relatedJoints[0] = i;
                distToJoints[0] = distance2(vec3(model * vec4(aPos,1.0)), jointList[i].position);
            }else if(relatedJoints[1] == -1){
                relatedJoints[1] = i;
                distToJoints[1] = distance2(vec3(model * vec4(aPos,1.0)), jointList[i].position);   
            }
        }
    }
    float aJointWeights[2];
    mat4 skinMat = mat4(1.0f);
    if(relatedJoints[0] != -1 && relatedJoints[1] != -1){
        aJointWeights[0] = distToJoints[0] / (distToJoints[0] + distToJoints[1]);
        aJointWeights[1] = distToJoints[1] / (distToJoints[0] + distToJoints[1]);
        skinMat = aJointWeights[0] * jointList[relatedJoints[0]].globalMat * jointList[relatedJoints[0]].offsetMat + aJointWeights[1] * jointList[relatedJoints[1]].globalMat * jointList[relatedJoints[1]].offsetMat;
        vs_out.Color = jointList[relatedJoints[0]].color * aJointWeights[0] + jointList[relatedJoints[1]].color * aJointWeights[1];
    }else if(relatedJoints[0] != -1){
        skinMat = jointList[relatedJoints[0]].globalMat * jointList[relatedJoints[0]].offsetMat;
        vs_out.Color = jointList[relatedJoints[0]].color;
    }else{
        vs_out.Color = vec3(0.8,0.4,0.4);
    }
    //mat4 skinMat = aJointWeights[0] * jointList[id1].globalMat * jointList[id1].offsetMat + aJointWeights[1] * jointList[id2].globalMat * jointList[id2].offsetMat;
    vs_out.FragPos = vec3(model * skinMat * vec4(aPos,1.0));
	vs_out.Normal = mat3(transpose(inverse(model))) * vec3(skinMat * vec4(aNormal, 0.0));
    //vs_out.Color = Color;

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

bool isInSphere(int i){
    float dist2 = distance2(vec3(model * vec4(aPos,1.0)), jointList[i].position);
    if(dist2 <= jointList[i].radius * jointList[i].radius){
        return true;
    }else{
        return false;
    }
}

bool isInFrustum(int i){
    if(jointList[i].sonID == -1){return false;}
    if(isInSphere(jointList[i].sonID)){return true;}
    vec3 toV = vec3(model * vec4(aPos,1.0)) - jointList[i].position;
    vec3 toS = jointList[jointList[i].sonID].position - jointList[i].position;
    if(dot(toV,toS) < 0){return false;}
    float ptoInter = dot(toV, normalize(toS));
    float vtoInter = length(ptoInter*normalize(toS) - toV);
    if(ptoInter > length(toS)){return false;}
    float newR = (ptoInter/length(toS)) * jointList[jointList[i].sonID].radius + (1- ptoInter/length(toS)) * jointList[i].radius;
    if(vtoInter <= newR){return true;}
    return false;
}

float distance2(vec3 a, vec3 b){
    float ret = 0.0f;
    ret += (a.x-b.x)*(a.x-b.x);
    ret += (a.y-b.y)*(a.y-b.y);
    ret += (a.z-b.z)*(a.z-b.z);
    return ret;
}