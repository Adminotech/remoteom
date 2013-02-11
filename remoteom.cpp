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


#include <QtGui>
#include <QTimer>

#include "remoteom.h"
#include "ui_remoteom.h"
#include "settings.h"

RemoteOM::RemoteOM(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RemoteOM)
{

    mapResponse[48] = "No fault";
    mapResponse[49] = "IP. Plate current too high.";
    mapResponse[50] = "HV. Plate/anode voltage too low.";
    mapResponse[51] = "SWR. Excessive reflected power.";
    mapResponse[52] = "Ig2. Screen limit exceeded.";
    mapResponse[53] = "Overdrive. Too much drive from exciter.";
    mapResponse[54] = "FWD=0, Power output is zero with drive power higher than zero.";
    mapResponse[55] = "Igmax. 1st grid current limit exceeded.";
    mapResponse[56] = "Tune. Insufficient tuning - retune PA";
    mapResponse[62] = "PA turned on.";
    mapResponse[60] = "PA turned off.";
    mapResponse[83] = "PA turned to stand by mode.";
    mapResponse[79] = "PA turned to operating mode.";
    mapResponse[76] = "List 20 last fault codes.";
    mapResponse[71] = "Reset PA succeeded.";
    mapResponse[87] = "PA is heating up. Please wait.";
    mapResponse[90] = "Operating is not possible.";
    mapResponse[84] = "Fatal fault.";

    dialog = new SettingsDialog();

    ui->setupUi(this);
    createActions();

    //Init settings if startprofile is empty
    if (startProfile.isEmpty())
    {
        dialog->FirstChoice(startProfile.remove("\""));
        startProfile = "";
    }

    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(TimeOut()));


    ui->menu->addAction(configureAct);
    ui->menu->addAction(exitAct);
    ui->ledDrive->hide();
    ui->ledGrid->hide();
    ui->ledOperate->hide();
    ui->ledPA->hide();
    ui->ledPower->hide();
    ui->ledStdby->hide();
    ui->ledSwr->hide();
    ui->ledTune->hide();
    ui->ledFault->hide();

    connect(ui->buttonConnect, SIGNAL(clicked()), this, SLOT(connectSocket()));
    socket = new QTcpSocket(this);
    ui->buttonPower->setStatusTip(tr("Turn PA ON/OFF."));
    ui->buttonOperate->setStatusTip(tr("Turn Operate ON/OFF."));
    ui->buttonConnect->setStatusTip(tr("Connect/Disconnect to Remote OM"));

    setInfoText("Connecting to Remote OM. Please wait.");

    receiveFaults = false;

    faultTimer = new QTimer(this);
    waitTimer = new QTimer(this);
    connect(faultTimer, SIGNAL(timeout()), this, SLOT(blinkFault()));
    connect(waitTimer, SIGNAL(timeout()), this, SLOT(blinkWait()));

    //Get settings
    if (!startProfile.isEmpty())
    {
       /* dialog->SetInitialSettings();
        QString show_ip = dialog->set_ipaddress;
        QString show_port = dialog->set_port;
        ui->txtCurrentSettings->setText("Current settings: "+show_ip+":"+show_port);*/
    }

}

RemoteOM::~RemoteOM()
{
    closeSocket();
    delete ui;
}

void RemoteOM::closeEvent(QCloseEvent *event)
{
    closeSocket();
    event->accept();
}


