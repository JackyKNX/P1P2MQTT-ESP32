#pragma once

//
// Firmware
//
#define FW_VERSION "0.1.0"

//
// OTA
//
#define OTA_HOSTNAME "p1p2mqtt"
#define OTA_PASSWORD "P1P2MQTT"

// UART0 (ESP8266 -> ATmega) na plycie

#define ENABLE_UART0_SNIFFER 1
#define UART0_RX_PIN 3
#define UART0_TX_PIN 1

// UART2 (ATmega -> ESP8266) zlacze zewnetrzne
#define UART2_RX_PIN 17
#define UART2_TX_PIN 16

#define UART_BAUD 250000


//
// Ethernet (M5Stack PoESP32 U138)
//
#define PHY_ADDR   1
#define ETH_POWER_PIN  5
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN   18
