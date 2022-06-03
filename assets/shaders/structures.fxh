struct GlobalConstants
{
    float4x4 camera_view_projection;
    float4 viewport_size;
};

struct ObjectConstants
{
    uint object_offset; // offset in g_ObjectAttribs
    uint padding0;
    uint padding1;
    uint padding2;
};

struct Object
{
    float4x4 model_matrix;    // object space position to world space

    #ifdef METAL
    float3x3 normal_matrix; // In Metal float4x3 type has 64 bytes size, but size of float3x3 is 48 bytes as float4x3 in HLSL.
    #else
    float4x3 normal_matrix; // object space normal to world space, float4x3 used because float3x3 has different size in D3D12 (36 bytes) and Vulkan (48 bytes)
    #endif

    uint material_id; // index in g_MaterialAttribs
    uint first_indice; // first indice in indice buffer
    uint first_vertex; // first vertex in vertex buffer
    uint mesh_id;  // Unused. Can be used to select index and vertex buffer in the buffer array.
};

struct Material
{
    float4 base_color_mask;
    uint sampler_index; // index in g_Samplers[];
    uint base_color_texture_index; // index in g_Textures[];
    uint padding0;
    uint padding1;
};

struct Vertex
{
    float3 position;
    float3 normal;
    float2 textcoord;
};