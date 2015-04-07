#include "glwidget.h"
#include <iostream>

#include "mouse.h"

#include <NsightHelper.h>

using namespace optix;

//----------------------------------------------------------------------------------------------------------------------
PinholeCamera* glWidget::m_camera               = 0;
Sample5*       glWidget::m_scene                = 0;

unsigned int   glWidget::m_frame_count          = 0;

bool           glWidget::m_display_frames       = true;
bool           glWidget::m_save_frames_to_file  = false;
std::string    glWidget::m_save_frames_basename = "";

unsigned int   glWidget::m_texId                = 0;
bool           glWidget::m_sRGB_supported       = false;
bool           glWidget::m_use_sRGB             = false;

int            glWidget::m_num_devices          = 0;

bool           glWidget::m_enable_cpu_rendering = false;
//----------------------------------------------------------------------------------------------------------------------

glWidget::glWidget(QWidget *parent) :
    QGLWidget(parent)
{
}

glWidget::~glWidget()
{
}

void glWidget::initializeGL()
{
#ifdef LINUX
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if(error != GLEW_OK){
        std::cerr<<"GLEW IS NOT OK!!! "<<std::endl;
    }
#endif
    glClearColor( 0.5f, 0.5f, 0.5f, 1.0f);
    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);
    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);

    // as re-size is not explicitly called we need to do this.
    glViewport(0,0,width(),height());

//    sample1Scene = new Sample1();
//    sample1Scene->setSize(512u, 512u);
//    sample1Scene->init();
//    sample1Scene->trace();

//      sample2Scene = new Sample2();
//      sample2Scene->setSize(512u, 384u);
//      sample2Scene->setNumBoxes( 6 );
//      sample2Scene->init();
//      sample2Scene->trace();

    m_scene = new Sample5();
    m_scene->setSize(1024u, 768u);

//    glEnable(GL_CULL_FACE);
//    glShadeModel(GL_SMOOTH);
//    glEnable(GL_LIGHTING);
//    glEnable(GL_LIGHT0);

//    static GLfloat lightPosition[4] = { 0, 0, 10, 1.0 };
//    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

      //startTimer(0);
}

void glWidget::resizeGL(int width, int height)
{
//    int side = qMin(width, height);
    glViewport( 0, 0, width, height );

//#ifdef QT_OPENGL_ES_1
//    glOrthof(-2, +2, -2, +2, 1.0, 15.0);
//#else
//    glOrtho(-2, +2, -2, +2, 1.0, 15.0);
//#endif
//    glMatrixMode(GL_MODELVIEW);
}

void glWidget::timerEvent(QTimerEvent *_event){
    updateGL();
}

void glWidget::paintGL()
{
    //glutSetWindowTitle(window_title);
    //glutReshapeWindow(width, height);
    m_scene->enableCPURendering(m_enable_cpu_rendering);
    m_scene->setNumDevices( m_num_devices );

    int buffer_width;
    int buffer_height;
    try {
      // Set up scene
      Sample5::InitialCameraData camera_data;
      m_scene->initScene( camera_data );

      //if( m_initial_window_width > 0 && m_initial_window_height > 0)
        //m_scene->resize( m_initial_window_width, m_initial_window_height );

      //if ( !m_camera_pose.empty() )
        //camera_data = Sample5::InitialCameraData( m_camera_pose );

      // Initialize camera according to scene params
      m_camera = new PinholeCamera( camera_data.eye,
                                    camera_data.lookat,
                                    camera_data.up,
                                    -1.0f, // hfov is ignored when using keep vertical
                                    camera_data.vfov,
                                    PinholeCamera::KeepVertical );

      Buffer buffer = m_scene->getOutputBuffer();
      RTsize buffer_width_rts, buffer_height_rts;
      buffer->getSize( buffer_width_rts, buffer_height_rts );
      buffer_width  = static_cast<int>(buffer_width_rts);
      buffer_height = static_cast<int>(buffer_height_rts);
      //m_mouse = new Mouse( m_camera, buffer_width, buffer_height );
    } catch( Exception& e ){
      //sutilReportError( e.getErrorString().c_str() );
        std::cout<<"Error Expection Caught # 1"<<std::endl;
      exit(2);
    }


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Initialize state
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, buffer_width, buffer_height);

    display();
}

void glWidget::display()
{
    bool display_requested = true;

      try {
        // render the scene
        float3 eye, U, V, W;
        m_camera->getEyeUVW( eye, U, V, W );
        // Don't be tempted to just start filling in the values outside of a constructor,
        // because if you add a parameter it's easy to forget to add it here.

        Sample5::RayGenCameraData camera_data( eye, U, V, W );

        {
          nvtx::ScopedRange r( "trace" );
          m_scene->trace( camera_data, display_requested );
        }

        // Always count rendered frames
        ++m_frame_count;

        if( display_requested && m_display_frames ) {
          // Only enable for debugging
          // glClearColor(1.0, 0.0, 0.0, 0.0);
          // glClear(GL_COLOR_BUFFER_BIT);

          nvtx::ScopedRange r( "displayFrame" );
          displayFrame();
        }
      } catch( Exception& e ){
        //sutilReportError( e.getErrorString().c_str() );
        std::cout<<"Error Expection Caught #2"<<std::endl;
        exit(2);
      }

}

