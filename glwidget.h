#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>

#include <optix.h>

#include "sample1.h"
#include "sample2.h"
#include "sample5.h"

class Mouse;
class PinholeCamera;

//-----------------------------------------------------------------------------
//
// OpenGL isplay
//
//-----------------------------------------------------------------------------
class glWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit glWidget(QWidget* _parent = 0);
    ~glWidget();

    void resizeGL(int width, int height);
    void initializeGL();
    void paintGL();
    void timerEvent(QTimerEvent* _event);

private:
    // Do the actual rendering to the display
    void displayFrame();

    void display();
    //void run();

    Sample1* sample1Scene;
    Sample2* sample2Scene;

private:
    static PinholeCamera* m_camera;
    static Sample5*       m_scene;

    static unsigned int   m_frame_count;

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
