#include <Arduino.h>
#include <ETH.h>
#include <WiFi.h>

#include "Config.h"
#include "OTAManager.h"
#include "WebManager.h"
#include "WebSerial.h"

bool eth_connected = false;

static bool servicesStarted = false;

void startServices()
{
    if (servicesStarted)
        return;

    otaSetup();

    webSerialSetup();

    webSetup();

    servicesStarted = true;
}

void WiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {
        case ARDUINO_EVENT_ETH_START:

            Serial.println();
            Serial.println("=== Ethernet START ===");

            ETH.setHostname(OTA_HOSTNAME);

            break;

        case ARDUINO_EVENT_ETH_CONNECTED:

            Serial.println("=== Ethernet LINK UP ===");

            break;

        case ARDUINO_EVENT_ETH_GOT_IP:

            eth_connected = true;

            Serial.println("=== Ethernet GOT IP ===");

            Serial.print("IP      : ");
            Serial.println(ETH.localIP());

            Serial.print("Mask    : ");
            Serial.println(ETH.subnetMask());

            Serial.print("Gateway : ");
            Serial.println(ETH.gatewayIP());

            Serial.print("MAC     : ");
            Serial.println(ETH.macAddress());

            startServices();

            break;

        case ARDUINO_EVENT_ETH_DISCONNECTED:

            eth_connected = false;

            Serial.println("=== Ethernet LINK DOWN ===");

            break;

        case ARDUINO_EVENT_ETH_STOP:

            eth_connected = false;

            Serial.println("=== Ethernet STOP ===");

            break;

        default:
            break;
    }
}

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("======================================");
    Serial.println("        P1P2MQTT ESP32");
    Serial.print("Version : ");
    Serial.println(FW_VERSION);
    Serial.println("======================================");

    WiFi.onEvent(WiFiEvent);

    Serial.println("Starting Ethernet...");

    ETH.begin(
        PHY_ADDR,
        ETH_POWER_PIN,
        ETH_MDC_PIN,
        ETH_MDIO_PIN,
        ETH_PHY_IP101,
        ETH_CLOCK_GPIO0_IN
    );

    Serial.println("Waiting for DHCP...");

    uint32_t timeout = millis();

    while (!ETH.localIP())
    {
        delay(100);

        if (millis() - timeout > 10000)
        {
            Serial.println("WARNING: DHCP not ready yet, continuing in background.");
            break;
        }
    }

    // OTA i WWW zostaną uruchomione automatycznie
    // po otrzymaniu adresu IP przez Ethernet.


#if ENABLE_UART0_SNIFFER

    Serial.println("Switching UART0 to sniffer...");

    Serial.flush();
    delay(100);

    Serial.end();

    Serial.begin(
        UART_BAUD,
        SERIAL_8N1,
        UART0_RX_PIN,
        UART0_TX_PIN
    );

    delay(50);

#endif

Serial2.begin(
    UART_BAUD,
    SERIAL_8N1,
    UART2_RX_PIN,
    UART2_TX_PIN
);

Serial2.flush();
delay(20);

}

static bool uart0NewLine = true;
static bool uart2NewLine = true;

void loop()
{

//
// UART0 - ESP8266 -> ATmega
//


// UART0
while (Serial.available())
{
    uint8_t b = Serial.read();

    if (uart0NewLine)
    {
        webSerialWriteUART0((const uint8_t*)"\n[U0] ",6);
        uart0NewLine = false;
    }

    webSerialWriteUART0(b);

    if (b == '\n')
        uart0NewLine = true;
}

// UART2
while (Serial2.available())
{
    uint8_t b = Serial2.read();

    if (uart2NewLine)
    {
        webSerialWriteUART2((const uint8_t*)"\n[U2] ",6);
        uart2NewLine = false;
    }

    webSerialWriteUART2(b);

    if (b == '\n')
        uart2NewLine = true;
}


    otaLoop();

    webLoop();

    webSerialLoop();

}