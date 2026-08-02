#version 330 core

in float vEnergy;
in float vT;
flat in int vRid;
flat in int vAid;

uniform float uTime; // playback time [ns]
uniform int uColorMode; // 0 generation, 1 energy, 2 species
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
        c = ramp(energyFraction());
    else if (uColorMode == 2)
        c = speciesColor(vAid);
    else
        c = genColor(vRid);
    fragColor = vec4(c, 1.0);
}
