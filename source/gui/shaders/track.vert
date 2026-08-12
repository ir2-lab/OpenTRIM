#version 330 core

layout(location = 0) in vec3 aPos; // nm
layout(location = 1) in float aEnergy; // eV
layout(location = 2) in float aT; // ps
layout(location = 3) in int aRid; // recoil generation
layout(location = 4) in int aAid; // atom id

uniform mat4 uMvp;

out float vEnergy;
centroid out float vT;
flat out int vRid;
flat out int vAid;

void main()
{
    gl_Position = uMvp * vec4(aPos, 1.0);
    vEnergy = aEnergy;
    vT = aT;
    vRid = aRid;
    vAid = aAid;
}
