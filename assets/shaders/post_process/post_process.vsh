struct PSInput 
{ 
    float4 Pos : SV_POSITION; 
    float2 UV  : TEX_COORD; 
};

void main(
    in uint VertexID : SV_VertexID,
    out PSInput PSIn
) 
{
    // fullscreen triangle
    PSIn.UV  = float2(VertexID >> 1, VertexID & 1) * 2.0;
    PSIn.Pos = float4(PSIn.UV * 2.0 - 1.0, 0.0, 1.0);
}