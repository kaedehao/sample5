//
//  Sample5Scene.h
//  Optix
//
//  Created by Hao Luo on 3/13/15.
//  Copyright (c) 2015 Hao Luo. All rights reserved.
//

#ifndef SMAPLE5_H
#define SMAPLE5_H

#include <optixu/optixpp_namespace.h>
#include "utility.h"
#include <QImage>
#include <QKeyEvent>

using namespace optix;

class glWidget;

class Sample5Scene
{
protected:
    typedef float3 float3;
public:
    // Used to pass current camera info to the ray gen program at render time.
    // eye - Camera position
    // W   - Viewing direction.             length(W) -> focal distance
    // U   - Horizontal axis of view plane. length(U) -> view plane width at focal distance
    // V   - Vertical axis of view plane.   length(V) -> view plane height at focal distance
    struct RayGenCameraData
    {
      RayGenCameraData() {}
      RayGenCameraData( const float3& m_eye, const float3& m_U, const float3& m_V, const float3& m_W )
        : eye(m_eye), U(m_U), V(m_V), W(m_W) {}
      float3 eye;
      float3 U;
      float3 V;
      float3 W;
    };

    // Used to specify initial viewing parameters
    struct InitialCameraData
    {
      InitialCameraData() {}
      InitialCameraData( const std::string& camera_string );
      InitialCameraData( float3 m_eye, float3 m_lookat, float3 m_up, float  m_vfov )
        : eye(m_eye), lookat(m_lookat), up(m_up), vfov(m_vfov) {}

      float3 eye;
      float3 lookat;
      float3 up;
      float  vfov;
    };

    Sample5Scene();
    ~Sample5Scene();

    void  signalCameraChanged() { m_camera_changed = true; }

    void  setNumDevices( int ndev );
    void  enableCPURendering(bool enable);

    void  setUseVBOBuffer( bool onoff ) { m_use_vbo_buffer = onoff; }
    bool  usesVBOBuffer() { return m_use_vbo_buffer; }

    //From SampleScene
    void initScene( InitialCameraData &camera_data );
    void trace( const RayGenCameraData &camera_data );
    void trace( const RayGenCameraData& camera_data, bool& display );

    // Return the output buffer to be displayed
    Buffer getOutputBuffer();

    bool keyPressEvent( int key );

    void setAdaptiveAA( bool adaptive_aa ) { m_adaptive_aa = adaptive_aa; }
    bool adaptive_aa(){ return m_adaptive_aa; }

    // This cleans up the Context.  If you override it, you should call
    // SampleScene::cleanUp() explicitly.
    void cleanUp();

    // Will resize the output buffer (which might use a VBO) then call doResize.
    // Override this if you want to handle ALL buffer resizes yourself.
    void   resize(unsigned int width, unsigned int height);

    // Accessor
    Context& getContext() { return m_context; }

    // Scene API
    void updateGeometry( float radius );
    void updateMaterial( float refraction_index );
    void updateLights( int index, float position );
    void updateAcceleration( bool accel );
    void updateAntialiasing( bool aa ) { setAdaptiveAA( aa ); }
    void updatePaintCamera( float scale );

private:
    int getEntryPoint() { return m_adaptive_aa ? AdaptivePinhole: Pinhole; }
    void genRndSeeds( unsigned int width, unsigned int height );

    enum {
        Pinhole = 0,
        AdaptivePinhole = 1
    };

    void createGeometry();

    Buffer       m_rnd_seeds;
    unsigned int m_frame_number;
    bool         m_adaptive_aa;

    unsigned int m_width;
    unsigned int m_height;

    //Buffer m_outputBuffer;
    static const char * const ptxpath( const std::string& target, const std::string& base );
    std::string texpath( const std::string& base );
    std::string texture_path;

protected:
    Buffer createOutputBuffer(RTformat format, unsigned int width, unsigned int height);

    Context m_context;

    bool   m_camera_changed;
    bool   m_use_vbo_buffer;
    int    m_num_devices;
    bool   m_cpu_rendering_enabled;

private:
  // Checks to see if CPU mode has been enabled and sets the appropriate flags.
  void updateCPUMode();
};
#endif // SMAPLE5_H
