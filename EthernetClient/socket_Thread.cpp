#include "socket_Thread.h"
#include "mainwindow.h"

int socket_Thread::m_stopFlag = false;

socket_Thread::socket_Thread(QObject *parent):
    QThread(parent)
{

}

void socket_Thread::run(){
    m_stopFlag = false;

    while(!m_stopFlag){
        emit ThreadTick();
        msleep(2); // control loop time 2ms -> 500Hz
    }

    socket_Thread::quit();
    socket_Thread::wait(5000);
}

void socket_Thread::stop(){
    m_stopFlag = true;
}
