#ifndef CHILICLIENT_H
#define CHILICLIENT_H

#include <QThread>
#include <zmq.h>
#include <QQuickItem>

//Chili Client
class ChiliClient : public QThread{
    Q_OBJECT
    Q_PROPERTY(QString svrIp MEMBER m_svrIp)
    Q_PROPERTY(QString initPort MEMBER m_initPort)
    Q_PROPERTY(QString updatePort MEMBER m_updatePort)
    Q_PROPERTY(QString subPort MEMBER m_subPort)
    Q_PROPERTY(QString storePort MEMBER m_storePort)

private:
    //To store IP addresses of client & server
    char* m_cliIp = (char *) malloc(16);
    QString m_svrIp;

    QString m_initPort;     //ReqSock
    QString m_updatePort;   //PushSock
    QString m_subPort;      //SubSock
    QString m_storePort;    //PushSock

    //Context & Sockets
    void *context;
    void *initializer;
    void *updater;
    void *subscriber;
    void *stateSaver;

signals:
    void subscribed(QString topic, QString newState);

public:
    //To run client thread
    void run() Q_DECL_OVERRIDE;

    //For Publication & Subscription
    Q_INVOKABLE void subscribe(QString topic);
    Q_INVOKABLE QString getInit(QString topic, QString defaultVal);
    Q_INVOKABLE void pubRequest(QString topic, QString newState);
    Q_INVOKABLE void storeState(QString topic, QString newState);

    //ETC
    char*  s_recv (void *socket);
    //Get IP Address (So far, we don't need to know each client's IP address, but I added this function for future purpose)
    Q_INVOKABLE char* getIp();

    explicit ChiliClient(QThread *parent = 0);
    ~ChiliClient();
};

#endif // CHILICLIENT_H
