#include "chiliclient.h"

#include <zmq.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <assert.h>
#include "helper.h"
using namespace std;

#include "state.h"

#define REQUEST_TIMEOUT 2500 // msecs, (> 1000!)
#define REQUEST_RETRIES 3 // Before we abandon

//  Version checking, and patch up missing constants to match 2.1
#if ZMQ_VERSION_MAJOR == 2
#   error "Please upgrade to ZeroMQ/3.2 for this program"
#endif

ChiliClient::ChiliClient(QThread *parent): QThread(parent){
    start(); //Start thread
}

void ChiliClient::run(){
    //Setting IP addresses
    m_cliIp = getIp();

    //Initial Setting (Context, Sockets)
    char addr[30];
    context = zmq_ctx_new ();

    initializer = zmq_socket (context, ZMQ_REQ);
    sprintf (addr, "tcp://%s:%s", m_svrIp.toUtf8().data(), m_initPort.toUtf8().data());
    int rc = zmq_connect (initializer, addr); assert(rc==0);

    updater = zmq_socket (context, ZMQ_PUSH);
    sprintf (addr, "tcp://%s:%s", m_svrIp.toUtf8().data(), m_updatePort.toUtf8().data());
    rc = zmq_connect (updater, addr); assert(rc==0);

    subscriber = zmq_socket (context, ZMQ_SUB);
    sprintf (addr, "tcp://%s:%s", m_svrIp.toUtf8().data(), m_subPort.toUtf8().data());
    rc = zmq_connect(subscriber, addr); assert(rc==0);

    stateSaver = zmq_socket (context, ZMQ_PUSH);
    sprintf (addr, "tcp://%s:%s", m_svrIp.toUtf8().data(), m_storePort.toUtf8().data());
    rc = zmq_connect (stateSaver, addr); assert(rc==0);

    // Initialize poll set
    zmq_pollitem_t msgs[] = {
        { subscriber, 0, ZMQ_POLLIN, 0 },
    };

    while(1){
        zmq_poll(msgs, 1, -1);

        //When a new state is published
       if (msgs[0].revents & ZMQ_POLLIN) {
            //1. Subscribe message
            char *msg = s_recv (subscriber);
            if(msg != NULL){ //qDebug("[Subscribed] %s", msg);
                //Read message
                char* topic = (char*) malloc(strlen(msg));
                char* newState = (char*) malloc(strlen(msg));
                sscanf(msg, "%s %s", topic, newState);

                //Emit 'subscribed' signal
                emit subscribed(QString(topic), QString(newState));
                free(topic);
                free(newState);
            }
            free(msg);
        }else{
            qDebug("Cannot handle msg arrived.");
            break;
        }
    }
}


QString ChiliClient::getInit(QString topic, QString defaultVal){
   int retries_left = REQUEST_RETRIES;
   char* cTopic = toChar(topic);
   char* cDefaultVal = toChar(defaultVal);
   int size = strlen(cTopic)+strlen(cDefaultVal)+2;

   // Poll socket for a reply, with timeout
   zmq_pollitem_t msgs [] = { { initializer, 0, ZMQ_POLLIN, 0 } };
   FILE* pFile;

   while (retries_left) {
       //send a query to get initState
       char* msg = (char *) malloc(size);
       sprintf(msg, "%s %s", cTopic, cDefaultVal);

       zmq_send (initializer, msg, size, 0);

       int expect_reply = 1;
       while (expect_reply) {
           zmq_poll (msgs, 1, REQUEST_TIMEOUT);

           if (msgs[0].revents & ZMQ_POLLIN) {
                char *initState = s_recv (initializer);
                if (!initState){
                    free(initState);
                    break; // Interrupted
                }else{
                    //Save state to a file
                    pFile = fopen(cTopic, "w");
                    if (pFile != NULL) {
                        fputs(initState, pFile);
                        fclose(pFile);
                    }else{
                        qDebug("Cannot open file, %s", cTopic);
                        free(initState);
                        break;
                    }
                    retries_left = REQUEST_RETRIES;
                    expect_reply = 0;
                    free(cTopic);
                    free(cDefaultVal);
                    return QString(initState);
                }
                free(msg);
            }else if (--retries_left == 0) {
                printf ("E: server seems to be offline, abandoning\n");
                break;
            }else {
                printf ("W: no response from server, retrying…\n");

                // Old socket is confused; close it and open a new one
                zmq_close (initializer);
                initializer = zmq_socket (context, ZMQ_REQ);
                char* addr = (char *) malloc(30);
                sprintf (addr, "tcp://%s:%s", m_svrIp.toUtf8().data(), m_initPort.toUtf8().data());
                zmq_connect (initializer, addr);

                // send a query again, on new socket
                char* updateMsg = (char *) malloc(size);
                sprintf(updateMsg, "%s %s", cTopic, cDefaultVal);
                zmq_send (initializer, updateMsg, size, 0);

                free(addr);
                free(updateMsg);
            }
        }
       free(msg);
   }
   free(cTopic);
   free(cDefaultVal);
   return NULL;
}


void ChiliClient::subscribe(QString topic){
    char* charTopic = toChar(topic);
    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, charTopic, strlen (charTopic));
    free(charTopic);
}

void ChiliClient::pubRequest(QString topic, QString newState){
    char* cTopic = toChar(topic);
    char* cNewState = toChar(newState);

    int size = strlen(cTopic)+strlen(cNewState)+2;
    char* msg = (char*) malloc(size);
    sprintf (msg, "%s %s", cTopic, cNewState);
    zmq_send (updater, msg, size, 0);

    free(cTopic);
    free(cNewState);
    free(msg);
}


void ChiliClient::storeState(QString topic, QString newState){
    char* cTopic = toChar(topic);
    char* cNewState = toChar(newState);

    int size = strlen(cTopic)+strlen(cNewState)+2;
    char* msg = (char*) malloc(size);
    sprintf (msg, "%s %s", cTopic, cNewState);
    zmq_send (stateSaver, msg, size, 0);
    //qDebug("storeState %s", update);
    free(cTopic);
    free(cNewState);
    free(msg);
}


char *  ChiliClient::s_recv (void *socket) {
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    int size = zmq_msg_recv(&msg, socket, 0);
    if(size == -1){
        return NULL;
    }
    char* str = (char *) malloc(size+1);
    memcpy(str, zmq_msg_data(&msg), size);
    zmq_msg_close(&msg);
    str[size]=0;
    return (str);
}

char* ChiliClient::getIp(){
    struct ifaddrs *ifAddrStruct=NULL;
    struct ifaddrs *ifa=NULL;
    void *tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            //char addressBuffer[INET_ADDRSTRLEN];
            char* addressBuffer = (char*) malloc(INET_ADDRSTRLEN);
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if(strcmp(ifa->ifa_name, "eth0") == 0){
               // printf("%s IP Address===== %s\n", ifa->ifa_name, addressBuffer);
                return addressBuffer;
            }
        } else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
           // printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return "[ERROR] getIp(): SOMETHING WRONG";
}

ChiliClient::~ChiliClient(){
    free(m_cliIp);
    //CLOSE Sockets & DESTROY context
    zmq_close (initializer);
    zmq_close (updater);
    zmq_close (subscriber);
    zmq_close (stateSaver);
    zmq_ctx_destroy (context);
}

