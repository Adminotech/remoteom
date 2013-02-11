/****************************************************************************
**
** Copyright 2010 Adminotech Ltd
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
** http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
*****************************************************************************/

#ifndef REMOTEOM_H
#define REMOTEOM_H

#include <QMainWindow>
#include <QDomDocument>
#include <QDomElement>
#include <QTcpSocket>
#include <QAuthenticator>
#include <QTimer>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

class QAction;
class QMenu;
class QTcpSocket;

namespace Ui {
    class RemoteOM;
}

class RemoteOM : public QMainWindow {
    Q_OBJECT

public:
    RemoteOM(QWidget *parent = 0);
    ~RemoteOM();
    Ui::RemoteOM *ui;
    //void showCurrentSettings();
    QString currentSettings;

    void SetStartProfile(QString profile) {

        this->startProfile = profile;
    }

protected:
   void closeEvent(QCloseEvent *event);   


private:

    void createActions();

    //Socket

    void closeSocket();
    void setInfoText(QString text);
    void stopFaultBlinking();
    void stopWaitBlinking();
    void startFaultTimer();

    QAction *exitAct;
    QAction *configureAct;
    QTcpSocket *socket;
    QTimer *faultTimer;
    QTimer *waitTimer;    

    QString sent, infoText;    
    bool receiveFaults;
    bool authenticate;
    QByteArray faultBuffer;
    QByteArray authBuffer;
    QMap<uchar, QString> mapResponse;
    SettingsDialog *dialog;
    Ui_SettingsDialog *settings_dialog;    
    QString startProfile;

private slots:
    void connectSocket();
    void on_buttonPower_clicked();
    void on_buttonReset_clicked();
    void on_buttonFaults_clicked();
    void on_buttonOperate_clicked();
    void configure();
    void blinkFault();
    void blinkWait();
    void TimeOut();

    //Socket
    void readyRead();
    void sendData(QByteArray input);    
    void error(QAbstractSocket::SocketError socketError);
    void proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *auth);

};

#endif // REMOTEOM_H