void RemoteOM::createActions()
{
    exitAct = new QAction(tr("Exit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    configureAct = new QAction(tr("Settings"), this);
    configureAct->setStatusTip(tr("Settings"));
    connect(configureAct, SIGNAL(triggered()), this, SLOT(configure()));
}

void RemoteOM::configure()
{
    if (dialog)
    {        
        dialog->Init();
        dialog->show();
    }
    else
    {
        dialog = new SettingsDialog();
        dialog->show();
    }

}

void RemoteOM::setInfoText(QString text)
{

    if (infoText.length() == 0)
        infoText = text;

   if (infoText != text)
   {
       infoText = text;
       QString temp = ui->txtInfo->text();
       temp = text + "\n"+ temp;
       if (temp.length() > 200)
       {
           temp.remove(200, (temp.length() - 200));
       }
       ui->txtInfo->setText(temp);
   }

}


//Socket calls
void RemoteOM::on_buttonPower_clicked()
{
    //Try connect socket if not connected
    //qDebug() << "Socket state power button clicked " << socket->state();
    if (socket->state() != QAbstractSocket::ConnectedState)
    {
        connectSocket();

        if (socket && socket->state() == QAbstractSocket::ConnectedState)
        {
            setInfoText("Connected to Remote OM.");
            ui->ledPower->show();
        }
        else
        {
            setInfoText("The host was not found. Please check the host name and port settings.");
            return;
        }
    }

    //Turn PA on
    if(ui->ledPA->isHidden())
    {
        setInfoText("Turning PA on.");

        QByteArray temp;
        temp.append(62);
        sendData(temp);
        ui->ledPA->show();

    }
    else
    {
        setInfoText("Turning PA off.");

        QByteArray temp;
        temp.append(60);
        sendData(temp);
        ui->ledPA->hide();
    }

}
void RemoteOM::on_buttonOperate_clicked()
{

    //Turn to opearate mode
    if (ui->ledOperate->isHidden())
    {
        setInfoText("Turn to operating mode.");

        QByteArray temp;
        temp.append(79);
        sendData(temp);
    }
    else
    {
        setInfoText("Turn to stand by mode.");

        QByteArray temp;
        temp.append(83);
        sendData(temp);
    }
}

void RemoteOM::on_buttonFaults_clicked()
{
    setInfoText("Listing 20 last fault codes.");

    QByteArray temp;
    temp.append(70);
    sendData(temp);

}

void RemoteOM::on_buttonReset_clicked()
{
    setInfoText("Resetting PA.");

    QByteArray temp;
    temp.append(82);
    sendData(temp);
}

//Socket functions
void RemoteOM::readyRead()
{
    QByteArray bytes = socket->readAll();
    QString read(bytes);
    socket->flush();

    if (bytes.isNull() || bytes.isEmpty())
        return;

    //Read password
    if (read.contains("Password"))
    {
        QString password = dialog->set_password;
        if ( password.length() <= 0)
        {
            QMessageBox::information(this, tr("Remote OM"),
                                     tr("The connection requires a password. Please, define password in settings."));
            return;
        }

        setInfoText("Authenticating connection.");

        for(int i = 0; i < password.length(); i++)
        {
            QChar c = password.at(i).toAscii();
            QByteArray parr;
            parr.append(c);
            sendData(parr);
        }

        QByteArray temp;
        temp.append(13);
        sendData(temp);

        authenticate = true;

        return;
    }

    //Read faults.
    if (read.contains("LAST 20 FAULTS:"))
    {
        receiveFaults = true;
    }

    //Read all to faultBuffer until ASCII space is received
    if (receiveFaults)
    {
        for(int i = 0; i < bytes.length(); i++)
        {
            uchar t = (uchar)bytes.at(i);
            if (faultBuffer.size() > 20)
            {
                receiveFaults = false;
            }
            else
            {
                faultBuffer.append(t);
                return;
            }
        }
    }

    //Read faultBuffer first and then continue normally
    if (!receiveFaults && faultBuffer.size() > 1)
    {

        for(int i = 0; i < faultBuffer.length(); i++)
        {
            uchar t = (uchar)faultBuffer.at(i);
            //Receive faults
            switch (t)
            {
            case 48:
                setInfoText(mapResponse[48]);
                break;
            case 49:
                setInfoText(mapResponse[49]);
                break;
            case 50:
                setInfoText(mapResponse[50]);
                break;
            case 51:
                setInfoText(mapResponse[51]);
                break;
            case 52:
                setInfoText(mapResponse[52]);
                break;
            case 53:
                setInfoText(mapResponse[53]);
                break;
            case 54:
                setInfoText(mapResponse[54]);
                break;
            case 55:
                setInfoText(mapResponse[55]);
                break;
            case 56:
                setInfoText(mapResponse[56]);
                break;
            default:
                break;
            }
        }
        faultBuffer.clear();
    }

    for(int i = 0; i < bytes.length(); i++)
    {

        uchar t = (uchar)bytes.at(i);

        if (t > 127)
        {
            int d = (int)t;
            d = d - 128;

            ui->progressBar->setValue(d*1.4);
            return;
        }

        switch (t)
        {
        case 10:
            authBuffer.append(t);
            break;
        case 13:
            if (authenticate && authBuffer.at(0) == 10 && socket->state() == QAbstractSocket::ConnectedState)
            {
                setInfoText("Authentication succeeded");
            }
            else
            {
                setInfoText("Authentication failed.");
            }

            authenticate = false;
            authBuffer.clear();
            break;
        case 62:
            setInfoText(mapResponse[62]);
            ui->ledPA->show();
            sent = "";
            break;
        case 60:
            setInfoText(mapResponse[60]);
            ui->ledPA->hide();
            sent = "";
            stopFaultBlinking();
            stopWaitBlinking();
            break;
        case 83:
            setInfoText(mapResponse[83]);
            if (ui->ledStdby->isHidden())
                ui->ledStdby->show();

            if (!ui->ledOperate->isHidden())
                ui->ledOperate->hide();

            if (ui->ledPA->isHidden())
                ui->ledPA->show();

            stopWaitBlinking();
            sent = "";
            break;
        case 79:
            setInfoText(mapResponse[79]);
            if (ui->ledOperate->isHidden())
                ui->ledOperate->show();

            if (!ui->ledStdby->isHidden())
                ui->ledStdby->hide();

            if (ui->ledPA->isHidden())
                ui->ledPA->show();

            stopWaitBlinking();
            sent = "";
            break;
        case 76:
            setInfoText(mapResponse[76]);
            sent = "";
            receiveFaults = true;
            break;
        case 71:
            setInfoText(mapResponse[71]);
            sent = "";
            stopFaultBlinking();
            break;
        case 87:
            setInfoText(mapResponse[87]);

            if (ui->ledPA->isHidden())
                ui->ledPA->show();

            waitTimer->start(500);
            break;
        case 90:
            setInfoText(mapResponse[90]);
            ui->ledOperate->hide();

            stopWaitBlinking();
            ui->ledGrid->hide();
            ui->ledDrive->hide();
            ui->ledTune->hide();
            ui->ledSwr->hide();
            ui->ledStdby->show();

            break;
        case 84:
            setInfoText(mapResponse[84]);
            if (ui->ledFault->isHidden())
                ui->ledFault->show();

            break;
        case 49:
            setInfoText(mapResponse[49]);
            if (ui->ledFault->isHidden())
            {
                ui->ledFault->show();
            }
            else
            {
                ui->ledFault->hide();
            }

            startFaultTimer();
            break;
        case 50:
            setInfoText(mapResponse[50]);
            if (ui->ledFault->isHidden())
            {
                ui->ledFault->show();
            }
            else
            {
                ui->ledFault->hide();
            }

            startFaultTimer();
            break;
        case 51:
            setInfoText(mapResponse[51]);
            if (ui->ledFault->isHidden())
            {
                ui->ledFault->show();
                ui->ledSwr->show();
            }
            else
            {
                ui->ledFault->hide();
                ui->ledSwr->hide();
            }

            startFaultTimer();
            break;
        case 52:
            setInfoText(mapResponse[52]);
            if (ui->ledFault->isHidden())
            {
                ui->ledFault->show();
                ui->ledGrid->show();
            }
            else
            {
                ui->ledFault->hide();
                ui->ledGrid->hide();
            }

            startFaultTimer();
            break;
        case 53:
            setInfoText(mapResponse[53]);
            if (ui->ledFault->isHidden())
            {
                ui->ledFault->show();
                ui->ledDrive->show();
            }
            else
            {
                ui->ledFault->hide();
                ui->ledDrive->hide();
            }

            startFaultTimer();
            break;
        case 54:
            setInfoText(mapResponse[54]);
            if (ui->ledFault->isHidden())
            {
                ui->ledFault->show();
            }
            else
            {
                ui->ledFault->hide();
            }

            startFaultTimer();
            break;
        case 55:
            setInfoText(mapResponse[55]);
            if (ui->ledFault->isHidden())
            {
                ui->ledFault->show();
                ui->ledGrid->show();
            }
            else
            {
                ui->ledFault->hide();
                ui->ledGrid->hide();
            }

            startFaultTimer();
            break;
        case 56:
            setInfoText(mapResponse[56]);
            if (ui->ledFault->isHidden())
            {
                ui->ledFault->show();
                ui->ledTune->show();
            }
            else
            {
                ui->ledFault->hide();
                ui->ledTune->hide();
            }

            startFaultTimer();
            break;
        default:
            break;
        }
    }

    //Send not replied. Do one re-send.
    if  (sent.length() > 0)
    {

        QByteArray temp;
        temp.append(sent);
        sendData(temp);
        sent = "";
    }

}

//"S" - enters PA into STBY - reply of PA: "S"
//"O" - enters PA into OPERATE - reply of PA: "O"
//"R" - reset fault - reply of PA: "G" folowed by "S" or "O"
//">" - Turn PA ON - reply of PA: ">"
//"<" - Turn PA OFF - reply of PA: "<"
//"F" - Sends last 20 fault codes. Reply is list of all 20 fault codes.

void RemoteOM::sendData(QByteArray input)
{

    //Try connect socket
    if (socket->state() == QAbstractSocket::ConnectedState)
    {        
        socket->write(input);
        sent = input.at(0);
    }
    else
    {
        setInfoText("The host was not found. Please check the host name and port settings.");
    }

}

void RemoteOM::TimeOut()
{
    if (!startProfile.isEmpty())
    {
        dialog->FirstChoice(startProfile.remove("\""));
        startProfile = "";
    }

    QString show_ip = dialog->set_ipaddress;    
    QString show_port = dialog->set_port;
    ui->txtCurrentSettings->setText("Current settings: "+show_ip+":"+show_port);
}

void RemoteOM::connectSocket()
{
    if (socket && socket->state() == QTcpSocket::ConnectedState)
    {
        closeSocket();
        ui->ledPower->hide();
        setInfoText("Connection closed");
        return;
    }

    if (socket->state() != QAbstractSocket::ConnectedState)
    {
        QString ipaddress = "";
        QString port = "";

        ipaddress = dialog->set_ipaddress;
        port = dialog->set_port;

        if (ipaddress.isEmpty() || port.isEmpty())
        {
            QMessageBox::information(this, tr("Remote OM"), tr("Please select settings."));
            return;
        }
        socket->connectToHost(ipaddress, port.toInt());
        if (!socket->waitForConnected(5000)) {
            emit error(socket->error());
            return;
        }

        if (socket && socket->state() == QAbstractSocket::ConnectedState)
        {
            setInfoText("Connected to Remote OM.");
            ui->ledPower->show();
        }
        else
        {
            setInfoText("The host was not found. Please check the host name and port settings.");
        }
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)),
                SLOT(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)),
                Qt::DirectConnection);
}


