#include <Arduino.h>
#include <ETH.h>
#include <WebServer.h>
#include <Update.h>

#include "Config.h"
#include "WebSerial.h"

extern bool eth_connected;

WebServer server(80);

static uint32_t bootMillis = 0;
static bool webStarted = false;

// ---- OTA-przez-WWW: stan aktualizacji ----
static bool otaWebInProgress = false;
static bool otaWebError = false;
static String otaWebErrorMsg;

void handleSerialFormatUART0()
{
    if(!server.hasArg("plain"))
    {
        server.send(400,"text/plain","missing");
        return;
    }

    String s = server.arg("plain");

    if(s=="ascii")
        webSerialSetFormatUART0(SERIAL_ASCII);

    else if(s=="hex")
        webSerialSetFormatUART0(SERIAL_HEX);

    else
        webSerialSetFormatUART0(SERIAL_BOTH);

    server.send(200,"text/plain","OK");
}



void handleSerialFormatUART2()
{
    if(!server.hasArg("plain"))
    {
        server.send(400,"text/plain","missing");
        return;
    }

    String s = server.arg("plain");

    if(s=="ascii")
        webSerialSetFormatUART2(SERIAL_ASCII);

    else if(s=="hex")
        webSerialSetFormatUART2(SERIAL_HEX);

    else
        webSerialSetFormatUART2(SERIAL_BOTH);

    server.send(200,"text/plain","OK");
}




String uptimeString()
{
    uint32_t sec = (millis() - bootMillis) / 1000;

    uint32_t days = sec / 86400;
    sec %= 86400;

    uint32_t hours = sec / 3600;
    sec %= 3600;

    uint32_t minutes = sec / 60;
    sec %= 60;

    char buf[32];

    sprintf(buf, "%ud %02u:%02u:%02u",
            days,
            hours,
            minutes,
            sec);

    return String(buf);
}

String jsonEscape(const String &s)
{
    String out;

    out.reserve(s.length() + 8);

    for (size_t i = 0; i < s.length(); i++)
    {
        char c = s[i];

        switch (c)
        {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\r': out += "\\r";  break;
            case '\n': out += "\\n";  break;
            case '\t': out += "\\t";  break;

            default:
                if ((uint8_t) c < 0x20)
                {
                    char buf[8];
                    sprintf(buf, "\\u%04x", c);
                    out += buf;
                }
                else
                {
                    out += c;
                }
                break;
        }
    }

    return out;
}

String htmlPage()
{
    String s;

    s.reserve(3000);

    s += "<!DOCTYPE html>";
    s += "<html>";
    s += "<head>";
    s += "<meta charset='utf-8'>";
    s += "<meta http-equiv='refresh' content='5'>";
    s += "<title>P1P2MQTT</title>";

    s += "<style>";
    s += "body{font-family:Arial;margin:30px;background:#f4f4f4;}";
    s += "table{border-collapse:collapse;}";
    s += "td{padding:6px 12px;border-bottom:1px solid #ddd;}";
    s += "h2{margin-bottom:20px;}";
    s += "</style>";

    s += "</head>";
    s += "<body>";

    s += "<h2>P1P2MQTT Bridge</h2>";

    s += "<table>";

    s += "<tr><td>Firmware</td><td>";
    s += FW_VERSION;
    s += "</td></tr>";

    s += "<tr><td>IP</td><td>";
    s += ETH.localIP().toString();
    s += "</td></tr>";

    s += "<tr><td>Gateway</td><td>";
    s += ETH.gatewayIP().toString();
    s += "</td></tr>";

    s += "<tr><td>Subnet</td><td>";
    s += ETH.subnetMask().toString();
    s += "</td></tr>";

    s += "<tr><td>MAC</td><td>";
    s += ETH.macAddress();
    s += "</td></tr>";

    s += "<tr><td>Ethernet</td><td>";
    s += eth_connected ? "CONNECTED" : "DOWN";
    s += "</td></tr>";

    s += "<tr><td>Uptime</td><td>";
    s += uptimeString();
    s += "</td></tr>";

    s += "</table>";

    s += "<br>";

    s += "<a href='/api/status'>JSON status</a>";
    s += " | ";
    s += "<a href='/update'>Firmware update</a>";
    s += " | ";
    s += "<a href='/serial'>Serial monitor</a>";

    s += "</body>";
    s += "</html>";

    return s;
}

