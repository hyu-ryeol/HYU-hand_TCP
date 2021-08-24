#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QTcpSocket>

#include "socket_Thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // motor control
    bool rxSuccess_flag = false;
    bool prePos_flag = false;

    //[0]:A/A , [1]:MCP, [2]:PIP, [3]DIP
    float T_kp[4] = {2,2,2,2};
    float I_kp[4] = {2,2,2,2};
    float M_kp[4] = {2,2,2,2};
    float R_kp[4] = {2,2,2,2};
    uint16_t T_target[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    uint16_t I_target[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    uint16_t M_target[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    uint16_t R_target[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    volatile uint16_t T_pos[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    volatile uint16_t I_pos[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    volatile uint16_t M_pos[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    volatile uint16_t R_pos[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    uint16_t preT_pos[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    uint16_t preI_pos[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    uint16_t preM_pos[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    uint16_t preR_pos[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    int T_Err[4] = {0,0,0,0};
    int I_Err[4] = {0,0,0,0};
    int M_Err[4] = {0,0,0,0};
    int R_Err[4] = {0,0,0,0};
    float T_torque[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    float I_torque[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    float M_torque[4] = {0x8000, 0x8000, 0x8000, 0x8000};
    float R_torque[4] = {0x8000, 0x8000, 0x8000, 0x8000};

    uint16_t offset = 0x8000;

    //file
    void fileWrite(QString filePath);
    void fileRead(QString filePath);

    void gainShow();

private:
    Ui::MainWindow *ui;

    QTcpSocket  _socket;
    bool connectFlag = false;
    uint8_t dataDivide_flag = 0; // 0: motor , 1: sensor
    qint16 nextBlockSize = 0;

    socket_Thread *motorThread, *sensorThread, *allThread;


private slots:
    void connectToServer();
    void openConnectServer();
    void closeConnectServer();
    void onReadyRead();

    void targetSend();
    void controlStart();
    void controlStop();
    void SetTargetSend();

    void sensorButton_click();
    void sensorStart();
    void sensorStop();

    void Threadrun_motor();
    void Threadrun_sensor();

    void allStart();
    void allStop();
    void Threadrun_all();

    void setGain();

};
#endif // MAINWINDOW_H
