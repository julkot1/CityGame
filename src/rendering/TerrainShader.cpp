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

out vec3 vWorldNormal;
out vec3 vWorldPosition;
out vec3 vColor;
out float vAO;

void main() {
    vec4 worldPos = uTransformationMatrix * position;
    vWorldPosition = worldPos.xyz;
    vWorldNormal = mat3(uTransformationMatrix) * normal;
    vColor = color;
    vAO = ao;
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

out vec4 fragColor;

void main() {
    vec3 N = normalize(vWorldNormal);
    vec3 L = normalize(uLightDirection);
    vec3 V = normalize(uCameraPosition - vWorldPosition);

    // 1. High-contrast directional diffuse
    float NdotL = dot(N, L);
    float diffuseFactor = max(NdotL, 0.0);

    // Wrapped diffuse lighting for rich fill
    float wrappedDiffuse = max(0.0, (NdotL * 0.75 + 0.25));

    // Directional sunlight + cool ambient shadow
    vec3 light = uLightColor * wrappedDiffuse + uAmbientColor;

    // 2. Hemispheric sky/ground light
    float skyHemisphere = clamp(N.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 skyBounce = mix(vec3(0.08, 0.08, 0.12), vec3(0.20, 0.22, 0.25), skyHemisphere);
    light += skyBounce;

    // 3. Apply Voxel Corner Ambient Occlusion (darkens crevices and interior wall corners)
    light *= mix(0.40, 1.0, vAO);

    // 4. Subtle specular highlight on glistening water
    vec3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    float specular = pow(NdotH, 24.0) * 0.35 * float(vColor.b > vColor.g);

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

    // Dramatically angled, warm golden sun (low angle = deep, long cast shadows and high contrast on cliff faces)
    setLightDirection(Vector3{0.75f, 0.55f, 0.40f}.normalized());
    setLightColor(Color3{1.0f, 0.92f, 0.78f} * 1.25f);  // Bright golden sunlight
    setAmbientColor(Color3{0.20f, 0.26f, 0.38f});       // Crisp deep blue shadow ambient
    setSkyColor(0xa5c9ea_rgbf);                         // Clear sky
    setFogDistances(60.0f, 260.0f);                     // Visibly prominent distance fog
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

}
