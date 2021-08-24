#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QHostAddress>

// 파일입출력
#include <QString> // 파일 입출력
#include <QFile> // 파일 입출력
#include <QTextStream> // 파일 입출력
#include <QFileDialog> // 프로젝트 경로 가져오기


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _socket(this)
{
    ui->setupUi(this);

    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(&_socket, SIGNAL(connected()), this, SLOT(openConnectServer()));
    connect(&_socket, SIGNAL(disconnected()), this, SLOT(closeConnectServer()));
    connect(&_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

    connect(ui->motorButton, SIGNAL(clicked()), this, SLOT(targetSend()));
    connect(ui->controlstartButton_motor, SIGNAL(clicked()), this, SLOT(controlStart()));
    connect(ui->controlstopButton_motor, SIGNAL(clicked()), this, SLOT(controlStop()));
    connect(ui->SetTargetButton1, SIGNAL(clicked()), this, SLOT(SetTargetSend()));
    connect(ui->SetTargetButton2, SIGNAL(clicked()), this, SLOT(SetTargetSend()));
    connect(ui->SetTargetButton3, SIGNAL(clicked()), this, SLOT(SetTargetSend()));
    connect(ui->SetTargetButton4, SIGNAL(clicked()), this, SLOT(SetTargetSend()));
    connect(ui->SetTargetButton5, SIGNAL(clicked()), this, SLOT(SetTargetSend()));

    connect(ui->sensorButton, SIGNAL(clicked()), this, SLOT(sensorButton_click()));
    connect(ui->sensorButton_start, SIGNAL(clicked()), this, SLOT(sensorStart()));
    connect(ui->sensorButton_stop, SIGNAL(clicked()), this, SLOT(sensorStop()));

    connect(ui->allStartButton, SIGNAL(clicked()), this, SLOT(allStart()));
    connect(ui->allStopButton, SIGNAL(clicked()), this, SLOT(allStop()));

    connect(ui->setgainButton, SIGNAL(clicked()), this, SLOT(setGain()));

    // Motor Thread
    motorThread = new socket_Thread(this);
    connect(motorThread, SIGNAL(ThreadTick()), this, SLOT(Threadrun_motor()));

    // Sensor Thread
    sensorThread = new socket_Thread(this);
    connect(sensorThread, SIGNAL(ThreadTick()), this, SLOT(Threadrun_sensor()));

    // All Thread
    allThread = new socket_Thread(this);
    connect(allThread, SIGNAL(ThreadTick()), this, SLOT(Threadrun_all()));

    // 파일에서 Gain값 읽어오기 (초기에 없다면 기본값으로 생성해줌)
    QString filePath = QDir::currentPath() + "/PGainFile.txt";
    fileRead(filePath);

    // 처음에 gain값 띄워주기
    gainShow();
}

MainWindow::~MainWindow()
{
    delete ui;

    motorThread->stop();
    sensorThread->stop();
    allThread->stop();
}

// 소켓 연결
void MainWindow::connectToServer(){
    // 서버 연결
    if(connectFlag == false){
        QString m_addr, m_port;
        m_addr = ui->addressEdit->text();
        m_port = ui->portEdit->text();

        if(m_addr == "" || m_port == ""){
            ui->statusLabel->setText("주소 또는 포트를 입력하세요.");
            return;
        }

        _socket.connectToHost(QHostAddress(m_addr), m_port.toUInt(NULL,10));

        if(!_socket.waitForConnected()){
            ui->statusLabel->setText("connect fail");
        }
    }
    // 서버 연결 끊기
    else{
        motorThread->stop();
        sensorThread->stop();
        allThread->stop();
        _socket.disconnectFromHost();
        return;
    }

}
void MainWindow::openConnectServer(){
    ui->statusLabel->setText("connect complete");
    ui->connectButton->setText("접속 종료");
    connectFlag = true;

    ui->groupBox_GetPosition->setEnabled(true);
    ui->groupBox_SensorData->setEnabled(true);
    ui->groupBox_SetPosition->setEnabled(true);
    ui->groupBox_SetTarget->setDisabled(true);
    ui->motorButton->setDisabled(true);
    ui->controlstartButton_motor->setEnabled(true);
    ui->controlstopButton_motor->setDisabled(true);
    ui->sensorButton_stop->setDisabled(true);
    ui->addressEdit->setDisabled(true);
    ui->portEdit->setDisabled(true);
    ui->allStartButton->setEnabled(true);
    ui->allStopButton->setDisabled(true);
    ui->groupBox_SetGain->setEnabled(true);
}
void MainWindow::closeConnectServer(){
    _socket.close();

    ui->statusLabel->setText("");
    ui->connectButton->setText("접속 하기");
    connectFlag = false;

    ui->groupBox_GetPosition->setDisabled(true);
    ui->groupBox_SensorData->setDisabled(true);
    ui->groupBox_SetPosition->setDisabled(true);
    ui->groupBox_SetTarget->setDisabled(true);
    ui->addressEdit->setEnabled(true);
    ui->portEdit->setEnabled(true);
    ui->allStartButton->setDisabled(true);
    ui->allStopButton->setDisabled(true);
    ui->groupBox_SetGain->setDisabled(true);
}

// set position 보내기 버튼 클릭 - 타켓값 변경
void MainWindow::targetSend(){
    ui->statusLabel->setText("");

    bool ok;
    int buffer;

    buffer = ui->lineEdit_SP_F1_1->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : T-1");}
    else if(ok == true){ T_target[0] = buffer; }
    buffer = ui->lineEdit_SP_F1_2->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : T-2");}
    else if(ok == true){ T_target[1] = buffer; }
    buffer = ui->lineEdit_SP_F1_3->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : T-3");}
    else if(ok == true){ T_target[2] = buffer; }
    buffer = ui->lineEdit_SP_F1_4->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : T-4");}
    else if(ok == true){ T_target[3] = buffer; }

    buffer = ui->lineEdit_SP_F2_1->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : I-1");}
    else if(ok == true){ I_target[0] = buffer; }
    buffer = ui->lineEdit_SP_F2_2->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : I-2");}
    else if(ok == true){ I_target[1] = buffer; }
    buffer = ui->lineEdit_SP_F2_3->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : I-3");}
    else if(ok == true){ I_target[2] = buffer; }
    buffer = ui->lineEdit_SP_F2_4->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : I-4");}
    else if(ok == true){ I_target[3] = buffer; }

    buffer = ui->lineEdit_SP_F3_1->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : M-1");}
    else if(ok == true){ M_target[0] = buffer; }
    buffer = ui->lineEdit_SP_F3_2->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : M-2");}
    else if(ok == true){ M_target[1] = buffer; }
    buffer = ui->lineEdit_SP_F3_3->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : M-3");}
    else if(ok == true){ M_target[2] = buffer; }
    buffer = ui->lineEdit_SP_F3_4->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : M-4");}
    else if(ok == true){ M_target[3] = buffer; }

    buffer = ui->lineEdit_SP_F4_1->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : R-1");}
    else if(ok == true){ R_target[0] = buffer; }
    buffer = ui->lineEdit_SP_F4_2->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : R-2");}
    else if(ok == true){ R_target[1] = buffer; }
    buffer = ui->lineEdit_SP_F4_3->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : R-3");}
    else if(ok == true){ R_target[2] = buffer; }
    buffer = ui->lineEdit_SP_F4_4->text().toUInt(&ok,10);
    if(buffer < 0 || 65534 < buffer){ ui->statusLabel->setText("err : R-4");}
    else if(ok == true){ R_target[3] = buffer; }
}

// 고정 타켓값 보내기
void MainWindow::SetTargetSend(){
    ui->statusLabel->setText("");

    QObject *obj = sender();

    if(obj->objectName() == "SetTargetButton1"){
        T_target[3] = 20816; // DIP
        T_target[2] = 16158; // PIP
        T_target[1] = 25542; // MCP
        T_target[0] = 43629; // A/A

        I_target[3] = 52519;
        I_target[2] = 11713;
        I_target[1] = 10521;
        I_target[0] = 28258;

        M_target[3] = 32768;
        M_target[2] = 32768;
        M_target[1] = 32768;
        M_target[0] = 32768;

        R_target[3] = 32768;
        R_target[2] = 32768;
        R_target[1] = 32768;
        R_target[0] = 32768;
    }
    else if(obj->objectName() == "SetTargetButton2"){
        T_target[3] = 27553;
        T_target[2] = 11541;
        T_target[1] = 19166;
        T_target[0] = 48580;

        I_target[3] = 32768;
        I_target[2] = 32768;
        I_target[1] = 32768;
        I_target[0] = 32768;

        M_target[3] = 50212;
        M_target[2] = 13963;
        M_target[1] = 11434;
        M_target[0] = 32768;

        R_target[3] = 32768;
        R_target[2] = 32768;
        R_target[1] = 32768;
        R_target[0] = 32768;
    }
    else if(obj->objectName() == "SetTargetButton3"){
        T_target[3] = 28783;
        T_target[2] = 10337;
        T_target[1] = 13539;
        T_target[0] = 56723;

        I_target[3] = 32768;
        I_target[2] = 32768;
        I_target[1] = 32768;
        I_target[0] = 32768;

        M_target[3] = 32768;
        M_target[2] = 32768;
        M_target[1] = 32768;
        M_target[0] = 32768;

        R_target[3] = 50000;
        R_target[2] = 13000;
        R_target[1] = 18108;
        R_target[0] = 40642;
    }
    else if(obj->objectName() == "SetTargetButton4"){
        T_target[3] = 10000;
        T_target[2] = 10000;
        T_target[1] = 20194;
        T_target[0] = 55633;

        I_target[3] = 50000;
        I_target[2] = 10000;
        I_target[1] = 10000;
        I_target[0] = 32768;

        M_target[3] = 50000;
        M_target[2] = 10000;
        M_target[1] = 10000;
        M_target[0] = 32768;

        R_target[3] = 50000;
        R_target[2] = 10000;
        R_target[1] = 10000;
        R_target[0] = 42768;
    }
    else if(obj->objectName() == "SetTargetButton5"){
        T_target[3] = 32768;
        T_target[2] = 32768;
        T_target[1] = 32768;
        T_target[0] = 32768;

        I_target[3] = 32768;
        I_target[2] = 32768;
        I_target[1] = 32768;
        I_target[0] = 32768;

        M_target[3] = 32768;
        M_target[2] = 32768;
        M_target[1] = 32768;
        M_target[0] = 32768;

        R_target[3] = 32768;
        R_target[2] = 32768;
        R_target[1] = 32768;
        R_target[0] = 32768;
    }

    // Thumb
    ui->lineEdit_SP_F1_4->setText(QString::number(T_target[3])); // DIP
    ui->lineEdit_SP_F1_3->setText(QString::number(T_target[2])); // PIP
    ui->lineEdit_SP_F1_2->setText(QString::number(T_target[1])); // MCP
    ui->lineEdit_SP_F1_1->setText(QString::number(T_target[0])); // A/A

    // Index
    ui->lineEdit_SP_F2_4->setText(QString::number(I_target[3]));
    ui->lineEdit_SP_F2_3->setText(QString::number(I_target[2]));
    ui->lineEdit_SP_F2_2->setText(QString::number(I_target[1]));
    ui->lineEdit_SP_F2_1->setText(QString::number(I_target[0]));

    // Middle
    ui->lineEdit_SP_F3_4->setText(QString::number(M_target[3]));
    ui->lineEdit_SP_F3_3->setText(QString::number(M_target[2]));
    ui->lineEdit_SP_F3_2->setText(QString::number(M_target[1]));
    ui->lineEdit_SP_F3_1->setText(QString::number(M_target[0]));

    // Ring
    ui->lineEdit_SP_F4_4->setText(QString::number(R_target[3]));
    ui->lineEdit_SP_F4_3->setText(QString::number(R_target[2]));
    ui->lineEdit_SP_F4_2->setText(QString::number(R_target[1]));
    ui->lineEdit_SP_F4_1->setText(QString::number(R_target[0]));
}

// 제어 시작 버튼 - Start motorThread
void MainWindow::controlStart(){
    ui->controlstartButton_motor->setDisabled(true);
    ui->motorButton->setEnabled(true);
    ui->controlstopButton_motor->setEnabled(true);
    ui->groupBox_SensorData->setDisabled(true);
    ui->groupBox_SetTarget->setEnabled(true);
    ui->allStartButton->setDisabled(true);

    dataDivide_flag = 0; // 데이터 수신 시 모터값으로 구분. (1: 센서값)
    rxSuccess_flag = false;
    prePos_flag = false;

    QByteArray sendData_hex8;
    sendData_hex8.resize(33);

    sendData_hex8[0] = 0x00;
    for(int i=0 ; i<(sendData_hex8.size()-1)/2 ; i++){
        sendData_hex8[i*2+1] = 0x80;
        sendData_hex8[i*2+2] = 0x00;
    }
    _socket.write(sendData_hex8);

    motorThread->start();
}

// 제어 종료 버튼 - Stop motorThread
void MainWindow::controlStop(){
    ui->controlstartButton_motor->setEnabled(true);
    ui->motorButton->setDisabled(true);
    ui->controlstopButton_motor->setDisabled(true);
    ui->groupBox_SensorData->setEnabled(true);
    ui->groupBox_SetTarget->setDisabled(true);
    ui->allStartButton->setEnabled(true);

    motorThread->stop();
}

// 모터 제어 스레드
void MainWindow::Threadrun_motor(){

    if(rxSuccess_flag == true){
        rxSuccess_flag = false;
#if 1
        if(prePos_flag == false){
            prePos_flag = true;
            for(int i=0 ; i<4 ; i++){
                preT_pos[i] = T_pos[i];
                preI_pos[i] = I_pos[i];
                preM_pos[i] = M_pos[i];
                preR_pos[i] = R_pos[i];
            }
        }

        //Calculate Err
        for(int i=0; i<4; i++){
            T_Err[i] = T_target[i] - T_pos[i];
            I_Err[i] = I_target[i] - I_pos[i];
            M_Err[i] = M_target[i] - M_pos[i];
            R_Err[i] = R_target[i] - R_pos[i];
        }

        //P control
        for(int i=0; i<4; i++){
            T_torque[i] = (float)(T_kp[i]*(T_Err[i]))+offset; //offset = 0x8000
            I_torque[i] = (float)(I_kp[i]*(I_Err[i]))+offset;
            M_torque[i] = (float)(M_kp[i]*(M_Err[i]))+offset;
            R_torque[i] = (float)(R_kp[i]*(R_Err[i]))+offset;

            if(T_torque[i]<0) T_torque[i] = 0;
            if(I_torque[i]<0) I_torque[i] = 0;
            if(M_torque[i]<0) M_torque[i] = 0;
            if(R_torque[i]<0) R_torque[i] = 0;

            if(T_torque[i]>65534) T_torque[i] = 65534;
            if(I_torque[i]>65534) I_torque[i] = 65534;
            if(M_torque[i]>65534) M_torque[i] = 65534;
            if(R_torque[i]>65534) R_torque[i] = 65534;
        }

        for(int i=0 ; i<4 ; i++){
            preT_pos[i] = T_pos[i];
            preI_pos[i] = I_pos[i];
            preM_pos[i] = M_pos[i];
            preR_pos[i] = R_pos[i];
        }

        QByteArray TIMR_Duty;
        TIMR_Duty.resize(33);

        TIMR_Duty[0] = 0x00; // ID
        for(int i=0; i<4; i++){
            TIMR_Duty[i*2+1] = ((int)T_torque[i] >> 8) & 0x00FF;
            TIMR_Duty[i*2+2] = (int)T_torque[i] & 0x00FF;

            TIMR_Duty[i*2+9] = ((int)I_torque[i] >> 8) & 0x00FF;
            TIMR_Duty[i*2+10] = (int)I_torque[i] & 0x00FF;

            TIMR_Duty[i*2+17] = ((int)M_torque[i] >> 8) & 0x00FF;
            TIMR_Duty[i*2+18] = (int)M_torque[i] & 0x00FF;

            TIMR_Duty[i*2+25] = ((int)R_torque[i] >> 8) & 0x00FF;
            TIMR_Duty[i*2+26] = (int)R_torque[i] & 0x00FF;
        }

#endif

#if 0 //duty만 보내기
        QByteArray TIMR_Duty;
        TIMR_Duty.resize(33);
        TIMR_Duty[0] = 0x00; // ID

        for(int i=0 ; i<4 ; i++){
            TIMR_Duty[i*2+1] = ((int)T_target[i] >> 8) & 0x00FF;
            TIMR_Duty[i*2+2] = (int)T_target[i] & 0x00FF;

            TIMR_Duty[i*2+9] = ((int)I_target[i] >> 8) & 0x00FF;
            TIMR_Duty[i*2+10] = (int)I_target[i] & 0x00FF;

            TIMR_Duty[i*2+17] = ((int)M_target[i] >> 8) & 0x00FF;
            TIMR_Duty[i*2+18] = (int)M_target[i] & 0x00FF;

            TIMR_Duty[i*2+25] = ((int)R_target[i] >> 8) & 0x00FF;
            TIMR_Duty[i*2+26] = (int)R_target[i] & 0x00FF;
        }
#endif

        _socket.write(TIMR_Duty);
    }
}

// 센서 데이터 받기 버튼
void MainWindow::sensorButton_click(){
    ui->statusLabel->setText("");

    QByteArray sendData;
    sendData.resize(1);

    sendData[0] = 0x03;
    _socket.write(sendData);

    dataDivide_flag = 1;
}

// 센서 데이터 연속 받기 버튼 - Start sensorThread
void MainWindow::sensorStart(){
    ui->groupBox_SetPosition->setDisabled(true);
    ui->groupBox_GetPosition->setDisabled(true);
    ui->sensorButton->setDisabled(true);
    ui->sensorButton_start->setDisabled(true);
    ui->sensorButton_stop->setEnabled(true);
    ui->allStartButton->setDisabled(true);

    rxSuccess_flag = false;
    dataDivide_flag = 1;

    QByteArray sendData;
    sendData.resize(1);

    sendData[0] = 0x03;
    _socket.write(sendData);

    sensorThread->start();
}

// 센서 데이터 연속 받기 종료 버튼 - Stop sensorThread
void MainWindow::sensorStop(){
    ui->groupBox_SetPosition->setEnabled(true);
    ui->groupBox_GetPosition->setEnabled(true);
    ui->sensorButton->setEnabled(true);
    ui->sensorButton_start->setEnabled(true);
    ui->sensorButton_stop->setDisabled(true);
    ui->allStartButton->setEnabled(true);

    sensorThread->stop();
}

// 센서 데이터받기 스레드
void MainWindow::Threadrun_sensor(){
    if(rxSuccess_flag == true){
        rxSuccess_flag = false;

        QByteArray sendData;
        sendData.resize(1);

        sendData[0] = 0x03;
        _socket.write(sendData);
    }
}

// 전체 데이터 받기 버튼 - Start allThread
void MainWindow::allStart(){
    ui->groupBox_SetTarget->setEnabled(true);
    ui->motorButton->setEnabled(true);
    ui->controlstartButton_motor->setDisabled(true);
    ui->controlstopButton_motor->setDisabled(true);
    ui->sensorButton->setDisabled(true);
    ui->sensorButton_start->setDisabled(true);
    ui->sensorButton_stop->setDisabled(true);
    ui->allStartButton->setDisabled(true);

    ui->allStopButton->setEnabled(true);

    // 처음엔 모터 데이터부터 받는다. 모터->센서->모터 순서
    dataDivide_flag = 0; // 데이터 수신 시 모터값으로 구분. (1: 센서값)
    rxSuccess_flag = false;
    prePos_flag = false;

    QByteArray sendData_hex8;
    sendData_hex8.resize(33);
    sendData_hex8[0] = 0x00;
    for(int i=0 ; i<(sendData_hex8.size()-1)/2 ; i++){
        sendData_hex8[i*2+1] = 0x80;
        sendData_hex8[i*2+2] = 0x00;
    }
    _socket.write(sendData_hex8);

    allThread->start();
}

// 전체 데이터 받기 종료 버튼 - Stop allThread
void MainWindow::allStop(){
    ui->groupBox_SetTarget->setDisabled(true);
    ui->motorButton->setDisabled(true);
    ui->controlstartButton_motor->setEnabled(true);
    ui->controlstopButton_motor->setDisabled(true);
    ui->sensorButton->setEnabled(true);
    ui->sensorButton_start->setEnabled(true);
    ui->sensorButton_stop->setDisabled(true);
    ui->allStartButton->setEnabled(true);

    ui->allStopButton->setDisabled(true);

    allThread->stop();
}

// Run allThrea
void MainWindow::Threadrun_all(){
    if(rxSuccess_flag == true){
        rxSuccess_flag = false;

        if(dataDivide_flag == 0){
            if(prePos_flag == false){
                prePos_flag = true;
                for(int i=0 ; i<4 ; i++){
                    preT_pos[i] = T_pos[i];
                    preI_pos[i] = I_pos[i];
                    preM_pos[i] = M_pos[i];
                    preR_pos[i] = R_pos[i];
                }
            }

            //Calculate Err
            for(int i=0; i<4; i++){
                T_Err[i] = T_target[i] - T_pos[i];
                I_Err[i] = I_target[i] - I_pos[i];
                M_Err[i] = M_target[i] - M_pos[i];
                R_Err[i] = R_target[i] - R_pos[i];
            }

            //P control
            for(int i=0; i<4; i++){
                T_torque[i] = (float)(T_kp[i]*(T_Err[i]))+offset; //offset = 0x8000
                I_torque[i] = (float)(I_kp[i]*(I_Err[i]))+offset;
                M_torque[i] = (float)(M_kp[i]*(M_Err[i]))+offset;
                R_torque[i] = (float)(R_kp[i]*(R_Err[i]))+offset;

                if(T_torque[i]<0) T_torque[i] = 0;
                if(I_torque[i]<0) I_torque[i] = 0;
                if(M_torque[i]<0) M_torque[i] = 0;
                if(R_torque[i]<0) R_torque[i] = 0;

                if(T_torque[i]>65534) T_torque[i] = 65534;
                if(I_torque[i]>65534) I_torque[i] = 65534;
                if(M_torque[i]>65534) M_torque[i] = 65534;
                if(R_torque[i]>65534) R_torque[i] = 65534;
            }

            for(int i=0 ; i<4 ; i++){
                preT_pos[i] = T_pos[i];
                preI_pos[i] = I_pos[i];
                preM_pos[i] = M_pos[i];
                preR_pos[i] = R_pos[i];
            }

            dataDivide_flag = 1;

            QByteArray sendData;
            sendData.resize(1);

            sendData[0] = 0x03;
            _socket.write(sendData);
        }
        else if(dataDivide_flag == 1){ // 센서값을 받았으면 다시 모터값 받기
            dataDivide_flag = 0;

            QByteArray TIMR_Duty;
            TIMR_Duty.resize(33);

            TIMR_Duty[0] = 0x00; // ID
            for(int i=0; i<4; i++){
                TIMR_Duty[i*2+1] = ((int)T_torque[i] >> 8) & 0x00FF;
                TIMR_Duty[i*2+2] = (int)T_torque[i] & 0x00FF;

                TIMR_Duty[i*2+9] = ((int)I_torque[i] >> 8) & 0x00FF;
                TIMR_Duty[i*2+10] = (int)I_torque[i] & 0x00FF;

                TIMR_Duty[i*2+17] = ((int)M_torque[i] >> 8) & 0x00FF;
                TIMR_Duty[i*2+18] = (int)M_torque[i] & 0x00FF;

                TIMR_Duty[i*2+25] = ((int)R_torque[i] >> 8) & 0x00FF;
                TIMR_Duty[i*2+26] = (int)R_torque[i] & 0x00FF;
            }

            _socket.write(TIMR_Duty);
        }
    }
}

// 파일 쓰기
void MainWindow::fileWrite(QString filePath)
{
    QFile file(filePath);
    // Trying to open in WriteOnly and Text mode
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        //qDebug() << " Could not open file for writing";
        return;
    }

    QTextStream out(&file);

    out << "[Thumb]\n";
    out << "DIP\t" << T_kp[3] << "\n";
    out << "PIP\t" << T_kp[2] << "\n";
    out << "MCP\t" << T_kp[1] << "\n";
    out << "AA\t" << T_kp[0] << "\n";

    out << "[Index]\n";
    out << "DIP\t" << I_kp[3] << "\n";
    out << "PIP\t" << I_kp[2] << "\n";
    out << "MCP\t" << I_kp[1] << "\n";
    out << "AA\t" << I_kp[0] << "\n";

    out << "[Middle]\n";
    out << "DIP\t" << M_kp[3] << "\n";
    out << "PIP\t" << M_kp[2] << "\n";
    out << "MCP\t" << M_kp[1] << "\n";
    out << "AA\t" << M_kp[0] << "\n";

    out << "[Ring]\n";
    out << "DIP\t" << R_kp[3] << "\n";
    out << "PIP\t" << R_kp[2] << "\n";
    out << "MCP\t" << R_kp[1] << "\n";
    out << "AA\t" << R_kp[0];

    file.flush();
    file.close();
}

