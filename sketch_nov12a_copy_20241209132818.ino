#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// Credenciales WiFi
const char* ssid = "RUIZ-AGUDELO";
const char* password = "1035916534H";

// URL del servidor
const char* dataServer = "https://guillermo.tecnoparquerionegro.com/post-esp-data.php";
const char* controlServer = "https://guillermo.tecnoparquerionegro.com/control-led.php";

// Clave API
String apiKeyValue = "tPmAT5Ab3j7F9";

// Configuración del LED
const int ledPin = 32; // Pin del LED (ánodo conectado al GPIO 32)

// Pin de la fotoresistencia
const int photoResistorPin = 34; // Pin analógico D34
const int lightThreshold = 1; // Umbral de luminosidad

void setup() {
  Serial.begin(115200);
  
  // Configurar el pin de la fotoresistencia y el LED
  pinMode(photoResistorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void loop() {
  // Verificar si hay solicitudes de control
  handleControlRequest();

  // Enviar datos al servidor
  sendSensorData();

  delay(30000); // Esperar 30 segundos antes de la próxima lectura
}

void sendSensorData() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setInsecure();
    HTTPClient https;

    https.begin(*client, dataServer);
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int lightValue = analogRead(photoResistorPin); // Leer el valor del sensor
    bool isDay = lightValue > lightThreshold;     // Evaluar si es de día según el umbral

    String httpRequestData = "api_key=" + apiKeyValue +
                             "&sensor=Photoresistor" +
                             "&location=Room" +
                             "&lux_value=" + String(lightValue) +
                             "&is_day=" + String(isDay ? "1" : "0");

    int httpResponseCode = https.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("Data sent, HTTP response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending data, code: ");
      Serial.println(httpResponseCode);
    }
    https.end();
  }
}

void handleControlRequest() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setInsecure();
    HTTPClient https;

    https.begin(*client, controlServer);
    int httpResponseCode = https.GET();

    if (httpResponseCode == 200) {
      String payload = https.getString();
      Serial.println("Response from server: " + payload); // Mostrar la respuesta del servidor
      if (payload == "ON") {
        digitalWrite(ledPin, HIGH); // Encender el LED
        Serial.println("LED ON");
      } else if (payload == "OFF") {
        digitalWrite(ledPin, LOW); // Apagar el LED
        Serial.println("LED OFF");
      }
    } else {
      Serial.print("Failed to fetch control, HTTP code: ");
      Serial.println(httpResponseCode);
    }
    https.end();
  }
}