String updatePage()
{
    String s;

    s.reserve(1200);

    s += "<!DOCTYPE html>";
    s += "<html>";
    s += "<head>";
    s += "<meta charset='utf-8'>";
    s += "<title>P1P2MQTT - OTA Update</title>";

    s += "<style>";
    s += "body{font-family:Arial;margin:30px;background:#f4f4f4;}";
    s += "h2{margin-bottom:20px;}";
    s += ".box{background:#fff;padding:20px;border-radius:6px;max-width:500px;}";
    s += "input[type=submit]{margin-top:10px;padding:8px 16px;}";
    s += "</style>";

    s += "</head>";
    s += "<body>";

    s += "<h2>P1P2MQTT Firmware Update</h2>";

    s += "<div class='box'>";
    s += "<p>Current firmware: ";
    s += FW_VERSION;
    s += "</p>";

    s += "<form method='POST' action='/update' enctype='multipart/form-data'>";
    s += "<input type='file' name='firmware' accept='.bin'>";
    s += "<br>";
    s += "<input type='submit' value='Upload & Flash'>";
    s += "</form>";

    s += "<p><a href='/'>&larr; Back</a></p>";

    s += "</div>";

    s += "</body>";
    s += "</html>";

    return s;
}

String serialPage()
{
    String s;

    s.reserve(2600);

    s += "<!DOCTYPE html>";
    s += "<html>";
    s += "<head>";
    s += "<meta charset='utf-8'>";
    s += "<title>P1P2MQTT - Serial Monitor</title>";

    s += "<style>";
    s += "body{font-family:Arial;margin:20px;background:#f4f4f4;}";
    s += "h2{margin-bottom:10px;}";
    s += ".term{background:#111;color:#0f0;font-family:Consolas,monospace;";
    s += "font-size:13px;padding:10px;height:35vh;overflow-y:scroll;";
    s += "white-space:pre-wrap;word-break:break-all;border-radius:6px;}";
    s += "button{padding:6px 14px;margin:8px 8px 8px 0;}";
    s += "#stat{color:#555;font-size:12px;margin-bottom:6px;}";
    s += "</style>";

    s += "</head>";
    s += "<body>";

    s += "<h2>P1P2 Serial Monitor</h2>";

    s += "<div id='stat'>polaczony...</div>";

    s += "<button id='btnPause'>Pauza</button>";
    s += "<button id='btnClear'>Wyczysc</button>";
    s += "<a href='/'>&larr; Strona glowna</a>";



    s += "<h3>UART0 (ESP8266)</h3>";

    s += "<div style='margin-bottom:8px'>";
    s += "<label><input type='radio' name='fmt0' value='ascii'>ASCII</label>";
    s += "<label><input type='radio' name='fmt0' value='hex'>HEX</label>";
    s += "<label><input type='radio' name='fmt0' value='both' checked>BOTH</label>";
    s += "</div>";

    s += "<div id='term0' class='term'></div>";

    s += "<br>";

    s += "<h3>UART2 (ATmega)</h3>";

    s += "<div style='margin-bottom:8px'>";
    s += "<label><input type='radio' name='fmt2' value='ascii'>ASCII</label>";
    s += "<label><input type='radio' name='fmt2' value='hex'>HEX</label>";
    s += "<label><input type='radio' name='fmt2' value='both' checked>BOTH</label>";
    s += "</div>";

    s += "<div id='term2' class='term'></div>";


    s += "<br>";

    s += "<input id='cmd' ";
    s += "style='width:500px;font-family:Consolas' ";
    s += "placeholder='UART command'>";

    s += "<button onclick='sendCmd()'>Send</button>";

    s += "<script>";

    s += "let since0 = 0;";
    s += "let since2 = 0;";
    s += "let paused = false;";
    s += "const term0 = document.getElementById('term0');";
    s += "const term2 = document.getElementById('term2');";
    s += "const stat = document.getElementById('stat');";

    s += "document.getElementById('btnPause').onclick = function(){";
    s += "  paused = !paused;";
    s += "  this.textContent = paused ? 'Wznow' : 'Pauza';";
    s += "};";

    s += "document.getElementById('btnClear').onclick = function(){";
    s += "  fetch('/api/serial/clear', {method:'POST'}).then(function(){";
    s += "    term0.textContent='';";
    s += "    term2.textContent='';";
    s += "  });";
    s += "};";


    s += "document.querySelectorAll('input[name=fmt0]').forEach(function(r){";

    s += "    r.onchange=function(){";

    s += "        fetch('/api/serial/format0',{";

    s += "            method:'POST',";

    s += "            body:this.value";

    s += "        });";

    s += "    };";

    s += "});";

    s += "document.querySelectorAll('input[name=fmt2]').forEach(function(r){";

    s += "    r.onchange=function(){";

    s += "        fetch('/api/serial/format2',{";

    s += "            method:'POST',";

    s += "            body:this.value";

    s += "        });";

    s += "    };";

    s += "});";


    s += "function poll(){";
    s += "  if (paused) { setTimeout(poll, 250); return; }";
    s += "  fetch('/api/serial/data?since0=' + since0 + '&since2=' + since2)";
    s += "    .then(function(r){ return r.json(); })";
    s += "    .then(function(j){";

    s += "since0=j.total0;";
    s += "since2=j.total2;";

    s += "if(j.overflow0)";
    s += "    term0.textContent+='\\n[overflow]\\n';";

    s += "if(j.overflow2)";
    s += "    term2.textContent+='\\n[overflow]\\n';";

    s += "if(j.data0.length)";
    s += "{";
    s += "    const atBottom0 =";
    s += "        term0.scrollHeight - term0.scrollTop - term0.clientHeight < 40;";

    s += "    term0.textContent += j.data0;";

    s += "    if(atBottom0)";
    s += "        term0.scrollTop = term0.scrollHeight;";
    s += "}";

    s += "if(j.data2.length)";
    s += "{";
    s += "    const atBottom2 =";
    s += "        term2.scrollHeight - term2.scrollTop - term2.clientHeight < 40;";

    s += "    term2.textContent += j.data2;";

    s += "    if(atBottom2)";
    s += "        term2.scrollTop = term2.scrollHeight;";
    s += "}";

    s += "stat.textContent='UART0: '+j.total0+' B    UART2: '+j.total2+' B';";

    s += "    })";
    s += "    .catch(function(){ stat.textContent = 'blad polaczenia...'; })";
    s += "    .finally(function(){ setTimeout(poll, 100); });";
    s += "}";

    s += "function sendCmd(){";

    s += " const c=document.getElementById('cmd').value.trim();";

    s += " if(c.length==0)";
    s += "     return;";

    s += " fetch('/api/serial/send',{";

    s += " method:'POST',";

    s += " body:c";

    s += " });";

    s += " document.getElementById('cmd').value='';";

    s += " document.getElementById('cmd').focus();";

    s += "}";

    s += "document.getElementById('cmd').addEventListener('keydown',function(e){";

    s += " if(e.key==='Enter') sendCmd();";

    s += "});";

    s += "poll();";

    s += "</script>";

    s += "</body>";
    s += "</html>";

    return s;
}

