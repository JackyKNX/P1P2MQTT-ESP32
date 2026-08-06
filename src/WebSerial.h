#pragma once

#include <Arduino.h>

constexpr size_t WEB_SERIAL_BUFFER_SIZE = 32768;

// Inicjalizacja
void webSerialSetup();

// Obsługa
void webSerialLoop();

// UART0
void webSerialWriteUART0(uint8_t b);
void webSerialWriteUART0(const uint8_t *data, size_t len);

// UART2
void webSerialWriteUART2(uint8_t b);
void webSerialWriteUART2(const uint8_t *data, size_t len);

// Kasowanie obu buforów
void webSerialClear();

// Rozmiar buforów
size_t webSerialSizeUART0();
size_t webSerialSizeUART2();

// Liczniki zapisanych bajtów
uint32_t webSerialTotalWrittenUART0();
uint32_t webSerialTotalWrittenUART2();

// Pobierz nowe dane od ostatniego odczytu
String webSerialGetSinceUART0(uint32_t &sinceTotal, bool &overflow);
String webSerialGetSinceUART2(uint32_t &sinceTotal, bool &overflow);

enum WebSerialFormat
{
    SERIAL_ASCII = 0,
    SERIAL_HEX   = 1,
    SERIAL_BOTH  = 2
};

void webSerialSetFormatUART0(WebSerialFormat f);
void webSerialSetFormatUART2(WebSerialFormat f);

WebSerialFormat webSerialGetFormatUART0();
WebSerialFormat webSerialGetFormatUART2();