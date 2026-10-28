#include <Adafruit_SH110X.h>
#include <Adafruit_GFX.h>
#include <LoRa.h>

#include <SPI.h>
#include <Wire.h>
// ======================================================
// IDENTIFICACIÓN
// ======================================================

const int MI_ID = 2;

// ======================================================
// OLED
// ======================================================

#define I2C_ADDRESS   0x3C
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ======================================================
// PINES LORA
// ======================================================

// definir los pines del módulo LoRa con #define
//       según la tabla de conexiones.

#define MISO 19
#define MOSI 23
#define SCK 18
#define NSS 5

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("RECEPTOR LORA");

  Serial.print("Mi ID: ");
  Serial.println(MI_ID);

  // ----------------------------------------------------
  // OLED (mensaje de inicio)
  // ----------------------------------------------------

  Wire.begin();
  delay(250);

  if (!display.begin(I2C_ADDRESS, true)) {
    Serial.println("ERROR: OLED no encontrada");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("RECEPTOR LORA");

  display.setCursor(0, 18);
  display.print("ID: ");
  display.println(MI_ID);

  display.setCursor(0, 36);
  display.println("Esperando mensaje...");

  display.display();

  // ----------------------------------------------------
  // LORA
  // ----------------------------------------------------

  // iniciar el bus SPI con SPI.begin(...) usando los pines definidos

  SPI.begin();

  // indicar a la librería los pines del módulo con LoRa.setPins(...)
  //       (NSS, RESET = -1, DIO0 = -1)

  LoRa.setPins(NSS, -1, -1);

  if (!LoRa.begin(915E6)) {

    Serial.println("ERROR: No se pudo iniciar LoRa");

    // OLED (mensaje de error)
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ERROR LORA");
    display.display();

    while (true);
  }

  Serial.println("LoRa iniciado correctamente");
  Serial.println("Esperando mensajes...");
}

// ======================================================
// LOOP
// ======================================================

void loop() {

  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    String paquete = "";

    while (LoRa.available()) {
      paquete += (char)LoRa.read();
    }

    procesarMensaje(paquete);
  }
}

// ======================================================
// PROCESAR MENSAJE
// ======================================================

void procesarMensaje(String paquete) {

  Serial.println();
  Serial.print("Paquete recibido: ");
  Serial.println(paquete);

  // Formato: ORIGEN|DESTINO|MENSAJE
  int separador1 = paquete.indexOf('|');
  int separador2 = paquete.indexOf('|', separador1 + 1);

  if (separador1 == -1 || separador2 == -1) {
    Serial.println("Formato incorrecto");
    return;
  }

  int idOrigen = paquete.substring(0, separador1).toInt();
  int idDestino = paquete.substring(separador1 + 1, separador2).toInt();
  String mensaje = paquete.substring(separador2 + 1);

  Serial.print("Origen: ");
  Serial.println(idOrigen);

  Serial.print("Destino: ");
  Serial.println(idDestino);

  Serial.print("Mensaje cifrado: ");
  Serial.println(mensaje);

  // ----------------------------------------------------
  // COMPROBAR DESTINO
  // ----------------------------------------------------

  if (idDestino != MI_ID) {
    Serial.println("Mensaje ignorado.");
    return;
  }

  int rssi = LoRa.packetRssi();

  Serial.println("Mensaje aceptado");
  Serial.print("RSSI: ");
  Serial.println(rssi);

  // ----------------------------------------------------
  // OLED
  // ----------------------------------------------------

  // limpiar la pantalla y mostrar en la OLED:
  //       - el ID de origen ("Mensaje de ID:")
  //       - el texto "Mensaje cifrado:"
  //       - el mensaje cifrado recibido
  //       - el RSSI

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("ID Mensaje cifrado: ");
  display.println(idDestino);

  display.setCursor(0,16);
  display.println("Mensaje cifrado: ");

  display.setCursor(0, 28);
  display.println(mensaje);

  display.setCursor(15, 54);
  display.print("RSSI: ");
  display.println(rssi);

  display.display(); 
}