void handleRoot()
{
    server.send(200, "text/html", htmlPage());
}

void handleStatus()
{
    String json;

    json.reserve(512);

    json += "{";

    json += "\"firmware\":\"";
    json += FW_VERSION;
    json += "\",";

    json += "\"ip\":\"";
    json += ETH.localIP().toString();
    json += "\",";

    json += "\"gateway\":\"";
    json += ETH.gatewayIP().toString();
    json += "\",";

    json += "\"subnet\":\"";
    json += ETH.subnetMask().toString();
    json += "\",";

    json += "\"mac\":\"";
    json += ETH.macAddress();
    json += "\",";

    json += "\"ethernet\":\"";
    json += eth_connected ? "CONNECTED" : "DOWN";
    json += "\",";

    json += "\"uptime\":\"";
    json += uptimeString();
    json += "\"";

    json += "}";

    server.send(200, "application/json", json);
}

void handleSerialSend()
{
    if (!server.hasArg("plain"))
    {
        server.send(400, "text/plain", "Missing body");
        return;
    }

    String txt = server.arg("plain");

    txt.trim();

    if (txt.length() == 0)
    {
        server.send(200, "text/plain", "EMPTY");
        return;
    }

    // pokaż w terminalu co wysłaliśmy

    webSerialWriteUART2((const uint8_t*)"\nTX> ",5);
    webSerialWriteUART2((const uint8_t*)txt.c_str(), txt.length());
    webSerialWriteUART2((const uint8_t*)"\n",1);

    Serial2.print(txt);
    Serial2.print("\r\n");

    server.send(200,"text/plain","OK");
}

void handle404()
{
    server.send(404, "text/plain", "404 Not Found");
}

// GET /update - formularz uploadu
void handleUpdatePage()
{
    server.send(200, "text/html", updatePage());
}