void RemoteOM::closeSocket()
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState)
    {
        //TURN PA OFF
        QByteArray temp;
        temp.append(60);
        sendData(temp);
        ui->ledPA->hide();

        socket->waitForBytesWritten();
        setInfoText("Closing connection and software, please wait.");
        socket->close();
    }
}

void RemoteOM::proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *auth)
{
    auth->setUser("qsockstest");
    auth->setPassword("password");
}

void RemoteOM::error(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Remote OM"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Remote OM"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the ip address and port are valid."));
        break;
    default:
        QMessageBox::information(this, tr("Remote OM"),
                                 tr("The following error occurred: %1.")
                                 .arg(socket->errorString()));
    }

}


//Ui fault and wait led blinking
void RemoteOM::blinkFault()
{
    faultTimer->stop();
    ui->ledFault->hide();
    ui->ledSwr->hide();
    ui->ledTune->hide();
    ui->ledGrid->hide();
    ui->ledDrive->hide();

}

void RemoteOM::blinkWait()
{
    if (ui->ledGrid->isHidden())
    {
        ui->ledGrid->show();
        ui->ledDrive->show();
        ui->ledTune->show();
        ui->ledSwr->show();
    }
    else
    {
        ui->ledGrid->hide();
        ui->ledDrive->hide();
        ui->ledTune->hide();
        ui->ledSwr->hide();
    }
}

void RemoteOM::stopFaultBlinking()
{
    if (faultTimer->isActive())
        faultTimer->stop();

    ui->ledGrid->hide();
    ui->ledDrive->hide();
    ui->ledTune->hide();
    ui->ledSwr->hide();
}

void RemoteOM::stopWaitBlinking()
{
    if (waitTimer->isActive())
    {
        waitTimer->stop();
        ui->ledFault->hide();
    }

}

void RemoteOM::startFaultTimer()
{
    if (!faultTimer->isActive())
        faultTimer->start(3000);
}

