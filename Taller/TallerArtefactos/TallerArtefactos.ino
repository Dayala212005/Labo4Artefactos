#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ======================================================
// CONFIGURACION WIFI
// ======================================================

const char* WIFI_SSID = "HONOR90";
const char* WIFI_PASSWORD = "daniel21";

// ======================================================
// CONFIGURACION MQTT
// ======================================================

const char* MQTT_SERVER = "10.190.162.129";
const int MQTT_PORT = 1883;

const char* TOPIC_TEMPERATURA = "clima/esp32/temperatura";
const char* TOPIC_HUMEDAD = "clima/esp32/humedad";
const char* TOPIC_LUZ = "clima/esp32/luz";

// ======================================================
// CONFIGURACION DE SENSORES
// ======================================================

#define DHTPIN 4
#define DHTTYPE DHT11
#define LDR_PIN 34

DHT dht(DHTPIN, DHTTYPE);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const unsigned long INTERVALO_ENVIO = 2000;
unsigned long ultimoEnvio = 0;

// ======================================================
// CONECTAR WIFI
// ======================================================

void conectarWiFi() {

  Serial.print("Conectando a WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");

  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

// ======================================================
// CONECTAR MQTT
// ======================================================

void conectarMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Conectando a Mosquitto...");

    String clientId = "ESP32-";
    clientId += String(
      (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF),
      HEX
    );

    if (mqttClient.connect(clientId.c_str())) {

      Serial.println(" conectado");

    } else {

      Serial.print(" error, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Reintentando en 2 segundos...");

      delay(2000);
    }
  }
}

// ======================================================
// LEER Y PUBLICAR SENSORES
// ======================================================

void publicarSensores() {

  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  int luz = analogRead(LDR_PIN);

  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error al leer el DHT11");
    return;
  }

  char textoTemperatura[12];
  char textoHumedad[12];
  char textoLuz[12];

  snprintf(textoTemperatura, sizeof(textoTemperatura),
           "%.2f", temperatura);

  snprintf(textoHumedad, sizeof(textoHumedad),
           "%.2f", humedad);

  snprintf(textoLuz, sizeof(textoLuz),
           "%d", luz);

  bool envioTemperatura = mqttClient.publish(
    TOPIC_TEMPERATURA, textoTemperatura
  );

  bool envioHumedad = mqttClient.publish(
    TOPIC_HUMEDAD, textoHumedad
  );

  bool envioLuz = mqttClient.publish(
    TOPIC_LUZ, textoLuz
  );

  Serial.println("----- MEDICIONES -----");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");

  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.println(" %");

  Serial.print("Lectura de luz: ");
  Serial.println(luz);

  Serial.print("Envio MQTT: ");

  if (envioTemperatura && envioHumedad && envioLuz) {
    Serial.println("correcto");
  } else {
    Serial.println("error al publicar");
  }

  Serial.println("----------------------");
}

// ======================================================
// CONFIGURACION INICIAL
// ======================================================

void setup() {

  Serial.begin(115200);

  // Inicializar sensores
  dht.begin();
  pinMode(LDR_PIN, INPUT);

  // Conectar WiFi
  conectarWiFi();

  // Configurar servidor MQTT
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}

// ======================================================
// CICLO PRINCIPAL
// ======================================================

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  mqttClient.loop();

  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {

    ultimoEnvio = millis();

    publicarSensores();
  }
}