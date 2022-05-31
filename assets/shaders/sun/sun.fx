#include "structures.fxh"

cbuffer VSConstants
{
    GlobalConstants g_Constants;
};

#define PI 3.14159265359
#define sunAngularRadius (32.0/2.0 / 60.0 * ((2.0 * PI)/180.0)) // Sun angular DIAMETER is 32 arc minutes
#define tanSunAngularRadius tan(sunAngularRadius)

struct SunVSOutput
{
    float2 NormalizedXY : NORMALIZED_XY; // Normalized device XY coordinates [-1,1]x[-1,1]
};

void SunPS(
    SunVSOutput VSOut,
    out float4 Color : SV_Target
)
{
    float2 cotanHalfFOV = float2(g_WorldViewProj[0][0], g_WorldViewProj[1][1]);
    float2 sunScreenSize = tanSunAngularRadius * cotanHalfFOV;
    float2 sunScreenPos = float2(1, 1);
    float2 xy = (VSOut.NormalizedXY - sunScreenPos) / sunScreenSize;

    Color.rgb = float3(1.0, 0.0, 0.0); //sqrt(saturate(1.0 - dot(xy, xy))) * float3(1.0, 1.0, 1.0);
    Color.a = 1.0;
}

void SunVS(
    in uint VertexId : SV_VertexID,
    out SunVSOutput VSOut,
    // IMPORTANT: non-system generated pixel shader input
    // arguments must have the exact same name as vertex shader 
    // outputs and must go in the same order.
    // Moreover, even if the shader is not using the argument,
    // it still must be declared.

    out float4 Pos : SV_Position
)
{
    float2 cotanHalfFOV = float2(g_Constants.WorldViewProj[0][0], g_Constants.WorldViewProj[1][1]);
    float2 sunScreenPos = float2(4, 4);
    float2 sunScreenSize = tanSunAngularRadius * cotanHalfFOV;
    float4 minMaxUV = sunScreenPos.xyxy + float4(-1.0, -1.0, 1.0, 1.0) * sunScreenSize.xyxy;
    
    /*
    float2 vertices[4];
    vertices[0] = minMaxUV.xy;
    vertices[1] = minMaxUV.xw;
    vertices[2] = minMaxUV.zy;
    vertices[3] = minMaxUV.zw;
    */

    float width = 1.0;
    float y = -1.0;
    
    float2 vertices[4];
    vertices[0] = float2(-width, y);
    vertices[1] = float2(-width, y);
    vertices[2] = float2(+width, y);
    vertices[3] = float2(+width, y);

    VSOut.NormalizedXY = vertices[VertexId];
    Pos = float4(vertices[VertexId], 1.0, 1.0);
}