
cbuffer Constants
{
    float4x4 g_WorldViewProj;
};


void main(
    in uint VertexID : SV_VertexID,
    out PlanePSInput PSIn
)
{
    float width = 5.0;
    float y = -2.0;
    
    float4 positions[4];
    positions[0] = float4(-width, y, -width, 1.0);
    positions[1] = float4(-width, y, +width, 1.0);
    positions[2] = float4(+width, y, -width, 1.0);
    positions[3] = float4(+width, y, +width, 1.0);

    PSIn.Pos = mul(positions[VertexID], g_WorldViewProj);
}