//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

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
