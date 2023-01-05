#version 410 core

out vec4 fragColor;

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texcoord;

struct Light {
    float intensity;
    vec3 color;
    vec3 points[4];
    bool twoSided;
};

uniform Light areaLight;
uniform vec3 areaLightTranslate;

struct Material {
    sampler2D diffuse; // texture mapping
    vec4 albedoRoughness; // (x,y,z) = color, w = roughness
};
uniform Material material;

uniform vec3 viewPosition;
uniform sampler2D LTC1; // for inverse M
uniform sampler2D LTC2; // GGX norm, fresnel, 0(unused), sphere

const float LUT_SIZE = 64.0; // ltc_texture size
const float LUT_SCALE = (LUT_SIZE - 1.0)/LUT_SIZE;
const float LUT_BIAS = 0.5/LUT_SIZE;

vec3 N = normalize(worldNormal);
vec3 V = normalize(viewPosition - worldPosition);
vec3 P = worldPosition;
float dotNV = clamp(dot(N, V), 0.0f, 1.0f);

// use roughness and sqrt(1-cos_theta) to sample M_texture
vec2 uv = vec2(material.albedoRoughness.w, sqrt(1.0f - dotNV));
uv *= LUT_SCALE;
uv += LUT_BIAS;

// get 4 parameters for inverse_M
vec4 t1 = texture(LTC1, uv);

// Get 2 parameters for Fresnel calculation
vec4 t2 = texture(LTC2, uv);

mat3 Minv = mat3(
    vec3(t1.x, 0, t1.y),
    vec3(   0, 1,    0),
    vec3(t1.z, 0, t1.w)
);

vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided) {
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N * dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    // polygon (allocate 4 vertices for clipping)
    vec3 L[4];

    // transform polygon from LTC back to origin Do (cosine weighted)
    L[0] = Minv * (points[0] - P);
    L[1] = Minv * (points[1] - P);
    L[2] = Minv * (points[2] - P);
    L[3] = Minv * (points[3] - P);

    // use tabulated horizon-clipped sphere
    // check if the shading point is behind the light
    vec3 dir = points[0] - P; // LTC space
    vec3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
    bool behind = (dot(dir, lightNormal) < 0.0);

    // cos weighted space
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    // integrate
    vec3 vsum = vec3(0.0);
    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]);
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);

    // form factor of the polygon in direction vsum
    float len = length(vsum);

    float z = vsum.z/len;
    if (behind)
        z = -z;

    vec2 uv = vec2(z*0.5f + 0.5f, len); // range [0, 1]
    uv = uv*LUT_SCALE + LUT_BIAS;

    // Fetch the form factor for horizon clipping
    float scale = texture(LTC2, uv).w;

    float sum = len*scale;
    if (!behind && !twoSided)
        sum = 0.0;

    // Outgoing radiance (solid angle) for the entire polygon
    vec3 Lo_i = vec3(sum, sum, sum);
    return Lo_i;
}