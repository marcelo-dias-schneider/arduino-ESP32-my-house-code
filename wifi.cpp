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

static int fromHexChar(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return 10 + (c - 'A');
  if (c >= 'a' && c <= 'f')
    return 10 + (c - 'a');
  return -1;
}

static String decodeUrlComponent(const String &encoded)
{
  String decoded;
  decoded.reserve(encoded.length());

  for (int i = 0; i < encoded.length(); i++)
  {
    char ch = encoded[i];
    if (ch == '+')
    {
      decoded += ' ';
    }
    else if (ch == '%' && i + 2 < encoded.length())
    {
      int hi = fromHexChar(encoded[i + 1]);
      int lo = fromHexChar(encoded[i + 2]);
      if (hi >= 0 && lo >= 0)
      {
        decoded += char((hi << 4) | lo);
        i += 2;
      }
      else
      {
        decoded += ch;
      }
    }
    else
    {
      decoded += ch;
    }
  }

  return decoded;
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
      "    * { box-sizing: border-box; }\n"
      "    body { font-family: Arial, sans-serif; padding: 24px; }\n"
      "    h1 { margin-bottom: 16px; }\n"
      "    .actions { display: flex; gap: 10px; margin-bottom: 12px; flex-wrap: wrap; }\n"
      "    button { padding: 10px 14px; cursor: pointer; }\n"
      "    .callout { position: fixed; right: 24px; bottom: 24px; min-width: 280px; max-width: 360px; background: #f5f7fb; border: 1px solid #cfd8e3; border-radius: 10px; box-shadow: 0 8px 22px rgba(0, 0, 0, 0.12); overflow: hidden; }\n"
      "    .callout.hidden { display: none; }\n"
      "    .callout.error { background: #fff4f4; border-color: #e8b8b8; }\n"
      "    .callout-content { padding: 12px 14px 10px; font-weight: bold; }\n"
      "    .callout-bar { height: 4px; width: 100%; background: #21a366; transition: width 0.05s linear; }\n"
      "    .callout.error .callout-bar { background: #d64545; }\n"
      "  </style>\n"
      "</head>\n"
      "<body>\n"
      "  <h1>Welcome to Marcelo Homes</h1>\n"
      "  <p>Window</p>\n"
      "  <div class=\"actions\">\n"
      "    <button data-action=\"/window/open\">Window Open</button>\n"
      "    <button data-action=\"/window/close\">Window Close</button>\n"
      "  </div>\n"
      "  <p>Yellow LED</p>\n"
      "  <div class=\"actions\">\n"
      "    <button data-action=\"/yellow-led/off\">LED Off</button>\n"
      "    <button data-action=\"/yellow-led/on\">LED On</button>\n"
      "    <button data-action=\"/yellow-led/blink\">LED Blink</button>\n"
      "  </div>\n"
      "  <p>NeoPixel</p>\n"
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
      "  <p>Fan Direction</p>\n"
      "  <div class=\"actions\">\n"
      "    <button data-action=\"/fan/off\">Fan Off</button>\n"
      "    <button data-action=\"/fan/forward\">Fan Forward</button>\n"
      "    <button data-action=\"/fan/reverse\">Fan Reverse</button>\n"
      "  </div>\n"
      "  <p>LCD Message</p>\n"
      "  <div class=\"actions\">\n"
      "    <input id=\"lcdMessage\" type=\"text\" placeholder=\"Type LCD message\" maxlength=\"16\" />\n"
      "    <button id=\"sendMessageBtn\" type=\"button\">Send Message</button>\n"
      "  </div>\n"
      "  <small>Max 16 characters (LCD width).</small>\n"
      "  <div id=\"statusCallout\" class=\"callout hidden\">\n"
      "    <div id=\"statusText\" class=\"callout-content\">Ready.</div>\n"
      "    <div id=\"statusBar\" class=\"callout-bar\"></div>\n"
      "  </div>\n"
      "\n"
      "  <script>\n"
      "    const baseUrl = 'http://192.168.1.6';\n"
      "    const statusCallout = document.getElementById('statusCallout');\n"
      "    const statusText = document.getElementById('statusText');\n"
      "    const statusBar = document.getElementById('statusBar');\n"
      "    const lcdInput = document.getElementById('lcdMessage');\n"
      "    const sendMessageBtn = document.getElementById('sendMessageBtn');\n"
      "    const lcdMaxLength = 16;\n"
      "    let statusTimer = null;\n"
      "\n"
      "    function showStatus(message, isError = false) {\n"
      "      if (statusTimer) {\n"
      "        clearInterval(statusTimer);\n"
      "        statusTimer = null;\n"
      "      }\n"
      "\n"
      "      statusCallout.classList.remove('hidden');\n"
      "      statusCallout.classList.toggle('error', isError);\n"
      "      statusText.textContent = message;\n"
      "      statusBar.style.width = '100%';\n"
      "\n"
      "      const durationMs = 4000;\n"
      "      const tickMs = 40;\n"
      "      let remaining = durationMs;\n"
      "\n"
      "      statusTimer = setInterval(() => {\n"
      "        remaining -= tickMs;\n"
      "        if (remaining <= 0) {\n"
      "          clearInterval(statusTimer);\n"
      "          statusTimer = null;\n"
      "          statusBar.style.width = '0%';\n"
      "          statusCallout.classList.add('hidden');\n"
      "          return;\n"
      "        }\n"
      "\n"
      "        const pct = (remaining / durationMs) * 100;\n"
      "        statusBar.style.width = pct + '%';\n"
      "      }, tickMs);\n"
      "    }\n"
      "\n"
      "    document.querySelectorAll('[data-action]').forEach((btn) => {\n"
      "      btn.addEventListener('click', async () => {\n"
      "        const action = btn.dataset.action;\n"
      "        showStatus('Sending ' + action + '...');\n"
      "\n"
      "        try {\n"
      "          const response = await fetch(baseUrl + action);\n"
      "          const data = await response.json();\n"
      "          if (!response.ok || data.status !== 'ok') {\n"
      "            throw new Error(data.message || 'Request failed');\n"
      "          }\n"
      "          showStatus('Success: ' + data.message);\n"
      "        } catch (err) {\n"
      "          showStatus('Error: ' + err.message, true);\n"
      "        }\n"
      "      });\n"
      "    });\n"
      "\n"
      "    sendMessageBtn.addEventListener('click', async () => {\n"
      "      const rawMessage = lcdInput.value.trim();\n"
      "\n"
      "      if (!rawMessage) {\n"
      "        showStatus('Error: Message cannot be empty', true);\n"
      "        return;\n"
      "      }\n"
      "\n"
      "      if (rawMessage.length > lcdMaxLength) {\n"
      "        showStatus('Error: Max ' + lcdMaxLength + ' chars for LCD', true);\n"
      "        return;\n"
      "      }\n"
      "\n"
      "      const action = '/message/' + encodeURIComponent(rawMessage);\n"
      "      showStatus('Sending ' + action + '...');\n"
      "\n"
      "      try {\n"
      "        const response = await fetch(baseUrl + action);\n"
      "        const data = await response.json();\n"
      "        if (!response.ok || data.status !== 'ok') {\n"
      "          throw new Error(data.message || 'Request failed');\n"
      "        }\n"
      "        showStatus('Success: ' + data.message);\n"
      "        lcdInput.value = '';\n"
      "      } catch (err) {\n"
      "        showStatus('Error: ' + err.message, true);\n"
      "      }\n"
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
  else if (req.startsWith("/message/"))
  {
    String encodedMessage = req.substring(9);
    String message = decodeUrlComponent(encodedMessage);

    if (message.length() == 0)
    {
      sendJson(client, 400, "{\"status\":\"error\",\"message\":\"Message cannot be empty\"}");
    }
    else if (message.length() > LCD_COLUMNS)
    {
      sendJson(client, 400, "{\"status\":\"error\",\"message\":\"Message too long for LCD\"}");
    }
    else
    {
      outputs.lcd.count = 1;
      outputs.lcd.messages[0] = message;
      outputs.lcd.delayTime = 2000;
      outputs.lcd.currentHash = String(millis());

      Serial.println("LCD message requested: " + message);
      sendJson(client, 200, "{\"status\":\"ok\",\"message\":\"LCD message updated\"}");
    }
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