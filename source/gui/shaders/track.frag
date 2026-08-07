#version 330 core

in float vEnergy;
centroid in float vT;
flat in int vRid;
flat in int vAid;

uniform float uTime; // playback time [ns]
uniform int uColorMode; // 0 generation, 1 energy, 2 species
uniform int uColorMap; // continuous: 0 ramp 1 viridis 2 turbo; discrete: 0 default 1 tab10
uniform float uEnergyMin; // [eV]
uniform float uEnergyMax; // [eV]
uniform int uEnergyLog; // 1 = log scale

out vec4 fragColor;

// blue -> cyan -> green -> yellow -> red; mirrored in TrackColorBar::rampColor
vec3 ramp(float t)
{
    t = clamp(t, 0.0, 1.0);
    const vec3 c0 = vec3(0.0, 0.0, 1.0);
    const vec3 c1 = vec3(0.0, 1.0, 1.0);
    const vec3 c2 = vec3(0.0, 1.0, 0.0);
    const vec3 c3 = vec3(1.0, 1.0, 0.0);
    const vec3 c4 = vec3(1.0, 0.0, 0.0);
    if (t < 0.25)
        return mix(c0, c1, t / 0.25);
    if (t < 0.50)
        return mix(c1, c2, (t - 0.25) / 0.25);
    if (t < 0.75)
        return mix(c2, c3, (t - 0.50) / 0.25);
    return mix(c3, c4, (t - 0.75) / 0.25);
}

// viridis, 17-point LUT from the CC0 colormap data (BIDS, N. Smith & S. van der Walt,
// https://github.com/BIDS/colormap); linearly interpolated.
vec3 viridis(float t)
{
    const vec3 lut[17] = vec3[17](
            vec3(0.267004, 0.004874, 0.329415), vec3(0.282327, 0.094955, 0.417331),
            vec3(0.278826, 0.175490, 0.483397), vec3(0.257322, 0.256130, 0.526563),
            vec3(0.227802, 0.326594, 0.546532), vec3(0.195860, 0.395433, 0.555276),
            vec3(0.160665, 0.478540, 0.558115), vec3(0.127568, 0.566949, 0.551229),
            vec3(0.119483, 0.614817, 0.537692), vec3(0.150148, 0.676631, 0.506589),
            vec3(0.220124, 0.725509, 0.466226), vec3(0.311925, 0.767822, 0.415586),
            vec3(0.440137, 0.811138, 0.340967), vec3(0.575563, 0.844566, 0.256415),
            vec3(0.709898, 0.868751, 0.169257), vec3(0.835270, 0.886029, 0.102646),
            vec3(0.993248, 0.906157, 0.143936));
    float x = clamp(t, 0.0, 1.0) * 16.0;
    int i = int(floor(x));
    if (i >= 16) return lut[16];
    return mix(lut[i], lut[i + 1], x - float(i));
}

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

vec3 continuousColor(int map, float t)
{
    if (map == 1) return viridis(t);
    if (map == 2) return turbo(t);
    return ramp(t);
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

// color by recoil generation
vec3 genColor(int g)
{
    if (g <= 0) return vec3(1.0, 0.85, 0.2); // source ion
    if (g == 1) return vec3(1.0, 0.45, 0.1);
    if (g == 2) return vec3(0.9, 0.2, 0.2);
    if (g == 3) return vec3(0.6, 0.3, 0.8);
    return vec3(0.35, 0.6, 1.0); // deeper recoils
}

// distinct hue per atom id; mirrored in TrackColorBar::speciesColor
vec3 speciesColor(int a)
{
    float h = fract(float(a) * 0.618034);
    vec3 p = abs(fract(vec3(h) + vec3(1.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0);
    return clamp(p - 1.0, 0.0, 1.0);
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

void main()
{
    if (vT > uTime) // time evolution: hide vertices not yet reached
        discard;

    vec3 c;
    if (uColorMode == 1)
        c = continuousColor(uColorMap, energyFraction());
    else if (uColorMode == 2)
        c = uColorMap == 1 ? tab10(vAid) : speciesColor(vAid);
    else
        c = uColorMap == 1 ? tab10(vRid) : genColor(vRid);
    fragColor = vec4(clamp(c, 0.0, 1.0), 1.0);
}