// POST /update - odpowiedź po zakończeniu uploadu (sukces/błąd)
void handleUpdateResult()
{
    bool ok = !Update.hasError();

    String s;
    s.reserve(600);

    s += "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    s += "<title>OTA Update</title></head><body style='font-family:Arial;margin:30px;'>";

    if (ok)
    {
        s += "<h2>Update OK</h2>";
        s += "<p>Device is rebooting...</p>";
    }
    else
    {
        s += "<h2>Update FAILED</h2>";
        s += "<p>";
        s += otaWebErrorMsg;
        s += "</p>";
        s += "<p><a href='/update'>Try again</a></p>";
    }

    s += "</body></html>";

    server.sendHeader("Connection", "close");
    server.send(ok ? 200 : 500, "text/html", s);

    otaWebInProgress = false;

    if (ok)
    {
        delay(500);
        ESP.restart();
    }
}

// Handler wywoływany podczas przesyłania kolejnych chunków pliku
void handleUpdateUpload()
{
    HTTPUpload &upload = server.upload();

    if (upload.status == UPLOAD_FILE_START)
    {
        otaWebInProgress = true;
        otaWebError = false;
        otaWebErrorMsg = "";

        Serial.println();
        Serial.println("================================");
        Serial.print("Web OTA: receiving ");
        Serial.println(upload.filename);

        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH))
        {
            otaWebError = true;
            otaWebErrorMsg = "Update.begin() failed - not enough space?";
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (!otaWebError)
        {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            {
                otaWebError = true;
                otaWebErrorMsg = "Write failed";
                Update.printError(Serial);
            }
            else
            {
                static uint32_t lastPrint = 0;

                if (millis() - lastPrint > 500)
                {
                    lastPrint = millis();
                    Serial.printf("Web OTA: %u bytes\n", upload.totalSize);
                }
            }
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (!otaWebError)
        {
            if (Update.end(true))
            {
                Serial.printf("Web OTA: OK, %u bytes\n", upload.totalSize);
            }
            else
            {
                otaWebError = true;
                otaWebErrorMsg = "Update.end() failed";
                Update.printError(Serial);
            }
        }

        Serial.println("================================");
    }
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
        otaWebError = true;
        otaWebErrorMsg = "Upload aborted";
        Update.end();

        Serial.println("Web OTA: aborted");
    }
}

// GET /serial - strona z terminalem
void handleSerialPage()
{
    server.send(200, "text/html", serialPage());
}

// GET /api/serial/data?since=N - zwraca nowe bajty od sekwencji N

void handleSerialData()
{
    uint32_t since0 = 0;
    uint32_t since2 = 0;

    if(server.hasArg("since0"))
        since0 = server.arg("since0").toInt();

    if(server.hasArg("since2"))
        since2 = server.arg("since2").toInt();

    bool overflow0 = false;
    bool overflow2 = false;

    String data0 = webSerialGetSinceUART0(since0, overflow0);
    String data2 = webSerialGetSinceUART2(since2, overflow2);

    String json;

    json.reserve(data0.length()+data2.length()+300);

    json += "{";

    json += "\"total0\":";
    json += since0;

    json += ",\"overflow0\":";
    json += overflow0 ? "true":"false";

    json += ",\"data0\":\"";
    json += jsonEscape(data0);
    json += "\"";

    json += ",\"total2\":";
    json += since2;

    json += ",\"overflow2\":";
    json += overflow2 ? "true":"false";

    json += ",\"data2\":\"";
    json += jsonEscape(data2);
    json += "\"";

    json += "}";

    server.send(200,"application/json",json);
}

// POST /api/serial/clear - czyści bufor terminala
void handleSerialClear()
{
    webSerialClear();

    server.send(200, "text/plain", "OK");
}

void webSetup()
{
    if (webStarted)
        return;

    bootMillis = millis();

    server.on("/", handleRoot);

    server.on("/api/status", handleStatus);

    server.on("/update", HTTP_GET, handleUpdatePage);
    server.on("/update", HTTP_POST, handleUpdateResult, handleUpdateUpload);

    server.on("/serial", HTTP_GET, handleSerialPage);
    server.on("/api/serial/data", HTTP_GET, handleSerialData);
    server.on("/api/serial/clear", HTTP_POST, handleSerialClear);
    server.on("/api/serial/send", HTTP_POST, handleSerialSend);

    server.on("/api/serial/format0",
              HTTP_POST,
              handleSerialFormatUART0);

    server.on("/api/serial/format2",
              HTTP_POST,
              handleSerialFormatUART2);

    server.onNotFound(handle404);

        server.begin();

    webStarted = true;

    Serial.println();
    Serial.println("================================");
    Serial.println("HTTP server started");
    Serial.print("URL: http://");
    Serial.println(ETH.localIP());
    Serial.println("================================");
}

void webLoop()
{
    if (webStarted)
        server.handleClient();
}