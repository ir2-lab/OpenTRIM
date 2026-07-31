#version 330 core

in float vEnergy;
in float vT;
flat in int vRid;

uniform float uTime; // playback time [ns]

out vec4 fragColor;

// color by recoil generation
vec3 genColor(int g)
{
    if (g <= 0) return vec3(1.0, 0.85, 0.2); // source ion
    if (g == 1) return vec3(1.0, 0.45, 0.1);
    if (g == 2) return vec3(0.9, 0.2, 0.2);
    if (g == 3) return vec3(0.6, 0.3, 0.8);
    return vec3(0.35, 0.6, 1.0); // deeper recoils
}

void main()
{
    if (vT > uTime) // time evolution: hide vertices not yet reached
        discard;
    fragColor = vec4(genColor(vRid), 1.0);
}