void glWidget::displayFrame()
{
    //m_scene->setUseVBOBuffer(true);
    // Draw the resulting image
    GLboolean sRGB = GL_FALSE;
    if (m_use_sRGB && m_sRGB_supported) {
      glGetBooleanv( GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &sRGB );
      if (sRGB) {
        glEnable(GL_FRAMEBUFFER_SRGB_EXT);
      }
    }

    // Draw the resulting image
    Buffer buffer = m_scene->getOutputBuffer();
    //Buffer buffer = sample2Scene->getOutputBuffer();
    RTsize buffer_width_rts, buffer_height_rts;
    buffer->getSize( buffer_width_rts, buffer_height_rts );
    int buffer_width  = static_cast<int>(buffer_width_rts);
    int buffer_height = static_cast<int>(buffer_height_rts);
    RTformat buffer_format = buffer->getFormat();

    if( m_save_frames_to_file ) {
      static char fname[128];
      std::string basename = m_save_frames_basename.empty() ? "frame" : m_save_frames_basename;
      sprintf(fname, "%s_%05d.ppm", basename.c_str(), m_frame_count);
      //sutilDisplayFilePPM( fname, buffer->get() );
    }

    unsigned int vboId = 0;
    //if( m_scene->usesVBOBuffer() ){
      //vboId = buffer->getGLBOId();
      std::cout<<"here"<<std::endl;
    //}

    if (vboId)
    {
      if (!m_texId)
      {

        glGenTextures( 1, &m_texId );
        glBindTexture( GL_TEXTURE_2D, m_texId);

        // Change these to GL_LINEAR for super- or sub-sampling
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        // GL_CLAMP_TO_EDGE for linear filtering, not relevant for nearest.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture( GL_TEXTURE_2D, 0);
      }

      glBindTexture( GL_TEXTURE_2D, m_texId );

      // send pbo to texture
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vboId);

      RTsize elementSize = buffer->getElementSize();
      if      ((elementSize % 8) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
      else if ((elementSize % 4) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      else if ((elementSize % 2) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
      else                             glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      {
        nvtx::ScopedRange r( "glTexImage" );
        if(buffer_format == RT_FORMAT_UNSIGNED_BYTE4) {
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer_width, buffer_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
        } else if(buffer_format == RT_FORMAT_FLOAT4) {
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, buffer_width, buffer_height, 0, GL_RGBA, GL_FLOAT, 0);
        } else if(buffer_format == RT_FORMAT_FLOAT3) {
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F_ARB, buffer_width, buffer_height, 0, GL_RGB, GL_FLOAT, 0);
        } else if(buffer_format == RT_FORMAT_FLOAT) {
          glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE32F_ARB, buffer_width, buffer_height, 0, GL_LUMINANCE, GL_FLOAT, 0);
        } else {
          assert(0 && "Unknown buffer format");
        }
      }
      glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

      glEnable(GL_TEXTURE_2D);

      // Initialize offsets to pixel center sampling.

      float u = 0.5f/buffer_width;
      float v = 0.5f/buffer_height;

      glBegin(GL_QUADS);
      glTexCoord2f(u, v);
      glVertex2f(0.0f, 0.0f);
      glTexCoord2f(1.0f, v);
      glVertex2f(1.0f, 0.0f);
      glTexCoord2f(1.0f - u, 1.0f - v);
      glVertex2f(1.0f, 1.0f);
      glTexCoord2f(u, 1.0f - v);
      glVertex2f(0.0f, 1.0f);
      glEnd();

      glDisable(GL_TEXTURE_2D);
    } else {
      GLvoid* imageData = buffer->map();
      assert( imageData );

      GLenum gl_data_type = GL_FALSE;
      GLenum gl_format = GL_FALSE;

      switch (buffer_format) {
            case RT_FORMAT_UNSIGNED_BYTE4:
              gl_data_type = GL_UNSIGNED_BYTE;
              gl_format    = GL_BGRA;
              break;

            case RT_FORMAT_FLOAT:
              gl_data_type = GL_FLOAT;
              gl_format    = GL_LUMINANCE;
              break;

            case RT_FORMAT_FLOAT3:
              gl_data_type = GL_FLOAT;
              gl_format    = GL_RGB;
              break;

            case RT_FORMAT_FLOAT4:
              gl_data_type = GL_FLOAT;
              gl_format    = GL_RGBA;
              break;

            default:
              fprintf(stderr, "Unrecognized buffer data type or format.\n");
              exit(2);
              break;
      }

      RTsize elementSize = buffer->getElementSize();
      int align = 1;
      if      ((elementSize % 8) == 0) align = 8;
      else if ((elementSize % 4) == 0) align = 4;
      else if ((elementSize % 2) == 0) align = 2;
      glPixelStorei(GL_UNPACK_ALIGNMENT, align);

      NVTX_RangePushA("glDrawPixels");
      glDrawPixels( static_cast<GLsizei>( buffer_width ), static_cast<GLsizei>( buffer_height ),
        gl_format, gl_data_type, imageData);
      NVTX_RangePop();

      buffer->unmap();

    }
    if (m_use_sRGB && m_sRGB_supported && sRGB) {
      glDisable(GL_FRAMEBUFFER_SRGB_EXT);
    }
}
