#ifndef PAINT_CAMERA
#define PAINT_CAMERA

#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>

rtTextureSampler<float4, 2> camera_map;
rtDeclareVariable(float, paint_camera_scale, , ) = 0;

static __host__ __device__ inline void paint_camera( optix::Ray* ray )
{
    float theta = atan2f( ray->direction.x, ray->direction.z );
    float phi   = M_PIf * 0.5f -  acosf( ray->direction.y );
    float u     = (theta + M_PIf) * (0.5f * M_1_PIf);
    float v     = 0.5f * ( 1.0f + sin(phi) );

    float3 map  = make_float3( tex2D(camera_map, u, v) );

    uchar4 map_color = make_color( map );

    float3 map_data = make_float3( map_color.x,
                                   map_color.y,
                                   map_color.z );
    map_data /= 255.0f;;
//    rtPrintf( "Camera texture color: %d, %d, %d!\n", map_color.x, map_color.y, map_color.z );

    // Apply painting on ray direction
    ray->direction =  ray->direction + map * paint_camera_scale ;

}

#endif // PAINT_CAMERA

