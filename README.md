⚠️ Early development
This project is currently under active development. The architecture is stabilizing before the integration of the upstream P1P2MQTT core.

# P1P2MQTT-ESP32

> Modern ESP32 + Ethernet platform for the P1P2MQTT ecosystem.

## Overview

P1P2MQTT-ESP32 is an ESP32-based hardware platform designed to run the P1P2MQTT ecosystem on modern hardware.

The project focuses on providing a robust and extensible ESP32 runtime while **reusing as much of the original P1P2MQTT implementation as possible**.

It is **not intended to replace or fork** the original project.

Instead, it provides:

- ESP32 platform support
- Native Ethernet connectivity
- OTA firmware updates
- Modern Web UI
- Advanced Web Terminal
- MQTT infrastructure
- Home Assistant integration
- OpenHAB integration
- Future REST/WebSocket APIs
- Enhanced diagnostics and logging

---

# Project Philosophy

The project follows one simple principle:

> **Protocol intelligence belongs upstream. Platform services belong to ESP32.**

The Daikin protocol implementation should remain maintained by the original P1P2MQTT project.

The ESP32 project provides only the runtime platform.

---

# Upstream Project

This project is built around the outstanding work of **Arnold** and the original **P1P2MQTT** project.

GitHub Repository

https://github.com/Arnold-n/P1P2MQTT

Useful documentation:

- README
  https://github.com/Arnold-n/P1P2MQTT/blob/main/README.md

- Documentation
  https://github.com/Arnold-n/P1P2MQTT/tree/main/doc

- E-Series Commands
  https://github.com/Arnold-n/P1P2MQTT/blob/main/doc/Commands-Eseries.md

- Home Assistant
  https://github.com/Arnold-n/P1P2MQTT/blob/main/doc/Home-Assistant.md

---

# Design Goals

## Preserve upstream compatibility

The following components should remain synchronized with Arnold's project whenever possible:

- P1P2 frame decoder
- Command library
- Register definitions
- Model tables
- CRC algorithms
- Protocol constants
- Support for new Daikin models

The ESP32 implementation should avoid reimplementing protocol logic.

---

## Modern ESP32 Runtime

ESP32 is responsible for:

- Ethernet
- UART management
- OTA updates
- MQTT client
- Web Server
- Web Terminal
- Diagnostics
- Logging
- Platform configuration

---

## Clean Architecture

Each module has a single responsibility.

```
                UART
                  │
                  ▼
            UART Manager
                  │
                  ▼
            Arnold Adapter
                  │
                  ▼
             Arnold Core
                  │
                  ▼
            Register Database
        ┌─────────┼──────────┐
        ▼         ▼          ▼
      MQTT      Web UI    REST API
        │
        ▼
Home Assistant / OpenHAB
```

---

# Repository Structure

```
P1P2MQTT-ESP32/

src/
    main.cpp
    Config.h

platform/
    UARTManager
    EthernetManager
    MQTTManager
    OTAManager
    WebManager
    WebSerial

adapter/
    ArnoldAdapter

database/
    RegisterDB

integrations/
    HomeAssistant
    OpenHAB

upstream/
    ArnoldCore
```

---

# Communication Flow

## Receiving data

```
P1P2 Bus
    │
    ▼
ATmega
    │
    ▼
UART Manager
    │
    ▼
Arnold Decoder
    │
    ▼
Register Database
    │
    ├── MQTT
    ├── Web UI
    ├── Home Assistant
    ├── OpenHAB
    └── Future APIs
```

---

## Sending commands

```
MQTT / HA / Web UI
        │
        ▼
Application API
        │
        ▼
Arnold Command Library
        │
        ▼
ATmega Transport
        │
        ▼
UART
        │
        ▼
ATmega
```

The application layer never needs to know the UART command syntax.

---

# Upstream Synchronization

One of the primary goals of this project is to keep protocol-specific code synchronized with the upstream P1P2MQTT project.

Typical workflow:

```
Arnold publishes update
        │
        ▼
Update ArnoldCore
        │
        ▼
Rebuild ESP32 Runtime
        │
        ▼
Done
```

This minimizes duplicated code and makes it easy to support newly released Daikin models.

---

# What belongs to ArnoldCore?

Whenever possible, these components should remain untouched:

- P1P2 protocol decoder
- Command implementation
- Register tables
- Model definitions
- CRC algorithms
- Protocol constants
- Daikin protocol knowledge

---

# What belongs to ESP32?

ESP32-specific functionality includes:

- Ethernet
- OTA
- MQTT
- Web UI
- Web Terminal
- Logging
- Diagnostics
- REST API
- WebSocket
- Home Assistant Discovery
- OpenHAB integration

---

# Long-Term Vision

The objective is to clearly separate:

## ArnoldCore

Responsible for:

- Daikin protocol
- Decoder
- Commands
- Register database
- New model support

## ESP32 Runtime

Responsible for:

- Hardware abstraction
- Networking
- Integrations
- User interface
- Diagnostics
- Platform services

This architecture allows both projects to evolve independently while remaining fully compatible.

---

# Acknowledgements

Special thanks to **Arnold** for the incredible amount of work invested in reverse engineering the Daikin P1P2 protocol and maintaining the P1P2MQTT project.

Without that work, this project would not exist.

The goal of P1P2MQTT-ESP32 is **not to replace** the original project, but to provide a modern ESP32 platform capable of running and extending the existing protocol implementation.

Whenever possible, improvements in protocol support should continue to come from the upstream project.

---

# License

This project follows the licensing terms of the original P1P2MQTT project for reused upstream components.

ESP32 platform-specific additions are licensed under the license specified in this repository.
