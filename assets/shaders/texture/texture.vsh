#include "structures.fxh"

cbuffer VSConstants
{
    GlobalConstants g_Constants;
};

// Vertex shader takes two inputs: vertex position and color.
// By convention, Diligent Engine expects vertex shader inputs to be 
// labeled 'ATTRIBn', where n is the attribute number.
struct VSInput
{
    float3 Pos : ATTRIB0;
    float3 Normal : ATTRIB1;
    float2 UV : ATTRIB1;
};

struct VSOutput 
{ 
    float4 Pos : SV_POSITION;
    float2 UV : TEX_COORD;
};

// Note that if separate shader objects are not supported (this is only the case for old GLES3.0 devices), vertex
// shader output variable name must match exactly the name of the pixel shader input variable.
// If the variable has structure type (like in this example), the structure declarations must also be identical.
void main(
    in VSInput VSIn,
    out VSOutput VsOut
) 
{
    VsOut.Pos = mul(float4(VSIn.Pos, 1.0), g_Constants.WorldViewProj);
    VsOut.UV = VSIn.UV;
}
