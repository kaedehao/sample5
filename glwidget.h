#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>

#include <optix.h>
#include "utility.h"

#include "sample1.h"
#include "sample2.h"
#include "sample5.h"

class Mouse;
class PinholeCamera;

//-----------------------------------------------------------------------------
//
// OpenGL display
//
//-----------------------------------------------------------------------------
class glWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit glWidget(QWidget* _parent = 0);
    ~glWidget();

    static Sample5* getScene(){ return m_scene; }

protected:
    void resizeGL(int width, int height);
    void initializeGL();
    void paintGL();

    void keyPressEvent( QKeyEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

    void timerEvent(QTimerEvent* _event);

    enum contDraw_E { CDNone=0, CDProgressive=1, CDAnimated=2, CDBenchmark=3, CDBenchmarkTimed=4 };

private:
    // Do the actual rendering to the display
    static void displayFrame();

    static void display();

    // Cleans up the rendering context and quits.  If there wasn't error cleaning up, the
    // return code is passed out, otherwise 2 is used as the return code.
    static void quit(int return_code=0);

private:
    GLfloat posx;
    GLfloat posy;
    Sample1* sample1Scene;
    Sample2* sample2Scene;

    // Draw text to screen at window pos x,y.  To make this public we will need to have
    // a public helper that caches the text for use in the display func
    static void drawText( const std::string& text, float x, float y, void* font );

    static PinholeCamera* m_camera;
    static Sample5*       m_scene;

    static double         m_last_frame_time;
    static unsigned int   m_last_frame_count;
    static unsigned int   m_frame_count;

    static bool           m_display_fps;
    static double         m_fps_update_threshold;
    static char           m_fps_text[32];
    static optix::float3  m_text_color;
    static optix::float3  m_text_shadow_color;

    static bool           m_print_mem_usage;

    static contDraw_E     m_cur_continuous_mode;

    static bool           m_display_frames;
    static bool           m_save_frames_to_file;
    static std::string    m_save_frames_basename;

    static unsigned int   m_texId;
    static bool           m_sRGB_supported;
    static bool           m_use_sRGB;

    static int            m_num_devices;

    static bool           m_enable_cpu_rendering; // enables CPU execution of OptiX programs
};

#endif // GLWIDGET_H
