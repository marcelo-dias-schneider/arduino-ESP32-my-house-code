#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include "my_wifi.h"
#include "system_state.h"

WiFiServer server(80);
static String item;

static void sendJson(WiFiClient &client, int statusCode, const String &body)
{
  client.print("HTTP/1.1 ");
  client.print(statusCode);
  client.print(" ");
  client.println(statusCode == 200 ? "OK" : "Not Found");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.println(body);
}

static void sendHomePage(WiFiClient &client)
{
  String html =
      "<!doctype html>\n"
      "<html lang=\"en\">\n"
      "<head>\n"
      "  <meta charset=\"UTF-8\" />\n"
      "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n"
      "  <title>Marcelo Homes</title>\n"
      "  <style>\n"
      "    body { font-family: Arial, sans-serif; margin: 24px; }\n"
      "    h1 { margin-bottom: 16px; }\n"
      "    .actions { display: flex; gap: 10px; margin-bottom: 12px; }\n"
      "    button { padding: 10px 14px; cursor: pointer; }\n"
      "    #status { font-weight: bold; }\n"
      "  </style>\n"
      "</head>\n"
      "<body>\n"
      "  <h1>Welcome to Marcelo Homes</h1>\n"
      "  <h2>Window</h2>\n"
      "  <div class=\"actions\">\n"
      "    <button data-action=\"/window/open\">Window Open</button>\n"
      "    <button data-action=\"/window/close\">Window Close</button>\n"
      "  </div>\n"
      "  <h2>Yellow LED</h2>\n"
      "  <div class=\"actions\">\n"
      "    <button data-action=\"/yellow-led/off\">LED Off</button>\n"
      "    <button data-action=\"/yellow-led/on\">LED On</button>\n"
      "    <button data-action=\"/yellow-led/blink\">LED Blink</button>\n"
      "  </div>\n"
      "  <h2>NeoPixel</h2>\n"
      "  <div class=\"actions\">\n"
      "    <button data-action=\"/neopixel/off\">Neo Off</button>\n"
      "    <button data-action=\"/neopixel/red\">Neo Red</button>\n"
      "    <button data-action=\"/neopixel/green\">Neo Green</button>\n"
      "    <button data-action=\"/neopixel/blue\">Neo Blue</button>\n"
      "    <button data-action=\"/neopixel/blinkred\">Neo Blink Red</button>\n"
      "    <button data-action=\"/neopixel/blinkgreen\">Neo Blink Green</button>\n"
      "    <button data-action=\"/neopixel/blinkblue\">Neo Blink Blue</button>\n"
      "    <button data-action=\"/neopixel/breathered\">Neo Breath Red</button>\n"
      "    <button data-action=\"/neopixel/breathgreen\">Neo Breath Green</button>\n"
      "    <button data-action=\"/neopixel/breathblue\">Neo Breath Blue</button>\n"
      "  </div>\n"
      "  <h2>Fan Direction</h2>\n"
      "  <div class=\"actions\">\n"
      "    <button data-action=\"/fan/off\">Fan Off</button>\n"
      "    <button data-action=\"/fan/forward\">Fan Forward</button>\n"
      "    <button data-action=\"/fan/reverse\">Fan Reverse</button>\n"
      "  </div>\n"
      "  <p id=\"status\">Ready.</p>\n"
      "\n"
      "  <script>\n"
      "    const baseUrl = 'http://192.168.1.6';\n"
      "    const statusEl = document.getElementById('status');\n"
      "\n"
      "    document.querySelectorAll('[data-action]').forEach((btn) => {\n"
      "      btn.addEventListener('click', async () => {\n"
      "        const action = btn.dataset.action;\n"
      "        statusEl.textContent = 'Sending ' + action + '...';\n"
      "\n"
      "        try {\n"
      "          const response = await fetch(baseUrl + action);\n"
      "          const data = await response.json();\n"
      "          if (!response.ok || data.status !== 'ok') {\n"
      "            throw new Error(data.message || 'Request failed');\n"
      "          }\n"
      "          statusEl.textContent = 'Success: ' + data.message;\n"
      "        } catch (err) {\n"
      "          statusEl.textContent = 'Error: ' + err.message;\n"
      "        }\n"
      "      });\n"
      "    });\n"
      "  </script>\n"
      "</body>\n"
      "</html>\n";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=utf-8");
  client.println("Connection: close");
  client.println();
  client.println(html);
}

void setupWiFi()
{
  const char *ssid = WIFI_SSID;
  const char *password = WIFI_PASSWORD;

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("TCP server started");
  MDNS.addService("http", "tcp", 80);
}

