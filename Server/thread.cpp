#include "thread.h"
#include <QHostAddress>
#include <QMessageBox>
#include <QList>
#include <QDataStream>

Thread::Thread(QVector<Item> *_items,QTcpServer * tcpServer,QTableWidget *_tableWidget)
{
    items = _items;
    tcpSocket = tcpServer->nextPendingConnection();
    tableWidget = _tableWidget;
}
void Thread::run()
{
    connect(tcpSocket,SIGNAL(readyRead()),\
            this,SLOT(on_tcpSocket_readyRead()));
}
void Thread::on_tcpSocket_readyRead()
{
    QString srcHostName,srcIP,destIP;
    int msgeType;
    //QMessageBox::information(0,"",QString("%1").arg((int)tcpSocket),QMessageBox::Ok);
    QDataStream in(tcpSocket);
    in>>msgeType>>srcIP>>srcHostName>>destIP;
    switch (msgeType)
    {
    case NewParticipant:
    {
        this->newParticipant(srcHostName,srcIP);
        break;
    }
    case ParticipantLeft:
    {
        this->participantLeft(srcIP);
        break;
    }
    case Message:
    {
        break;
    }
    default:
        break;
    }

}
void Thread::newParticipant(QString hostName, QString IP)
{
    //更新用户列表
    bool isEmpty = tableWidget->findItems(IP,Qt::MatchExactly).isEmpty();
    if(isEmpty)
    {
        QTableWidgetItem * host = new QTableWidgetItem(hostName);
        QTableWidgetItem * ip = new QTableWidgetItem(IP);
        QTableWidgetItem * Socket = new QTableWidgetItem(QString("%1").arg((int)tcpSocket));
        tableWidget->insertRow(0);
        tableWidget->setItem(0,0,ip);
        tableWidget->setItem(0,1,host);
        tableWidget->setItem(0,2,Socket);
    }
}
void Thread::participantLeft(QString IP)
{
   // QMessageBox::information(0,"",IP);
    //依据IP查找和删除某个用户
    QList<QTableWidgetItem *> tmp = tableWidget->findItems(IP,Qt::MatchExactly);
    if (tmp.isEmpty() == false)
    {
        int rowNum = tmp.first()->row();
        tableWidget->removeRow(rowNum);
       // tableWidget->(tr("onlines:%1").arg(ui->userTableWidget->rowCount()));
    }
}
void Thread::sendMessage(\
        MessageType type,\
        QString destIP,\
        QString srcIP,\
        QString srcHostName,\
        QString msge)
{
    QTcpSocket * destSocket = NULL;
    QVector<Item>::iterator it = items->begin();
    for(; it != items->end() ; ++it)
    {
        if(it->IP == destIP)
        {
            destSocket = it->tcpSocket;
        }
    }
    if(destSocket != NULL)
    {
        //发送消息
        QByteArray data;
        QDataStream  out(&data,QIODevice::WriteOnly);
        out<<type<<srcIP<<srcHostName<<destIP<<msge;
        destSocket->write(data);

    }
    else
    {
        //如果没有找到destIP 对应的SOCKET，那么把错误返回给发送者
    }
}