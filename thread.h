#ifndef THREAD_H
#define THREAD_H

#include <QtCore>

class Thread : public QThread
{
private:
    void run()
    {
        //qDebug()<<"From worker thread: "<<currentThreadId();
        python_run();
    }

    void python_run();
};

#endif // THREAD_H






