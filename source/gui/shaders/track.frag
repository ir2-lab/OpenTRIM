#version 330 core

in float vEnergy;
centroid in float vT;
flat in int vRid;
flat in int vAid;

uniform float uTime; // playback time [ps]
uniform int uColorMode; // 0 generation, 1 energy, 2 species
uniform int uColorMap; // continuous: 0 rainbow 1 turbo; discrete: 0 tab10 1 set1
uniform float uEnergyMin; // [eV]
uniform float uEnergyMax; // [eV]
uniform int uEnergyLog; // 1 = log scale

out vec4 fragColor;

// turbo, (c) Google LLC, Apache-2.0. Design A. Mikhailov, GLSL approx R. Du
// (https://gist.github.com/mikhailov-work/0d177465a8151eb6ede1768d51d476c7).
vec3 turbo(float t)
{
    t = clamp(t, 0.0, 1.0);
    const vec4 kRedVec4 = vec4(0.13572138, 4.61539260, -42.66032258, 132.13108234);
    const vec4 kGreenVec4 = vec4(0.09140261, 2.19418839, 4.84296658, -14.18503333);
    const vec4 kBlueVec4 = vec4(0.10667330, 12.64194608, -60.58204836, 110.36276771);
    const vec2 kRedVec2 = vec2(-152.94239396, 59.28637943);
    const vec2 kGreenVec2 = vec2(4.27729857, 2.82956604);
    const vec2 kBlueVec2 = vec2(-89.90310912, 27.34824973);
    vec4 v4 = vec4(1.0, t, t * t, t * t * t);
    vec2 v2 = v4.zw * v4.z;
    return vec3(dot(v4, kRedVec4) + dot(v2, kRedVec2), dot(v4, kGreenVec4) + dot(v2, kGreenVec2),
                dot(v4, kBlueVec4) + dot(v2, kBlueVec2));
}

// matplotlib 'rainbow' colormap (exact analytic definition, not a fit).
// Source: matplotlib lib/matplotlib/_cm.py (_rainbow_data):
//   red = |2x - 1/2|, green = sin(pi x), blue = cos(pi x / 2)
// License: matplotlib license (BSD-compatible). https://matplotlib.org
vec3 mplRainbow(float x)
{
    x = clamp(x, 0.0, 1.0);
    return vec3(clamp(abs(2.0 * x - 0.5), 0.0, 1.0),
                sin(3.14159265 * x),
                cos(1.57079633 * x));
}

// Tableau 10 palette; values from matplotlib's BSD-licensed TABLEAU_COLORS ("tab10").
vec3 tab10(int i)
{
    i = i - 10 * (i / 10); // wrap into [0,10)
    if (i == 0) return vec3(0.121, 0.466, 0.705);
    if (i == 1) return vec3(1.000, 0.498, 0.054);
    if (i == 2) return vec3(0.172, 0.627, 0.172);
    if (i == 3) return vec3(0.839, 0.152, 0.156);
    if (i == 4) return vec3(0.580, 0.403, 0.741);
    if (i == 5) return vec3(0.549, 0.337, 0.294);
    if (i == 6) return vec3(0.890, 0.466, 0.760);
    if (i == 7) return vec3(0.498, 0.498, 0.498);
    if (i == 8) return vec3(0.737, 0.741, 0.133);
    return vec3(0.090, 0.745, 0.811);
}

// Set1 palette; values from matplotlib's BSD-licensed ("Set1"), ColorBrewer qualitative.
vec3 set1(int i)
{
    i = i - 9 * (i / 9); // wrap into [0,9)
    if (i == 0) return vec3(0.894, 0.102, 0.110);
    if (i == 1) return vec3(0.216, 0.494, 0.722);
    if (i == 2) return vec3(0.302, 0.686, 0.290);
    if (i == 3) return vec3(0.596, 0.306, 0.639);
    if (i == 4) return vec3(1.000, 0.498, 0.000);
    if (i == 5) return vec3(1.000, 1.000, 0.200);
    if (i == 6) return vec3(0.651, 0.337, 0.157);
    if (i == 7) return vec3(0.969, 0.506, 0.749);
    return vec3(0.600, 0.600, 0.600);
}

float energyFraction()
{
    float e = vEnergy, lo = uEnergyMin, hi = uEnergyMax;
    if (uEnergyLog == 1) {
        e = log(max(e, 1.0));
        lo = log(max(uEnergyMin, 1.0));
        hi = log(max(uEnergyMax, 1.0));
    }
    return (hi > lo) ? (e - lo) / (hi - lo) : 0.0;
}

vec3 continuousColor(int map, float t)
{
    return map == 0 ? mplRainbow(t) : turbo(t);
}

void main()
{
    if (vT > uTime) // time evolution: hide vertices not yet reached
        discard;

    vec3 c;
    if (uColorMode == 1)
        c = continuousColor(uColorMap, energyFraction());
    else if (uColorMode == 2)
        c = uColorMap == 0 ? tab10(vAid) : set1(vAid);
    else
        c = uColorMap == 0 ? tab10(vRid) : set1(vRid);

    fragColor = vec4(clamp(c, 0.0, 1.0), 1.0);
}
