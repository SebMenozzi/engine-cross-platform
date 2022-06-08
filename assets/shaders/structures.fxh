struct GlobalConstants
{
    float4x4 camera_view_projection;
    float4x4 camera_view_projection_inverse;
    float3 camera_position;
    float3 sun_direction;
};

struct Vertex
{
    float3 position;
    float3 normal;
    float2 textcoord;
}; 