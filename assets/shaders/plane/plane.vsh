#include "structures.fxh"

cbuffer VSConstants
{
    GlobalConstants g_Constants;
};

struct VSOutput 
{ 
    float4 Pos : SV_POSITION;
};

void main(
    in uint VertexID : SV_VertexID,
    out VSOutput VsOut
)
{
    float width = 5.0;
    float y = -3.0;
    
    float4 positions[4];
    positions[0] = float4(-width, y, -width, 1.0);
    positions[1] = float4(-width, y, +width, 1.0);
    positions[2] = float4(+width, y, -width, 1.0);
    positions[3] = float4(+width, y, +width, 1.0);

    VsOut.Pos = mul(positions[VertexID], g_Constants.camera_view_projection);
}