// 파일 읽기
void MainWindow::fileRead(QString filePath)
{

    QFile file(filePath);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        //qDebug() << " Could not open the file for reading";
        fileWrite(filePath); // 읽을 때 파일이 없다면 생성
        return;
    }

    QTextStream in(&file);
    QString myText = in.readAll();

    QStringList list = myText.split("\n");

    for(int i=0 ; i<4 ; i++){
        QStringList list2 = list[i+1].split("\t");
        T_kp[3-i] = list2[1].toFloat();
    }

    for(int i=0 ; i<4 ; i++){
        QStringList list2 = list[i+6].split("\t");
        I_kp[3-i] = list2[1].toFloat();
    }

    for(int i=0 ; i<4 ; i++){
        QStringList list2 = list[i+11].split("\t");
        M_kp[3-i] = list2[1].toFloat();
    }

    for(int i=0 ; i<4 ; i++){
        QStringList list2 = list[i+16].split("\t");
        R_kp[3-i] = list2[1].toFloat();
    }

    file.close();
}

// gain값 설정
void MainWindow::setGain(){
    ui->statusLabel->setText("");

    bool ok;
    float buffer;

    buffer = ui->lineEdit_SG_F1_1->text().toFloat(&ok);
    if(ok == true){ T_kp[0] = buffer; }
    buffer = ui->lineEdit_SG_F1_2->text().toFloat(&ok);
    if(ok == true){ T_kp[1] = buffer; }
    buffer = ui->lineEdit_SG_F1_3->text().toFloat(&ok);
    if(ok == true){ T_kp[2] = buffer; }
    buffer = ui->lineEdit_SG_F1_4->text().toFloat(&ok);
    if(ok == true){ T_kp[3] = buffer; }

    buffer = ui->lineEdit_SG_F2_1->text().toFloat(&ok);
    if(ok == true){ I_kp[0] = buffer; }
    buffer = ui->lineEdit_SG_F2_2->text().toFloat(&ok);
    if(ok == true){ I_kp[1] = buffer; }
    buffer = ui->lineEdit_SG_F2_3->text().toFloat(&ok);
    if(ok == true){ I_kp[2] = buffer; }
    buffer = ui->lineEdit_SG_F2_4->text().toFloat(&ok);
    if(ok == true){ I_kp[3] = buffer; }

    buffer = ui->lineEdit_SG_F3_1->text().toFloat(&ok);
    if(ok == true){ M_kp[0] = buffer; }
    buffer = ui->lineEdit_SG_F3_2->text().toFloat(&ok);
    if(ok == true){ M_kp[1] = buffer; }
    buffer = ui->lineEdit_SG_F3_3->text().toFloat(&ok);
    if(ok == true){ M_kp[2] = buffer; }
    buffer = ui->lineEdit_SG_F3_4->text().toFloat(&ok);
    if(ok == true){ M_kp[3] = buffer; }

    buffer = ui->lineEdit_SG_F4_1->text().toFloat(&ok);
    if(ok == true){ R_kp[0] = buffer; }
    buffer = ui->lineEdit_SG_F4_2->text().toFloat(&ok);
    if(ok == true){ R_kp[1] = buffer; }
    buffer = ui->lineEdit_SG_F4_3->text().toFloat(&ok);
    if(ok == true){ R_kp[2] = buffer; }
    buffer = ui->lineEdit_SG_F4_4->text().toFloat(&ok);
    if(ok == true){ R_kp[3] = buffer; }

    QString filePath = QDir::currentPath() + "/PGainFile.txt";
    fileWrite(filePath);

    ui->statusLabel->setText("Complete");
}

