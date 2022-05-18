#pragma once

#include <vector>

#include "object.hpp"
#include "array_3D.hpp"

namespace engine
{
    namespace object
    {
        enum FACE_POSITION { ABOVE, BELOW, LEFT, RIGHT, FRONT, BACK };

        const int CHUNK_SIZE = 10;
        const int BLOCK_SIZE = 1;

                /*
                     2--------0
                   / |       /|
                  /  |      / |
                 /   |     /  |
                3--------1    |
                |    5---|----4 
                |   /    |   /
                |  /     |  /
                | /      | /
                7--------6

                <------->
                   size
        */
        inline void generate_face(
            std::vector<Diligent::float3>& vertices,
            int x, int y, int z, 
            FACE_POSITION facing
        )
        {
            Diligent::float3 v;

            if (facing == ABOVE)
            {
                v.x = x + BLOCK_SIZE; v.y = y + BLOCK_SIZE; v.z = z + BLOCK_SIZE; vertices.push_back(v); // 0
                v.x = x + BLOCK_SIZE; v.y = y + BLOCK_SIZE; v.z = z;              vertices.push_back(v); // 1
                v.x = x;              v.y = y + BLOCK_SIZE; v.z = z + BLOCK_SIZE; vertices.push_back(v); // 2

                v.x = x;              v.y = y + BLOCK_SIZE; v.z = z + BLOCK_SIZE; vertices.push_back(v); // 2
                v.x = x + BLOCK_SIZE; v.y = y + BLOCK_SIZE; v.z = z;              vertices.push_back(v); // 1
                v.x = x;              v.y = y + BLOCK_SIZE; v.z = z;              vertices.push_back(v); // 3
            }
            else if (facing == BELOW)
            {
                v.x = x + BLOCK_SIZE; v.y = y;              v.z = z + BLOCK_SIZE; vertices.push_back(v); // 4
                v.x = x;              v.y = y;              v.z = z + BLOCK_SIZE; vertices.push_back(v); // 5
                v.x = x + BLOCK_SIZE; v.y = y;              v.z = z;              vertices.push_back(v); // 6

                v.x = x + BLOCK_SIZE; v.y = y;              v.z = z;              vertices.push_back(v); // 6
                v.x = x;              v.y = y;              v.z = z + BLOCK_SIZE; vertices.push_back(v); // 5
                v.x = x;              v.y = y;              v.z = z;              vertices.push_back(v); // 7
            }
            else if (facing == LEFT)
            {
                v.x = x;              v.y = y + BLOCK_SIZE; v.z = z + BLOCK_SIZE; vertices.push_back(v); // 2
                v.x = x;              v.y = y + BLOCK_SIZE; v.z = z;              vertices.push_back(v); // 3
                v.x = x;              v.y = y;              v.z = z + BLOCK_SIZE; vertices.push_back(v); // 5

                v.x = x;              v.y = y;              v.z = z + BLOCK_SIZE; vertices.push_back(v); // 5
                v.x = x;              v.y = y + BLOCK_SIZE; v.z = z;              vertices.push_back(v); // 3
                v.x = x;              v.y = y;              v.z = z;              vertices.push_back(v); // 7
            }
            else if (facing == RIGHT)
            {
                v.x = x + BLOCK_SIZE; v.y = y + BLOCK_SIZE; v.z = z + BLOCK_SIZE; vertices.push_back(v); // 0
                v.x = x + BLOCK_SIZE; v.y = y;              v.z = z + BLOCK_SIZE; vertices.push_back(v); // 4
                v.x = x + BLOCK_SIZE; v.y = y + BLOCK_SIZE; v.z = z;              vertices.push_back(v); // 1

                v.x = x + BLOCK_SIZE; v.y = y + BLOCK_SIZE; v.z = z;              vertices.push_back(v); // 1
                v.x = x + BLOCK_SIZE; v.y = y;              v.z = z + BLOCK_SIZE; vertices.push_back(v); // 4
                v.x = x + BLOCK_SIZE; v.y = y;              v.z = z;              vertices.push_back(v); // 6
            }
            else if (facing == FRONT)
            {
                v.x = x + BLOCK_SIZE; v.y = y + BLOCK_SIZE; v.z = z + BLOCK_SIZE; vertices.push_back(v); // 0
                v.x = x;              v.y = y + BLOCK_SIZE; v.z = z + BLOCK_SIZE; vertices.push_back(v); // 2
                v.x = x;              v.y = y;              v.z = z + BLOCK_SIZE; vertices.push_back(v); // 5

                v.x = x;              v.y = y;              v.z = z + BLOCK_SIZE; vertices.push_back(v); // 5
                v.x = x + BLOCK_SIZE; v.y = y;              v.z = z + BLOCK_SIZE; vertices.push_back(v); // 4
                v.x = x + BLOCK_SIZE; v.y = y + BLOCK_SIZE; v.z = z + BLOCK_SIZE; vertices.push_back(v); // 0
            }
            else if (facing == BACK)
            {
                v.x = x;              v.y = y;              v.z = z; vertices.push_back(v); // 7
                v.x = x;              v.y = y + BLOCK_SIZE; v.z = z; vertices.push_back(v); // 3
                v.x = x + BLOCK_SIZE; v.y = y + BLOCK_SIZE; v.z = z; vertices.push_back(v); // 1

                v.x = x + BLOCK_SIZE; v.y = y + BLOCK_SIZE; v.z = z; vertices.push_back(v); // 1
                v.x = x + BLOCK_SIZE; v.y = y;              v.z = z; vertices.push_back(v); // 6
                v.x = x;              v.y = y;              v.z = z; vertices.push_back(v); // 7
            }
        }

