//Qt
#include <QCoreApplication>
#include <QUdpSocket>
#include <QString>
#include <QDebug>

//Liblo
#include <lo/lo.h>
#include <lo/lo_cpp.h>

//others
#include <iostream>
#include <bitset>

//defs
#define MSG(msg) std::cout<<msg<<std::endl;

int main(int argc, char *argv[])
{
        QCoreApplication a(argc, argv);

        //help output
        if(a.arguments().contains("help"))
        {
            MSG("### OSCGrabber HELP ###");
            MSG(" ");
            MSG("The following Arguments are available:");
            MSG("   osc-server-address (address to send osc messages to) [default: 127.0.0.1]");
            MSG("   osc-send-port      (port to send osc messages to) [default: 7000]");
            MSG("   osc-listen-port    (port on wich OSCGrabber is listening for incomming osc messages) [default: 9001]");
            MSG("   udp-send-address   (address to send udp converted messages to) [default: 127.0.0.1]");
            MSG("   udp-send-port      (port to send udp converted messages to) [default: 7799]");
            MSG("   udp-listen-port    (port on wich OSCGrabber is listening for incomming udp messages) [default: 7788]");
            MSG(" ");
            MSG("examaple calls:");
            MSG("   OSCGrabber osc-listen-port 7000 osc-server-address 1.2.3.4");
            MSG(" ");
            MSG("Type 'help' for this message.");
            a.exit();
            return a.exec();
        }

        //read command line arguments
        int oscListenPort = 9001;
        if(a.arguments().contains("osc-listen-port"))
            oscListenPort = a.arguments().at(a.arguments().indexOf(QRegExp("osc-listen-port"))+1).toInt();

        QString oscServerAddress = "127.0.0.1";
        if(a.arguments().contains("osc-server-address"))
            oscServerAddress = a.arguments().at(a.arguments().indexOf(QRegExp("osc-server-address"))+1);

        int oscSendPort = 9000;
        if(a.arguments().contains("osc-send-port"))
            oscSendPort = a.arguments().at(a.arguments().indexOf(QRegExp("osc-send-port"))+1).toInt();

        int udpSendPort = 7799;
        if(a.arguments().contains("udp-send-port"))
            udpSendPort = a.arguments().at(a.arguments().indexOf(QRegExp("udp-send-port"))+1).toInt();

        QString udpSendAddress = "127.0.0.1";
        if(a.arguments().contains("udp-send-address"))
            udpSendAddress = a.arguments().at(a.arguments().indexOf(QRegExp("udp-send-address"))+1);

        int udpListenPort = 7788;
        if(a.arguments().contains("udp-listen-port"))
            udpListenPort = a.arguments().at(a.arguments().indexOf(QRegExp("udp-listen-port"))+1).toInt();


        //start creating the services
        MSG("### [Creating] UDP service at Port "<<udpListenPort);
        MSG("### Sending to "<<udpSendAddress.toStdString());
        QUdpSocket *listenSocket = new QUdpSocket(&a);
        listenSocket->bind(QHostAddress::AnyIPv4, udpListenPort);

        if(!listenSocket->isValid())
        {
            MSG("Couldn't creat udp service at port "<<udpSendPort<<" . Port already occupied?");
            a.exit();
            return 0; //a.exec();
        }

        MSG("### [Launching] OSC Bridge to address "<<oscServerAddress.toStdString()<<" "<<oscListenPort)
        lo::ServerThread * server = new lo::ServerThread(oscListenPort);
        server->start();

        if (!server->is_valid()) { //this check isn't working. why?
            MSG("Couldn't creat osc service listening to port "<<oscListenPort<<" . Port already occupied?");
            a.exit();
            return 0; //a.exec();
        }
        MSG("### --> Listening to Port: "<<oscListenPort);

        lo::string_type address(oscServerAddress.toStdString());
        lo::Address * oscSender = new lo::Address(address, oscSendPort);

//        lo::Address * oscSender = new lo::Address("localhost", oscSendPort);
        MSG("### --> Writing on Port: "<<oscSendPort);


        //####################################################################################
        //####################################################################################
        //  OSC to UDP
        //####################################################################################
        //####################################################################################

        QUdpSocket *senderSocket = new QUdpSocket(&a);

        server->add_method(NULL, NULL, [=](const char *path, const lo::Message& msg)
        {
//            msg.print();
            QString type = QString::fromStdString(msg.types());
            QString data(path);
            data.append("%");
            data.append(type);
            data.append("%");

            for (int i = 0; i < msg.argc(); i++) {
                if(type.at(i) == 'i')
                {
                    data.append(QString::number(msg.argv()[i]->i)+"%");
//                    std::cout<<std::bitset<32>()<<std::endl;
                }
                else if(type.at(i) == 'f')
                    data.append(QString::number(msg.argv()[i]->f)+"%");
                else if(type.at(i) == 's')
                    data.append(QString::fromStdString(&msg.argv()[i]->S)+"%");
            }

            if(-1 == senderSocket->writeDatagram(data.toUtf8(), QHostAddress(udpSendAddress), udpSendPort))
            {
                //qDebug()<<"ERROR: "<<senderSocket->errorString()<<" "<<senderSocket->error();
            }
            else
            {
                //qDebug()<<"From live: "<<data;
                //qDebug()<<"NO ERROR?: "<<senderSocket->errorString()<<" "<<senderSocket->error()<<" valid? "<<senderSocket->isValid();
            }
        });

        //####################################################################################
        //####################################################################################
        //  UDP to OSC
        //####################################################################################
        //####################################################################################

        QObject::connect(listenSocket, &QUdpSocket::readyRead, [=]()
        {
            QByteArray datagram;
            datagram.resize(listenSocket->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;

            listenSocket->readDatagram(datagram.data(), datagram.size(),
                                            &sender, &senderPort);

            //qDebug()<<"Message from: "<<sender<<" "<<senderPort<<" "<<QString::fromUtf8(datagram);

            QStringList splittedData = QString::fromUtf8(datagram).split("%");

            if(splittedData.length() > 1)
            {

                lo::string_type path(splittedData.takeFirst().toStdString());
                QString dataTypes = splittedData.takeFirst();
                lo::Message message;

                for(int i = 0; i<dataTypes.length(); i++)
                {
                    //qDebug()<<"type: "<<dataTypes.at(i);
                    if(dataTypes.at(i) == 'i')
                        message.add(splittedData.at(i).toInt());
                    else if(dataTypes.at(i) == 'f')
                        message.add(splittedData.at(i).toFloat());
                    else if(dataTypes.at(i) == 's')
                    {
                        lo::string_type str(splittedData.at(i).toStdString());
                        message.add(str);
                    }
                }
                //qDebug()<<"Stringlist: "<<splittedData;
                /*qDebug()<<"To live: "<<*/
                oscSender->send(path, message);//<<QString::fromStdString(message.types())<<path._s;

            }
            else
            {
                lo::string_type path(splittedData.takeFirst().toStdString());
                /*qDebug()<<"To live: "<<*/
                oscSender->send(path);
            }

        });

        //clean up services
        QObject::connect(&a, &QCoreApplication::destroyed, [=](){
            server->stop();
            delete server;
        });

        return a.exec();
}

