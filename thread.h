#ifndef THREAD_H
#define THREAD_H

#include <QtCore>
#include <Python/Python.h>

class Thread : public QThread
{
public:
    void run()
    {
        //qDebug()<<"From worker thread: "<<currentThreadId();
        python_subscribe();
    }

    void python_subscribe();

    static void* python_retrieve_camera();
    static float camera_array[];

    static PyObject* globalDict;
    static PyObject* localDict;
};

#endif // THREAD_H






