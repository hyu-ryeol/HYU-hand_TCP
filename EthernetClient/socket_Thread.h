#ifndef SOCKET_THREAD_H
#define SOCKET_THREAD_H

#include <QThread>
#include <unistd.h>

class socket_Thread:public QThread
{
    Q_OBJECT
public:
    explicit socket_Thread(QObject *parent = 0);
    void stop();
    static int m_stopFlag;

private:
    void run();

signals:
    void ThreadTick();
};

#endif // SOCKET_THREAD_H
