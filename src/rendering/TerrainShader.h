#pragma once

#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Color.h>

namespace Magnum {

/**
 * Custom GLSL 3D Shader with Directional Sunlight, Ambient lighting, and Distance Fog.
 */
class TerrainShader: public GL::AbstractShaderProgram {
    public:
        typedef GL::Attribute<0, Vector3> Position;
        typedef GL::Attribute<1, Vector3> Normal;
        typedef GL::Attribute<2, Color3> Color;
        typedef GL::Attribute<3, Float> AmbientOcclusion;

        explicit TerrainShader();

        TerrainShader& setCameraPosition(const Vector3& eyePos);
        TerrainShader& setTransformationMatrix(const Matrix4& matrix);
        TerrainShader& setProjectionMatrix(const Matrix4& matrix);
        TerrainShader& setLightDirection(const Vector3& direction);
        TerrainShader& setLightColor(const Color3& color);
        TerrainShader& setAmbientColor(const Color3& color);
        TerrainShader& setSkyColor(const Color3& color);
        TerrainShader& setFogDistances(Float nearDist, Float farDist);
        TerrainShader& setTime(Float timeSeconds);

    private:
        Int _cameraPositionUniform{0};
        Int _transformationMatrixUniform{1};
        Int _projectionMatrixUniform{2};
        Int _lightDirectionUniform{3};
        Int _lightColorUniform{4};
        Int _ambientColorUniform{5};
        Int _skyColorUniform{6};
        Int _fogNearUniform{7};
        Int _fogFarUniform{8};
        Int _timeUniform{9};
};

}
