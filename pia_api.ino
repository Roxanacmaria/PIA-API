#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
String URL1="http://proiectia.bogdanflorea.ro/api/the-meal-db/recipes";
String URL2="http://proiectia.bogdanflorea.ro/api/the-meal-db/recipe?idMeal=";
bool connect(String ssid, String password)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid.c_str(), password.c_str());
  int startConection=millis();
  while(WiFi.status()!=WL_CONNECTED)
  {
    delay(500);
    if(millis()-startConection>10000)
      return false;
  }
  return true;
}
void handleCommand(String jsonData)
{
  StaticJsonDocument<1024> jsonDoc;
  DeserializationError error=deserializeJson(jsonDoc, jsonData);
  if(error)
  {
    SerialBT.println("{\"error\":\"Invalid JSON\"}");
    return;
  }
  String action=jsonDoc["action"].as<String>();
  String response;
  if(action=="getNetworks")
  {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    int networksFound=WiFi.scanNetworks();
    for(int i=0;i<networksFound;++i)
    {
      StaticJsonDocument<256>net;
      net["ssid"]=WiFi.SSID(i);
      net["strength"]=WiFi.RSSI(i);
      net["encryption"]=WiFi.encryptionType(i);
      net["teamId"]="A19";
      serializeJson(net,response);
      SerialBT.println(response);
    }
  }
  else if(action=="connect")
  {
    String ssid=jsonDoc["ssid"].as<String>();
    String password=jsonDoc["password"].as<String>();
    bool connected=connect(ssid,password);
    StaticJsonDocument<256> net;
    net["teamId"]="A19";
    net["ssid"]=ssid;
    net["connected"]=connected;
    serializeJson(net, response);
    SerialBT.println(response);
  }
  else if(action=="getData")
  {
    HTTPClient http;
    http.begin(URL1);
    int code=http.GET();
    if(code==200)
    {
      String payload=http.getString();
      StaticJsonDocument<2048> doc;
      deserializeJson(doc,payload);
      JsonArray arr=doc.as<JsonArray>();
      for(JsonVariant v : arr)
      {
        StaticJsonDocument<256> item;
        item["id"]=v["idMeal"];
        item["name"]=v["strMeal"];
        item["image"]=v["mealThumb"];
        item["teamId"]="A19";
        serializeJson(item,response);
        SerialBT.println(response);

      }
    }
    http.end();
  }
  else if(action=="getDetails")
  {
    String id=jsonDoc["id"].as<String>();
    HTTPClient http;
    http.begin(URL2+id);
    int code=http.GET();
    if(code>0)
    {
      String payload=http.getString();
      StaticJsonDocument<2048>doc;
      deserializeJson(doc,payload);
      StaticJsonDocument<512> res;
      res["id"]=doc["idMeal"];
      res["name"]=doc["strMeal"];
      res["image"]=doc["mealThumb"];
      res["teamId"]="A19";
      String description="";
      description+="Name: "+String(doc["strMeal"].as<const char*>())+"\n";
      description+="Category: "+String(doc["strCategory"].as<const char*>())+"\n";
      description+="Area: "+String(doc["strInstructions"].as<const char*>())+"\n";
      description+="Video: "+String(doc["strYoutube"].as<const char*>())+"\n";
      res["description"]=description;
      serializeJson(res,response);
      SerialBT.println(response);
    }
    http.end();
  }
}
void setup()
{
  Serial.begin(115200);
  SerialBT.begin("ESP32_TheMealDB");
  Serial.println("Bluetooth classic started");
}
void loop()
{
  if(SerialBT.available())
  {
    String input=SerialBT.readStringUntil('\n');
    handleCommand(input);
  }
}


