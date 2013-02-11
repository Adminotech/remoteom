#include "socket.h"

Socket::Socket()
{
}

Socket::~Socket()
{
}

//Socket functions
void Socket::readyRead()
{

    QFile file("test.txt");
    if (!file.open(QFile::WriteOnly)) {
        return;
    }

    file.write(socket->readAll());
    file.close();
//    QByteArray bytes = socket->readAll();
//    //QString test(bytes);
//    QString test = QString::fromUtf8(socket->readAll());
//    //QString test2 = QString::fromAscii(bytes);
//    QString test2 = test.toAscii();
//    QString test3 = test.toLocal8Bit();
//
//    QDataStream in(socket);
//    in.setVersion(QDataStream::Qt_4_6);
//
////    if (blockSize == 0) {
////        if (socket->bytesAvailable() < (int)sizeof(quint16))
////            return;
////
////        in >> blockSize;
////    }
////
////    if (socket->bytesAvailable() < blockSize)
////        return;
//
//    in >> tcpData;
//    ui->helpLabel->setText(tcpData);
    

}

//0x0D start character in Hex
//"S" - enters PA into STBY - reply of PA: "S"
//"O" - enters PA into OPERATE - reply of PA: "O"
//"R" - reset fault - reply of PA: "G" folowed by "S" or "O"
//">" - Turn PA ON - reply of PA: ">"
//"<" - Turn PA OFF - reply of PA: "<"
//"F" - Sends last 20 fault codes. Reply is list of all 20 fault codes.


void Socket::sendData(const char *input)
{        

    QByteArray temp;
    temp.append(60);
    socket->write(temp);
//    QByteArray arr("0x0D>");
//    QString test("0x0D>");
//    socket->write(test.toAscii()+"\r\n");
//    socket->write(test.toLocal8Bit()+"\r\n");
//    socket->write(arr);
//    socket->write("0x0D>");
//    socket->write(test.toUtf8() + "\r\n");
}

void Socket::connectSocket(QString &ipaddress, QString &port)
{

//    QString ipaddress;
//    QString port;
//    Settings s;
//
//    if (!s.readXmlFile(ipaddress, port))
//    {
//        QMessageBox::warning(this, tr("RemoteOM"), tr("Settings not found."));
//        return;
//    }

    const int Timeout = 5 * 1000;
    socket = new QTcpSocket(this);
    socket->connectToHost(ipaddress, port.toInt());

    if (!socket->waitForConnected(Timeout)) {
        emit error(socket->error());
        return;
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));    
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

}

void Socket::error(QAbstractSocket::SocketError socketError)
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
