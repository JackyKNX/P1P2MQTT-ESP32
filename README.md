# P1P2MQTT-ESP32
P1P2MQTT-ESP32
Acknowledgements
This project is not a fork of P1P2MQTT. It is an ESP32 runtime designed to execute and extend the upstream P1P2MQTT protocol implementation while keeping protocol-specific logic synchronized with the original project whenever possible.
This project would not exist without the outstanding work of Arnold and the P1P2MQTT project.
The complete Daikin P1/P2 protocol reverse engineering, command implementation and support for multiple Daikin heat pump models are the result of many years of development and testing by Arnold and the community.
P1P2MQTT-ESP32 does not attempt to replace or reimplement that work.
Instead, this project provides a modern ESP32 runtime platform while reusing as much of Arnold's implementation as possible.
The long-term objective is to keep protocol-specific logic synchronized with the upstream project whenever possible. (Ecosyste.ms)
________________________________________
Upstream Project
The original P1P2MQTT project is available at:
Arnold-n/P1P2MQTT GitHub Repository
Documentation:
•	Project README
•	Documentation directory
•	E-Series Commands Reference
•	Home Assistant Integration Guide
________________________________________
Project Philosophy
The project follows one simple rule:
Protocol intelligence belongs upstream. Platform services belong to ESP32.
The ESP32 implementation focuses on:
•	Ethernet connectivity
•	Modern ESP32 platform support
•	OTA firmware updates
•	Enhanced Web UI
•	Advanced diagnostics
•	Web-based UART terminal
•	MQTT infrastructure
•	Home Assistant integration
•	OpenHAB integration
•	Future REST/WebSocket APIs
The following components should remain synchronized with the upstream P1P2MQTT project whenever possible:
•	P1P2 protocol decoder
•	Command library
•	Register tables
•	Model definitions
•	CRC/checksum algorithms
•	Daikin protocol updates
•	Support for newly released heat pump models
________________________________________
Upstream Synchronization Strategy
Rather than maintaining an independent implementation of the Daikin protocol, this project aims to keep protocol-related code as close as possible to the upstream implementation.
Whenever new functionality becomes available upstream:
Arnold publishes update
        │
        ▼
Import upstream changes
        │
        ▼
Rebuild ESP32 runtime
        │
        ▼
Done
This minimizes duplicated code and ensures that support for new Daikin models and protocol improvements can be adopted with minimal effort.
________________________________________
Architecture
                 Upstream (Arnold)

         +-----------------------------+
         |                             |
         |   P1P2 Decoder              |
         |   Command Library           |
         |   Register Tables           |
         |   Model Definitions         |
         |   CRC                       |
         +--------------+--------------+
                        |
                 Adapter Layer
                        |
        +---------------+---------------+
        |                               |
        ▼                               ▼
  ESP32 Runtime                 Future Platforms
        |
        +-- Ethernet
        +-- OTA
        +-- MQTT
        +-- Web UI
        +-- Web Terminal
        +-- Home Assistant
        +-- OpenHAB
________________________________________
