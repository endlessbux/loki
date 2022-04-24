#ifndef __TARGETSERVER_H_
#define __TARGETSERVER_H_

#include "common/NodeHandle.h"
#include "common/BaseApp.h"


using namespace std;

class TargetServer : public BaseApp {
    public:
        static int tcpPort;
        static NodeHandle address;


        TargetServer();
        ~TargetServer();
        virtual void initializeApp(int stage);
        virtual void finishApp();
        virtual void handleDataReceived(TransportAddress address, cPacket* msg, bool urgent);
        void printLog(string functionName) {
            EV << "[TargetServer::" << functionName
               << " @ " << thisNode.getIp().str()
               << "(" << thisNode.getKey().toString(16) << ")"
               << "]" << endl;
        }
};

#endif