void wifiRequest()
{
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }
  while (client.connected() && !client.available())
  {
    delay(1);
  }
  String req = client.readStringUntil('\r');
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1)
  {
    Serial.print("Invalid request: ");
    Serial.println(req);
    return;
  }
  req = req.substring(addr_start + 1, addr_end);
  item = req;
  Serial.println(item);
  if (req == "/")
  {
    sendHomePage(client);
  }
  else if (req == "/window/open")
  {
    outputs.window.delayMs = 1000;
    outputs.window.action = WindowActuatorState::OPEN;
    Serial.println("Window open requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"Window opening initiated\"}");
  }
  else if (req == "/window/close")
  {
    outputs.window.delayMs = 1000;
    outputs.window.action = WindowActuatorState::CLOSED;
    Serial.println("Window close requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"Window closing initiated\"}");
  }
  else if (req == "/yellow-led/off")
  {
    outputs.yellowLed.action = YellowLedActuatorState::OFF;
    Serial.println("Yellow LED OFF requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"Yellow LED set to OFF\"}");
  }
  else if (req == "/yellow-led/on")
  {
    outputs.yellowLed.action = YellowLedActuatorState::ON;
    Serial.println("Yellow LED ON requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"Yellow LED set to ON\"}");
  }
  else if (req == "/yellow-led/blink")
  {
    outputs.yellowLed.action = YellowLedActuatorState::BLINK;
    Serial.println("Yellow LED BLINK requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"Yellow LED set to BLINK\"}");
  }
  else if (req == "/neopixel/off")
  {
    outputs.ledNeoPixel.action = LedNeoPixelActuatorState::OFF;
    Serial.println("NeoPixel OFF requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"NeoPixel set to OFF\"}");
  }
  else if (req == "/neopixel/red")
  {
    outputs.ledNeoPixel.action = LedNeoPixelActuatorState::RED;
    Serial.println("NeoPixel RED requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"NeoPixel set to RED\"}");
  }
  else if (req == "/neopixel/green")
  {
    outputs.ledNeoPixel.action = LedNeoPixelActuatorState::GREEN;
    Serial.println("NeoPixel GREEN requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"NeoPixel set to GREEN\"}");
  }
  else if (req == "/neopixel/blue")
  {
    outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BLUE;
    Serial.println("NeoPixel BLUE requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"NeoPixel set to BLUE\"}");
  }
  else if (req == "/neopixel/blinkred")
  {
    outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BLINKRED;
    Serial.println("NeoPixel BLINKRED requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"NeoPixel set to BLINKRED\"}");
  }
  else if (req == "/neopixel/blinkgreen")
  {
    outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BLINKGREEN;
    Serial.println("NeoPixel BLINKGREEN requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"NeoPixel set to BLINKGREEN\"}");
  }
  else if (req == "/neopixel/blinkblue")
  {
    outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BLINKBLUE;
    Serial.println("NeoPixel BLINKBLUE requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"NeoPixel set to BLINKBLUE\"}");
  }
  else if (req == "/neopixel/breathered")
  {
    outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BREATHERED;
    Serial.println("NeoPixel BREATHERED requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"NeoPixel set to BREATHERED\"}");
  }
  else if (req == "/neopixel/breathgreen")
  {
    outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BREATHGREEN;
    Serial.println("NeoPixel BREATHGREEN requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"NeoPixel set to BREATHGREEN\"}");
  }
  else if (req == "/neopixel/breathblue")
  {
    outputs.ledNeoPixel.action = LedNeoPixelActuatorState::BREATHBLUE;
    Serial.println("NeoPixel BREATHBLUE requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"NeoPixel set to BREATHBLUE\"}");
  }
  else if (req == "/fan/off")
  {
    outputs.fan.directionAction = FanDirectionActuatorState::OFF;
    outputs.fan.speedAction = 0;
    Serial.println("Fan OFF requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"Fan set to OFF\"}");
  }
  else if (req == "/fan/forward")
  {
    outputs.fan.directionAction = FanDirectionActuatorState::FORWARD;
    outputs.fan.speedAction = 200;
    Serial.println("Fan FORWARD requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"Fan set to FORWARD\"}");
  }
  else if (req == "/fan/reverse")
  {
    outputs.fan.directionAction = FanDirectionActuatorState::REVERSE;
    outputs.fan.speedAction = 200;
    Serial.println("Fan REVERSE requested");
    sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"Fan set to REVERSE\"}");
  }
  else
  {
    Serial.print("Unknown request: ");
    Serial.println(req);
    sendJson(client, 404, "{\"status\":\"error\",\"message\":\"Unknown route\"}");
  }

  client.stop();
}