#ifndef __TARGETSERVER_H_
#define __TARGETSERVER_H_

#include "common/NodeHandle.h"
#include "common/BaseApp.h"


using namespace std;

class TargetServer : public BaseApp {
    public:
        static int port;
        static TransportAddress address;


        TargetServer();
        ~TargetServer();
        virtual void initializeApp(int stage);
        virtual void finishApp();
        virtual bool internalHandleRpcCall(BaseCallMessage* msg) override;
        virtual void handleUDPMessage(cMessage* msg) override;
        void printLog(string functionName) {
            EV << "[TargetServer::" << functionName
               << " @ " << thisNode.getIp().str()
               << "(" << thisNode.getKey().toString(16) << ")"
               << "]" << endl;
        }
};

#endif
