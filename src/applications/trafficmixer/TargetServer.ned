import oversim.common.BaseApp;
import oversim.common.ITier;

package oversim.applications.trafficmixer;


simple TargetServer extends BaseApp {
    parameters:
        @class(TargetServer);
        string dataTransferMode @enum("bytecount","object","bytestream") = default("object");    // tcp dataTransferMode
}


module TargetServerModules like ITier {
    @display("i=device/server");
    gates:
        input udpIn;             // gate from the UDP layer
        output udpOut;           // gate to the UDP layer
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        output to_lowerTier;     // gate to the lower tier
        output to_upperTier;     // gate to the upper tier
        input trace_in;          // gate for trace file commands
        input tcpIn;             // gate from the TCP layer
        output tcpOut;           // gate to the TCP layer
        
    submodules:
        server: TargetServer {
            parameters:
                @display("p=60,60");
        }
        
    connections allowunconnected:
        udpIn --> server.udpIn;
        udpOut <-- server.udpOut;
        tcpIn --> server.tcpIn;
        tcpOut <-- server.tcpOut;
}
