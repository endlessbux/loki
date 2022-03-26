package oversim.applications.trafficmixer;


simple Circuit {
    parameters:
        @class(Circuit);
}


simple CircuitBuilder {
    parameters:
        @class(CircuitBuilder);
        int maxRelayPoolSize = 10;
        int defaultLength = 3;
}


simple CircuitManager {
    parameters:
        @class(CircuitManager);
        int maxCircuitPoolSize = 5;
}