        inline VERTEX_DATA generate_random_chunk()
        {
            std::vector<Diligent::float3> vertices;
            std::vector<Diligent::float2> textcoords;

            Array3D<int> points(
                BLOCK_SIZE * CHUNK_SIZE, 
                BLOCK_SIZE * CHUNK_SIZE, 
                BLOCK_SIZE * CHUNK_SIZE
            );
            
            for (int x = 0; x < CHUNK_SIZE; x += BLOCK_SIZE)
                for (int y = 0; y < CHUNK_SIZE; y += BLOCK_SIZE)
                    for (int z = 0; z < CHUNK_SIZE; z += BLOCK_SIZE)
                        points(x, y, z) = rand() % 2;

            for (int x = 0; x < CHUNK_SIZE; x += BLOCK_SIZE)
            {
                for (int y = 0; y < CHUNK_SIZE; y += BLOCK_SIZE)
                {
                    for (int z = 0; z < CHUNK_SIZE; z += BLOCK_SIZE)
                    {
                        int value = points(x, y, z);

                        // If the current block is solid
                        if (value == 1)
                        {
                            int n1, n2, n3, n4, n5, n6;

                            try { n1 = points(x - BLOCK_SIZE, y, z); }
                            catch (const std::out_of_range& e) { n1 = 0; }

                            try { n2 = points(x + BLOCK_SIZE, y, z); }
                            catch (const std::out_of_range& e) { n2 = 0; }

                            try { n3 = points(x, y - BLOCK_SIZE, z); }
                            catch (const std::out_of_range& e) { n3 = 0; }

                            try { n4 = points(x, y + BLOCK_SIZE, z); }
                            catch (const std::out_of_range& e) { n4 = 0; }

                            try { n5 = points(x, y, z - BLOCK_SIZE); }
                            catch (const std::out_of_range& e) { n5 = 0; }

                            try { n6 = points(x, y, z + BLOCK_SIZE); }
                            catch (const std::out_of_range& e) { n6 = 0; }
                            
                            if (n1 == 0)
                            {
                                generate_face(vertices, x, y, z, LEFT);
                                textcoords.push_back(Diligent::float2(1, 0));
                                textcoords.push_back(Diligent::float2(0, 0));
                                textcoords.push_back(Diligent::float2(0, 1));
                                textcoords.push_back(Diligent::float2(1, 1));
                            }
                            if (n2 == 0)
                            {
                                generate_face(vertices, x, y, z, RIGHT);
                                textcoords.push_back(Diligent::float2(0, 1));
                                textcoords.push_back(Diligent::float2(1, 1));
                                textcoords.push_back(Diligent::float2(1, 0));
                                textcoords.push_back(Diligent::float2(0, 0));
                            }
                            if (n3 == 0)
                            {
                                generate_face(vertices, x, y, z, BELOW);
                                textcoords.push_back(Diligent::float2(0, 1));
                                textcoords.push_back(Diligent::float2(0, 0));
                                textcoords.push_back(Diligent::float2(1, 0));
                                textcoords.push_back(Diligent::float2(1, 1));
                            }
                            if (n4 == 0)
                            {
                                generate_face(vertices, x, y, z, ABOVE);
                                textcoords.push_back(Diligent::float2(1, 1));
                                textcoords.push_back(Diligent::float2(0, 1));
                                textcoords.push_back(Diligent::float2(0, 0));
                                textcoords.push_back(Diligent::float2(1, 0));
                            }
                            if (n5 == 0)
                            {
                                generate_face(vertices, x, y, z, BACK);
                                textcoords.push_back(Diligent::float2(0, 1));
                                textcoords.push_back(Diligent::float2(0, 0));
                                textcoords.push_back(Diligent::float2(1, 0));
                                textcoords.push_back(Diligent::float2(1, 1));
                            }
                            if (n6 == 0)
                            {
                                generate_face(vertices, x, y, z, FRONT);
                                textcoords.push_back(Diligent::float2(0, 1));
                                textcoords.push_back(Diligent::float2(0, 0));
                                textcoords.push_back(Diligent::float2(1, 0));
                                textcoords.push_back(Diligent::float2(1, 1));
                            }
                        }
                    }
                }
            }

            VERTEX_DATA chunk_vertex_data;
            chunk_vertex_data.positions = vertices;
            chunk_vertex_data.texcoords = textcoords;

            return chunk_vertex_data;
        }
    }
}