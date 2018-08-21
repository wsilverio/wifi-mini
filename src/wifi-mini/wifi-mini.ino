
/*
arquivo: wifi-mini.ino
plomoplomo - 2018
*/

///////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>
#include <Adafruit_NeoPixel.h>
#include "modules\plomoplomo_headers\plomoplomo_typedefs.h"

///////////////////////////////////////////////////////
// Defines
///////////////////////////////////////////////////////

#define NDEBUG

#define DATAPIN 14

///////////////////////////////////////////////////////
// Variáveis
///////////////////////////////////////////////////////

// WiFi
const char* ssid = "ssid";
const char* password = "password";
//IPAddress gateway(192, 168, 1, 1);
//IPAddress subnet(255, 255, 255, 0);
//IPAddress ip(192, 168, 1, 52);

// Neopixel
const uint8_t numLeds = 170;
const uint16_t numberOfChannels = numLeds * 3;
Adafruit_NeoPixel leds = Adafruit_NeoPixel(numLeds, DATAPIN, NEO_GRB + NEO_KHZ800);

// Artnet
ArtnetWifi artnet;
const uint16_t startUniverse = 0;
const uint16_t maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];

///////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////

void setup()
{
    delay(250);
    Serial.begin(115200);

#ifndef NDEBUG
    Serial.println("@setup()");
#endif // !NDEBUG

    while (ConnectWifi() != SUCCESS) {
        Serial.println("#Retrying to connect...");
    }

    artnet.begin();

    leds.begin();

    initTest();

    artnet.setArtDmxCallback(onDmxFrame);
}

///////////////////////////////////////////////////////
// Loop
///////////////////////////////////////////////////////

void loop()
{
#ifndef NDEBUG
    static uint32_t i = 0;

    if ((i++ % 100000) == 0) {
        Serial.println("@loop()");
    }
#endif // !NDEBUG

    artnet.read();
}

///////////////////////////////////////////////////////
// Funções
///////////////////////////////////////////////////////

ErrorStatus ConnectWifi(void)
{
#ifndef NDEBUG
    Serial.println("@ConnectWifi()");
#endif // !NDEBUG

    ErrorStatus returnValue = SUCCESS;
    
    uint8_t i = 0;

    WiFi.begin(ssid, password);
    //WiFi.config(ip, gateway, subnet);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);

        if (i++ > 20)
        {
            returnValue = ERROR;
            break;
        }
    }

    if (returnValue == SUCCESS)
    {
        Serial.println("");
        Serial.print("Connected to: ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC: ");
        Serial.println(WiFi.macAddress());
    }
    else
    {
        Serial.println("");
        Serial.println("Connection failed.");
    }

    return returnValue;
}

void initTest()
{
#ifndef NDEBUG
    Serial.println("@initTest()");
#endif // !NDEBUG

    for (uint8_t i = 0; i < numLeds; i++)
    {
        leds.setPixelColor(i, 127, 0, 0);
    }

    leds.show();
    delay(500);

    for (uint8_t i = 0; i < numLeds; i++)
    {
        leds.setPixelColor(i, 0, 127, 0);
    }

    leds.show();
    delay(500);

    for (uint8_t i = 0; i < numLeds; i++)
    {
        leds.setPixelColor(i, 0, 0, 127);
    }

    leds.show();
    delay(500);

    for (uint8_t i = 0; i < numLeds; i++)
    {
        leds.setPixelColor(i, 0, 0, 0);
    }

    leds.show();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
#ifndef NDEBUG
    Serial.println("@onDmxFrame()");
#endif // !NDEBUG

    static uint16_t previousDataLength = 0;

    bool sendFrame = true;

    if (universe == 15) {
        leds.setBrightness(data[0]);
        leds.show();
    }

    // Store which universe has got in
    if ((universe - startUniverse) < maxUniverses) {
        universesReceived[universe - startUniverse] = 1;
    }

    for (int i = 0; i < maxUniverses; i++) {
        if (universesReceived[i] == 0) {
            //Serial.println("Broke");
            sendFrame = 0;
            break;
        }
    }

    // read universe and put into the right part of the display buffer
    for (int i = 0; i < length / 3; i++) {
        int led = i + (universe - startUniverse) * (previousDataLength / 3);

        if (led < numLeds) {
            leds.setPixelColor(led, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
        }
    }

    previousDataLength = length;

    if (sendFrame) {
        leds.show();
        // Reset universeReceived to 0
        memset(universesReceived, 0, maxUniverses);
    }
}