// 처음에 gain값 띄워주기
void MainWindow::gainShow(){
    ui->lineEdit_SG_F1_1->setText(QString::number(T_kp[0]));
    ui->lineEdit_SG_F1_2->setText(QString::number(T_kp[1]));
    ui->lineEdit_SG_F1_3->setText(QString::number(T_kp[2]));
    ui->lineEdit_SG_F1_4->setText(QString::number(T_kp[3]));

    ui->lineEdit_SG_F2_1->setText(QString::number(I_kp[0]));
    ui->lineEdit_SG_F2_2->setText(QString::number(I_kp[1]));
    ui->lineEdit_SG_F2_3->setText(QString::number(I_kp[2]));
    ui->lineEdit_SG_F2_4->setText(QString::number(I_kp[3]));

    ui->lineEdit_SG_F3_1->setText(QString::number(M_kp[0]));
    ui->lineEdit_SG_F3_2->setText(QString::number(M_kp[1]));
    ui->lineEdit_SG_F3_3->setText(QString::number(M_kp[2]));
    ui->lineEdit_SG_F3_4->setText(QString::number(M_kp[3]));

    ui->lineEdit_SG_F4_1->setText(QString::number(R_kp[0]));
    ui->lineEdit_SG_F4_2->setText(QString::number(R_kp[1]));
    ui->lineEdit_SG_F4_3->setText(QString::number(R_kp[2]));
    ui->lineEdit_SG_F4_4->setText(QString::number(R_kp[3]));
}

