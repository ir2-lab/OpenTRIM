#version 330 core

layout(location = 0) in vec3 aPos; // nm
layout(location = 1) in float aEnergy; // eV
layout(location = 2) in float aT; // ns
layout(location = 3) in int aRid; // recoil generation

uniform mat4 uMvp;

out float vEnergy;
out float vT;
flat out int vRid;

void main()
{
    gl_Position = uMvp * vec4(aPos, 1.0);
    vEnergy = aEnergy;
    vT = aT;
    vRid = aRid;
}
