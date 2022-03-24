import oversim.common.BaseApp;
import oversim.common.ITier;
import oversim.applications.dht.DHTModules;

package oversim.applications.trafficmixer;


simple TrafficMixer extends BaseApp {
    parameters:
        @class(TrafficMixer);
        int poolSize = 10;
        int numCircuits = 3;
}


module TrafficMixerModules like ITier {
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
        mix: TrafficMixer;

    connections allowunconnected:
        from_lowerTier --> mix.from_lowerTier;
        to_lowerTier <-- mix.to_lowerTier;
        udpIn --> mix.udpIn;
        udpOut <-- mix.udpOut;
        tcpIn --> mix.tcpIn;
        tcpOut <-- mix.tcpOut;
        trace_in --> mix.trace_in;
}