// 데이터 수신 (모터 위치값 or 센서데이터)
void MainWindow::onReadyRead()
{
    QByteArray datas = _socket.readAll();
    //qDebug() << datas;
    //qDebug() << datas.size();

    QStringList dataStrList;

    if(dataDivide_flag == 0){
        for(int i=0 ; i<datas.size()/8 ; i++){ // 32/2/4 -> 32/8 -> 4
            T_pos[i] = ((datas[i*2] << 8) & 0xFF00) | (datas[i*2+1] & 0x00FF);
            dataStrList.append(QString::number(T_pos[i]));
        }

        for(int i=0 ; i<datas.size()/8 ; i++){ // 32/2/4 -> 32/8 -> 4
            I_pos[i] = ((datas[i*2+8] << 8) & 0xFF00) | (datas[i*2+9] & 0x00FF);
            dataStrList.append(QString::number(I_pos[i]));
        }

        for(int i=0 ; i<datas.size()/8 ; i++){ // 32/2/4 -> 32/8 -> 4
            M_pos[i] = ((datas[i*2+16] << 8) & 0xFF00) | (datas[i*2+17] & 0x00FF);
            dataStrList.append(QString::number(M_pos[i]));
        }

        for(int i=0 ; i<datas.size()/8 ; i++){ // 32/2/4 -> 32/8 -> 4
            R_pos[i] = ((datas[i*2+24] << 8) & 0xFF00) | (datas[i*2+25] & 0x00FF);
            dataStrList.append(QString::number(R_pos[i]));
        }

        ui->lineEdit_GP_F1_1->setText(dataStrList.at(0));
        ui->lineEdit_GP_F1_2->setText(dataStrList.at(1));
        ui->lineEdit_GP_F1_3->setText(dataStrList.at(2));
        ui->lineEdit_GP_F1_4->setText(dataStrList.at(3));

        ui->lineEdit_GP_F2_1->setText(dataStrList.at(4));
        ui->lineEdit_GP_F2_2->setText(dataStrList.at(5));
        ui->lineEdit_GP_F2_3->setText(dataStrList.at(6));
        ui->lineEdit_GP_F2_4->setText(dataStrList.at(7));

        ui->lineEdit_GP_F3_1->setText(dataStrList.at(8));
        ui->lineEdit_GP_F3_2->setText(dataStrList.at(9));
        ui->lineEdit_GP_F3_3->setText(dataStrList.at(10));
        ui->lineEdit_GP_F3_4->setText(dataStrList.at(11));

        ui->lineEdit_GP_F4_1->setText(dataStrList.at(12));
        ui->lineEdit_GP_F4_2->setText(dataStrList.at(13));
        ui->lineEdit_GP_F4_3->setText(dataStrList.at(14));
        ui->lineEdit_GP_F4_4->setText(dataStrList.at(15));
    }
    else if(dataDivide_flag == 1){
        for(int i=0 ; i<datas.size()/2 ; i++){
            dataStrList.append(QString::number(((datas[i*2] << 8) & 0xFF00) | (datas[i*2+1] & 0x00FF)));
        }

        ui->lineEdit_F1_1->setText(dataStrList.at(0));
        ui->lineEdit_F1_2->setText(dataStrList.at(1));
        ui->lineEdit_F1_3->setText(dataStrList.at(2));
        ui->lineEdit_F1_4->setText(dataStrList.at(3));
        ui->lineEdit_F1_5->setText(dataStrList.at(4));
        ui->lineEdit_F1_6->setText(dataStrList.at(5));
        ui->lineEdit_F1_7->setText(dataStrList.at(6));
        ui->lineEdit_F1_8->setText(dataStrList.at(7));
        ui->lineEdit_F1_9->setText(dataStrList.at(8));
        ui->lineEdit_F1_10->setText(dataStrList.at(9));
        ui->lineEdit_F1_11->setText(dataStrList.at(10));
        ui->lineEdit_F1_12->setText(dataStrList.at(11));
        ui->lineEdit_F1_13->setText(dataStrList.at(12));
        ui->lineEdit_F1_14->setText(dataStrList.at(13));

        ui->lineEdit_F2_1->setText(dataStrList.at(14));
        ui->lineEdit_F2_2->setText(dataStrList.at(15));
        ui->lineEdit_F2_3->setText(dataStrList.at(16));
        ui->lineEdit_F2_4->setText(dataStrList.at(17));
        ui->lineEdit_F2_5->setText(dataStrList.at(18));
        ui->lineEdit_F2_6->setText(dataStrList.at(19));
        ui->lineEdit_F2_7->setText(dataStrList.at(20));
        ui->lineEdit_F2_8->setText(dataStrList.at(21));
        ui->lineEdit_F2_9->setText(dataStrList.at(22));
        ui->lineEdit_F2_10->setText(dataStrList.at(23));
        ui->lineEdit_F2_11->setText(dataStrList.at(24));
        ui->lineEdit_F2_12->setText(dataStrList.at(25));
        ui->lineEdit_F2_13->setText(dataStrList.at(26));
        ui->lineEdit_F2_14->setText(dataStrList.at(27));

        ui->lineEdit_F3_1->setText(dataStrList.at(28));
        ui->lineEdit_F3_2->setText(dataStrList.at(29));
        ui->lineEdit_F3_3->setText(dataStrList.at(30));
        ui->lineEdit_F3_4->setText(dataStrList.at(31));
        ui->lineEdit_F3_5->setText(dataStrList.at(32));
        ui->lineEdit_F3_6->setText(dataStrList.at(33));
        ui->lineEdit_F3_7->setText(dataStrList.at(34));
        ui->lineEdit_F3_8->setText(dataStrList.at(35));
        ui->lineEdit_F3_9->setText(dataStrList.at(36));
        ui->lineEdit_F3_10->setText(dataStrList.at(37));
        ui->lineEdit_F3_11->setText(dataStrList.at(38));
        ui->lineEdit_F3_12->setText(dataStrList.at(39));
        ui->lineEdit_F3_13->setText(dataStrList.at(40));
        ui->lineEdit_F3_14->setText(dataStrList.at(41));

        ui->lineEdit_F4_1->setText(dataStrList.at(42));
        ui->lineEdit_F4_2->setText(dataStrList.at(43));
        ui->lineEdit_F4_3->setText(dataStrList.at(44));
        ui->lineEdit_F4_4->setText(dataStrList.at(45));
        ui->lineEdit_F4_5->setText(dataStrList.at(46));
        ui->lineEdit_F4_6->setText(dataStrList.at(47));
        ui->lineEdit_F4_7->setText(dataStrList.at(48));
        ui->lineEdit_F4_8->setText(dataStrList.at(49));
        ui->lineEdit_F4_9->setText(dataStrList.at(50));
        ui->lineEdit_F4_10->setText(dataStrList.at(51));
        ui->lineEdit_F4_11->setText(dataStrList.at(52));
        ui->lineEdit_F4_12->setText(dataStrList.at(53));
        ui->lineEdit_F4_13->setText(dataStrList.at(54));
        ui->lineEdit_F4_14->setText(dataStrList.at(55));
    }

    rxSuccess_flag = true;
}

