#include "rendering/TerrainShader.h"
#include <Corrade/Containers/StringView.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Extensions.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

namespace Magnum {

using namespace Math::Literals;

TerrainShader::TerrainShader() {
    GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex};
    GL::Shader frag{GL::Version::GL330, GL::Shader::Type::Fragment};

    vert.addSource(R"(
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in float ao;

uniform mat4 uTransformationMatrix;
uniform mat4 uProjectionMatrix;
uniform float uTime;

out vec3 vWorldNormal;
out vec3 vWorldPosition;
out vec3 vColor;
out float vAO;

void main() {
    vec4 worldPos = uTransformationMatrix * position;
    vec3 animNormal = mat3(uTransformationMatrix) * normal;
    float decodedAO = ao;

    // 1. Wind-swaying tree foliage animation (encoded with ao in [3.0 .. 4.1])
    if(ao > 2.5 && ao < 4.2) {
        decodedAO = ao - 3.0;
        // Upper canopy sways more than lower branches (based on decodedAO)
        float swayWeight = clamp((decodedAO - 0.72) * 3.5, 0.15, 1.0);
        float phaseX = uTime * 2.1 + worldPos.x * 0.65 + worldPos.z * 0.45;
        float phaseZ = uTime * 1.7 + worldPos.z * 0.65 - worldPos.x * 0.35;
        worldPos.x += sin(phaseX) * 0.045 * swayWeight;
        worldPos.z += cos(phaseZ) * 0.035 * swayWeight;
        worldPos.y += sin(phaseX * 1.5) * 0.012 * swayWeight;
    }

    vWorldPosition = worldPos.xyz;
    vWorldNormal = animNormal;
    vColor = color;
    vAO = decodedAO;
    gl_Position = uProjectionMatrix * worldPos;
}
)");

    frag.addSource(R"(
in vec3 vWorldNormal;
in vec3 vWorldPosition;
in vec3 vColor;
in float vAO;

uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform vec3 uSkyColor;
uniform float uFogNear;
uniform float uFogFar;
uniform float uTime;

out vec4 fragColor;

void main() {
    vec3 N = normalize(vWorldNormal);
    vec3 L = normalize(uLightDirection);
    vec3 V = normalize(uCameraPosition - vWorldPosition);

    // Emissive Victorian Gaslamp Lanterns with warm flame flicker (encoded with vAO in [1.5 .. 2.5])
    if(vAO > 1.5 && vAO < 2.5) {
        float distanceToCam = length(uCameraPosition - vWorldPosition);
        float fogFactor = smoothstep(uFogNear, uFogFar, distanceToCam);
        float flicker = 1.0 + 0.10 * sin(uTime * 8.5 + vWorldPosition.x * 11.0)
                            * cos(uTime * 13.0 + vWorldPosition.z * 17.0);
        vec3 gasGlow = clamp(vColor * 1.35 * flicker, 0.0, 1.0);
        vec3 finalGlow = mix(gasGlow, uSkyColor, fogFactor);
        fragColor = vec4(finalGlow, 1.0);
        return;
    }

    bool isWater = (vColor.b > vColor.g * 1.04) && (vColor.b > vColor.r * 1.25);

    // 1. High-contrast directional diffuse (clean, flat-shaded faces)
    float NdotL = dot(N, L);
    float wrappedDiffuse = max(0.0, (NdotL * 0.75 + 0.25));

    // Directional sunlight + ambient shadow
    vec3 light = uLightColor * wrappedDiffuse + uAmbientColor;

    // 2. Hemispheric sky/ground bounce light
    float skyHemisphere = clamp(N.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 skyBounce = mix(vec3(0.08, 0.07, 0.06), vec3(0.18, 0.20, 0.22), skyHemisphere);
    light += skyBounce;

    // 3. Apply Voxel Corner Ambient Occlusion
    light *= mix(0.40, 1.0, clamp(vAO, 0.0, 1.0));

    // 4. Animated specular highlight on water waves
    vec3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    float specular = pow(NdotH, 28.0) * 0.42 * float(isWater);

    vec3 litColor = vColor * light + vec3(specular);

    // 5. Atmospheric distance fog
    float distanceToCam = length(uCameraPosition - vWorldPosition);
    float fogFactor = clamp((distanceToCam - uFogNear) / (uFogFar - uFogNear), 0.0, 1.0);
    fogFactor = smoothstep(0.0, 1.0, fogFactor);

    vec3 finalColor = mix(litColor, uSkyColor, fogFactor);
    fragColor = vec4(finalColor, 1.0);
}
)");

    CORRADE_INTERNAL_ASSERT(vert.compile());
    CORRADE_INTERNAL_ASSERT(frag.compile());

    attachShaders({vert, frag});
    CORRADE_INTERNAL_ASSERT(link());

    _cameraPositionUniform = uniformLocation("uCameraPosition");
    _transformationMatrixUniform = uniformLocation("uTransformationMatrix");
    _projectionMatrixUniform = uniformLocation("uProjectionMatrix");
    _lightDirectionUniform = uniformLocation("uLightDirection");
    _lightColorUniform = uniformLocation("uLightColor");
    _ambientColorUniform = uniformLocation("uAmbientColor");
    _skyColorUniform = uniformLocation("uSkyColor");
    _fogNearUniform = uniformLocation("uFogNear");
    _fogFarUniform = uniformLocation("uFogFar");
    _timeUniform = uniformLocation("uTime");

    // Dramatically angled, warm golden sun (low angle = deep, long cast shadows and high contrast on cliff faces)
    setLightDirection(Vector3{0.75f, 0.55f, 0.40f}.normalized());
    setLightColor(Color3{1.0f, 0.92f, 0.78f} * 1.25f);  // Bright golden sunlight
    setAmbientColor(Color3{0.20f, 0.26f, 0.38f});       // Crisp deep blue shadow ambient
    setSkyColor(0xa5c9ea_rgbf);                         // Clear sky
    setFogDistances(60.0f, 260.0f);                     // Visibly prominent distance fog
    setTime(0.0f);
}

TerrainShader& TerrainShader::setCameraPosition(const Vector3& eyePos) {
    setUniform(_cameraPositionUniform, eyePos);
    return *this;
}

TerrainShader& TerrainShader::setTransformationMatrix(const Matrix4& matrix) {
    setUniform(_transformationMatrixUniform, matrix);
    return *this;
}

TerrainShader& TerrainShader::setProjectionMatrix(const Matrix4& matrix) {
    setUniform(_projectionMatrixUniform, matrix);
    return *this;
}

TerrainShader& TerrainShader::setLightDirection(const Vector3& direction) {
    setUniform(_lightDirectionUniform, direction);
    return *this;
}

TerrainShader& TerrainShader::setLightColor(const Color3& color) {
    setUniform(_lightColorUniform, color);
    return *this;
}

TerrainShader& TerrainShader::setAmbientColor(const Color3& color) {
    setUniform(_ambientColorUniform, color);
    return *this;
}

TerrainShader& TerrainShader::setSkyColor(const Color3& color) {
    setUniform(_skyColorUniform, color);
    return *this;
}

TerrainShader& TerrainShader::setFogDistances(Float nearDist, Float farDist) {
    setUniform(_fogNearUniform, nearDist);
    setUniform(_fogFarUniform, farDist);
    return *this;
}

TerrainShader& TerrainShader::setTime(Float timeSeconds) {
    setUniform(_timeUniform, timeSeconds);
    return *this;
}

}
