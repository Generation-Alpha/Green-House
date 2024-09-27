#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "max6675.h"
#include "DHT.h"

#define DHTTYPE DHT11 
const int DHTPin = 5; //--> The pin used for the DHT11 sensor is Pin D1 = GPIO5
DHT dht(DHTPin, DHTTYPE); 
DHT dht1(D4, DHTTYPE);
int h=0;
float t=0;
int hb=0;
float tb=0;

const int AirValue = 620;   
const int WaterValue = 310;  
int soilMoistureValue = 0;
int soilmoisturepercent=0;
int sm=0;
int sensorPin= A0;

int thermoDO = D0;
int thermoCS = D2;
int thermoCLK = D3;
float tc=0;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);


#define ON_Board_LED 2  

//----------------------------------------SSID and Password of your WiFi router.
const char* ssid = "greenhouse"; 
const char* password = "greenhouse"; 
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; 

String GAS_ID = "AKfycbxEhhqjzcyumpztNNSkrBqyM7tNq1pLcwFqaoOoNfEELu7IDQ8JEnthQcZEgFdxDfrl"; //--> spreadsheet script ID

void setup() 
{
  
  Serial.begin(115200);
  delay(500);

  dht.begin();  
  delay(500);
  
  WiFi.begin(ssid, password); 
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT); 
  digitalWrite(ON_Board_LED, HIGH); 

  
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
   
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); 
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
}

void loop()
{
  Serial.println(" ---------------------- Data from Soil Moisture Sensor ----------------------------");
  soilmoisture();

  Serial.println(" ---------------------- Data from Humidity Sensor ---------------------------------");
  humidity();

  Serial.println(" ---------------------- Data from Thermocouple Sensor -----------------------------");
  thcmpl();
  
  
  sendData(t, h, tc, sm, tb, hb); //--> Calls the sendData Subroutine
}

void sendData(float tem, int hum, float thmcl, int mos, float tembot, int humbot) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  String string_temperature =  String(tem);
  // String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum, DEC); 
  String string_thermocouple= String(thmcl);
  String string_moisture= String(mos,DEC);
  String string_temperaturebot =  String(tembot);
  // String string_temperature =  String(tem, DEC); 
  String string_humiditybot =  String(humbot, DEC); 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity + "&thermocouple=" + string_thermocouple + "&soilmoisture=" + string_moisture + "&temperature_bottle=" + string_temperaturebot + "&humidity_bottle=" + string_humiditybot;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  
} 
void thcmpl()
{
  delay(1000);
  Serial.println("\n");
  tc= thermocouple.readCelsius();
  Serial.println(tc);
  delay(1000);
  
}

void soilmoisture()
{
    soilMoistureValue = analogRead(sensorPin);  //put Sensor insert into soil moisture sensor
    Serial.print("Soil moisture value: ");
    Serial.println(soilMoistureValue);
    soilmoisturepercent = ( 100.00 - ( (analogRead(sensorPin)/1023.00) * 100.00 ) );
    sm= soilmoisturepercent;
    Serial.println(sm);
    Serial.println("\n");
    delay(250);
}

void humidity()
{
  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();

  hb= dht1.readHumidity();
  tb= dht1.readTemperature();
  // Check if any reads failed and exit early (to try again).
  
  }
  String Temp = "Temperature : " + String(t) + " °C";
  String Humi = "Humidity : " + String(h) + " %";
  Serial.println(Temp);
  Serial.println(Humi);

  String Tempbot = "Bottle Temperature : " + String(tb) + " °C";
  String Humibot = "Bottle Humidity : " + String(hb) + " %";
  Serial.println(Tempbot);
  Serial.println(Humibot);
}
