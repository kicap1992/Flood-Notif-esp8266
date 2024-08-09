#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define WATER_SENSOR_1_PIN D2 // First water level sensor
#define WATER_SENSOR_2_PIN D5 // Second water level sensor
#define TRIG_PIN D3           // Ultrasonic sensor trig pin
#define ECHO_PIN D4           // Ultrasonic sensor echo pin
#define LED_PIN D0 // Using D0 (GPIO16) for the LED

const int ledPin = 5;

const float SCALE_FACTOR = 10.0; // Scale factor for 10 cm to 1 meter

// wifi
// const char *ssid = "Bismillah";
// const char *password = "1234567890";
const char *ssid = "KARAN";
const char *password = "12345679";

WiFiClient client;

void setup()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  pinMode(WATER_SENSOR_1_PIN, INPUT);
  pinMode(WATER_SENSOR_2_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(ledPin, LOW);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  Serial.println(WiFi.localIP());

  Serial.println("Starting up...");

  digitalWrite(ledPin, HIGH);
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(ledPin, HIGH);
    // Check water level sensors
    int waterSensor1State = digitalRead(WATER_SENSOR_1_PIN);
    int waterSensor2State = digitalRead(WATER_SENSOR_2_PIN);

    if (waterSensor2State == HIGH)
    {
      Serial.println("Danger: Water level at full height!");
    }
    else if (waterSensor1State == HIGH)
    {
      Serial.println("Warning: Water level reaching high!");
    }
    else
    {
      Serial.println("Normal: Water level is below warning level.");
    }

    // Measure distance using ultrasonic sensor
    long duration, distance;
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    distance = duration * 0.034 / 2; // Calculate the distance in cm
    float water_height = 0;

    if (waterSensor2State == HIGH)
    {

      // Adjust distance for simulation scale
      float scaledDistance = distance * SCALE_FACTOR;
      // Calculate the water height
      float waterHeight = 100 - scaledDistance; // Assuming sensor is 100 cm above the water base
      Serial.print("Water Height: ");
      Serial.print(waterHeight / 100); // Convert to meters
      Serial.println(" m");

      water_height = waterHeight / 100;
    }
    
    String jsonSend = "{\"water_height\": " + String(water_height) + ", \"warning_level\": " + String(waterSensor1State) + ", \"danger_level\": " + String(waterSensor2State) + "}";
    Serial.println(jsonSend);

    HTTPClient http;

    // http.begin(client, "http://192.168.20.45:3005/");
    http.begin(client, "http://banjir-notif.kicap-karan.com/");

    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonSend);

    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    delay(1000);
  }
  else
  {
    Serial.println("WiFi is disconnected, attempting to reconnect");
    digitalWrite(ledPin, LOW);
    WiFi.begin(ssid, password);

    // Wait for connection
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20)
    { // wait for 10 seconds max
      delay(500);
      Serial.print(".");
      timeout++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("");
      Serial.println("Reconnected to WiFi");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      digitalWrite(ledPin, LOW); // Turn on LED to indicate Wi-Fi connected
    }
    else
    {
      Serial.println("");
      Serial.println("Failed to reconnect to WiFi");
    }
  }

  delay(1000);
  // Wait for 1 second before taking another reading
}
