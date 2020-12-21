/*
    Author: Bruno Lima
    Description: Turn on the builtin led of a ESP32 board when we have the sunset and turn off on the sunrise
*/

//Included libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

//Onboard LEd used to show when it is night
#define LED 2

//Wifi
//Home credencials
//const char *WIFI_SSID = "NOS-DCE0";
//const char *WIFI_PASSWORD = "a0966c563a8c";
const char *WIFI_SSID = "M4I";
const char *WIFI_PASSWORD = "M4Ilda18";

const char *charJSON;

//time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

//Variables for local time
int localHours, localMinutes;
//Variables for sunrise
int sunriseHours, sunriseMinutes;
//Variables for sunset
int sunsetHours, sunsetMinutes;
//Temp string and total times in minutes
String temp;
int totalSunrise, totalSunset,totalLocalTime;

//FUNCTIONS
String httpGETRequest(const char *serverName) {
    HTTPClient http;

    // Your IP address with path or Domain name with URL path
    http.begin(serverName);

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    String payload = "{}";

    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    return payload;
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
  }
  localMinutes = timeinfo.tm_min;
  localHours = timeinfo.tm_hour;

  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");

}

//END FUNCTIONS

//SETUP
void setup() {
    // Set pin mode
    pinMode(LED, OUTPUT);
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
    // Init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
}
//END SETUP

//LOOP
void loop() {
    HTTPClient http;
    //http request to know the sunrise and sunset of Funchal in Madeira Island
    String json = httpGETRequest("https://api.sunrise-sunset.org/json?lat=32.6511&lng=-16.9097&date=today");
    Serial.println("=================================================");

    //USing the ArduinoJson libray to convert to a Json object
    charJSON = json.c_str();
    DynamicJsonDocument doc(768);
    deserializeJson(doc, charJSON);

    JsonObject results = doc["results"];
    const char *results_sunrise = results["sunrise"]; // "8:05:38 AM"
    const char *results_sunset = results["sunset"]; // "6:05:48 PM"
    const char *results_solar_noon = results["solar_noon"]; // "1:05:43 PM"
    const char *results_day_length = results["day_length"]; // "10:00:10"
    const char *results_civil_twilight_begin = results["civil_twilight_begin"]; // "7:38:28 AM"
    const char *results_civil_twilight_end = results["civil_twilight_end"]; // "6:32:57 PM"
    const char *results_nautical_twilight_begin = results["nautical_twilight_begin"]; // "7:07:41 AM"
    const char *results_nautical_twilight_end = results["nautical_twilight_end"]; // "7:03:44 PM"
    const char *results_astronomical_twilight_begin = results["astronomical_twilight_begin"]; // "6:37:37 AM"
    const char *results_astronomical_twilight_end = results["astronomical_twilight_end"]; // "7:33:48 PM"

    Serial.print("Sunrise: ");
    Serial.println(results_sunrise);
    Serial.print("Sunset: ");
    Serial.println(results_sunset);
    Serial.print("Local hours: ");
    Serial.println(localHours);
    Serial.print("Local minutes: ");
    Serial.println(localMinutes);
    //Typical response from the API
    //Sunrise:8:05:38 AM
    //Sunset:6:05:48 PM
    temp = results_sunrise[0];
    sunriseHours = temp.toInt();
    temp = results_sunrise[02];
    temp += results_sunrise[03];
    sunriseMinutes = temp.toInt();
    temp = results_sunset[0];
    sunsetHours = temp.toInt();
    temp = results_sunset[02];
    temp += results_sunset[03];
    Serial.println(temp);
    sunsetMinutes = temp.toInt();

    //Calculate if we should turn ON the LED or not
    totalSunrise = (sunriseHours * 60) + sunriseMinutes;
    totalSunset = (sunsetHours + 12) * 60 + sunsetMinutes;
    totalLocalTime = (localHours * 60)  + localMinutes;
    if (!(totalLocalTime > totalSunrise && totalLocalTime < totalSunset)) {
        digitalWrite(LED,HIGH);
    }
    Serial.println();
    Serial.println("Total sunrise: ");
    Serial.print(totalSunrise);
    Serial.println();
    Serial.println("Total sunset: ");
    Serial.print(totalSunset);
    Serial.println();
    Serial.println("Total Local time: ");
    Serial.print(totalLocalTime);
    Serial.println();

    // Disconnect
    http.end();
    delay(60000);
}
//END LOOP