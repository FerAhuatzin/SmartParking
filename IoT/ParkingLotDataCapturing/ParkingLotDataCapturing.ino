/*
 * Sistema de Estacionamiento Inteligente
 * Desarrollado para ESP32 con sensores ultrasónicos HC-SR04
 * Monitoreo de 3 espacios de estacionamiento con envío de datos a Firebase
 */

#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <time.h>

// Configuración de WiFi
#define WIFI_SSID "TOTALPLAY_D706CB"           // Cambiar por tu SSID
#define WIFI_PASSWORD "12345678"    // Cambiar por tu contraseña

// Configuración de Firebase
#define API_KEY "AIzaSyAU19O3Vpbzz9TWRI-qvBkDJlRr_1_ztNI"                          // Cambiar por tu API Key de Firebase
#define DATABASE_URL "https://smart-parking-ca5f8-default-rtdb.firebaseio.com"  // Cambiar por la URL de tu base de datos
#define USER_EMAIL "f126ag@gmail.com"             // Cambiar por tu email de Firebase
#define USER_PASSWORD "password123"          // Cambiar por tu contraseña de Firebase

// Definición de pines para los sensores ultrasónicos
#define TRIGGER_PIN_1 5
#define ECHO_PIN_1 18
#define TRIGGER_PIN_2 19
#define ECHO_PIN_2 21
#define TRIGGER_PIN_3 22
#define ECHO_PIN_3 23

// Umbral de distancia para considerar ocupado un lugar (en cm)
#define THRESHOLD_DISTANCE 14

// Variables para Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variables para el control de tiempo
unsigned long lastUploadTime = 0;
const unsigned long uploadInterval = 1000; // Intervalo para subir datos (2 segundos)
unsigned long lastHistoricalUpdate = 0;
const unsigned long historicalInterval = 60000; // Actualizar historial cada minuto (60000 ms)

// Variables para almacenar el estado de los espacios
bool espacio1Ocupado = false;
bool espacio2Ocupado = false;
bool espacio3Ocupado = false;

int horaFalsa = 0;

// Variables para estadísticas con medición continua
unsigned long tiempoOcupado1 = 0;
unsigned long tiempoOcupado2 = 0;
unsigned long tiempoOcupado3 = 0;
unsigned long ultimaMedicion = 0;

// Función para medir la distancia con un sensor ultrasónico
float medirDistancia(int triggerPin, int echoPin) {
  // Enviar pulso ultrasónico
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  
  // Medir el tiempo que tarda en volver el eco
  long duracion = pulseIn(echoPin, HIGH);
  
  // Calcular la distancia en cm
  float distancia = duracion * 0.034 / 2;
  
  return distancia;
}

// Función para obtener la hora actual
String obtenerHoraActual() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error al obtener la hora");
    return "Error";
  }
  
  char timeString[9];
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo); // Formato hora completa
  return String(timeString);
}

// Obtener solo la hora (sin minutos ni segundos)
String obtenerHoraSinMinutos() {
  /*struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error al obtener la hora");
    return "Error";
  }
  
  char timeString[9];
  strftime(timeString, sizeof(timeString), "%H:00:00", &timeinfo);
  
  */
  char timeString[9];
  sprintf(timeString, "%02d:00:00", horaFalsa);
  horaFalsa = (horaFalsa + 1) % 24;
  return String(timeString);
}

// Función para obtener la fecha actual (YYYY-MM-DD)
String obtenerFechaActual() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error al obtener la fecha");
    return "Error";
  }
  
  char dateString[11];
  strftime(dateString, sizeof(dateString), "%Y-%m-%d", &timeinfo);
  
  return String(dateString);
}

void setup() {
  // Iniciar comunicación serial y esperar a que esté lista
  Serial.begin(115200);
  delay(1000); // Esperar un segundo para que el puerto serial se inicialice
  
  Serial.println("\n\n----- Sistema de Estacionamiento Inteligente -----");
  Serial.println("Iniciando configuración...");
  
  // Configurar pines de los sensores
  pinMode(TRIGGER_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIGGER_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(TRIGGER_PIN_3, OUTPUT);
  pinMode(ECHO_PIN_3, INPUT);
  
  // Conectar a WiFi
  Serial.print("Conectando a WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    Serial.print(".");
    delay(500);
    intentos++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Conectado con IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("ERROR: No se pudo conectar a WiFi. Verificar credenciales.");
    // Continuar de todos modos para pruebas sin WiFi
  }
  
  // Configurar hora
  Serial.println("Configurando hora del sistema...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  
  // Configurar Firebase
  Serial.println("Configurando Firebase...");
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  // Inicializar tiempo de última medición
  ultimaMedicion = millis();
  
  Serial.println("Sistema de estacionamiento inteligente iniciado correctamente");
  Serial.println("Comenzando monitoreo de espacios...");
  Serial.println("---------------------------------------------");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Medir distancias de los sensores
  float distancia1 = medirDistancia(TRIGGER_PIN_1, ECHO_PIN_1);
  float distancia2 = medirDistancia(TRIGGER_PIN_2, ECHO_PIN_2);
  float distancia3 = medirDistancia(TRIGGER_PIN_3, ECHO_PIN_3);
  
  // Determinar si los espacios están ocupados
  bool nuevoEstado1 = (distancia1 < THRESHOLD_DISTANCE);
  bool nuevoEstado2 = (distancia2 < THRESHOLD_DISTANCE);
  bool nuevoEstado3 = (distancia3 < THRESHOLD_DISTANCE);
  
  // Calcular tiempo transcurrido desde la última medición
  unsigned long tiempoTranscurrido = currentMillis - ultimaMedicion;
  
  // Acumular tiempo de ocupación continuamente para cada espacio ocupado
  if (espacio1Ocupado) {
    tiempoOcupado1 += tiempoTranscurrido;
  }
  
  if (espacio2Ocupado) {
    tiempoOcupado2 += tiempoTranscurrido;
  }
  
  if (espacio3Ocupado) {
    tiempoOcupado3 += tiempoTranscurrido;
  }
  
  // Actualizar el tiempo de última medición
  ultimaMedicion = currentMillis;
  
  // Actualizar los estados
  bool cambioEstado1 = (nuevoEstado1 != espacio1Ocupado);
  bool cambioEstado2 = (nuevoEstado2 != espacio2Ocupado);
  bool cambioEstado3 = (nuevoEstado3 != espacio3Ocupado);
  
  espacio1Ocupado = nuevoEstado1;
  espacio2Ocupado = nuevoEstado2;
  espacio3Ocupado = nuevoEstado3;
  
  // Mostrar lecturas de sensores cada 2 segundos
  static unsigned long lastPrintTime = 0;
  if (currentMillis - lastPrintTime >= 2000) {
    lastPrintTime = currentMillis;
    
    Serial.println("\n----- Lecturas de sensores -----");
    Serial.print("Espacio 1: ");
    Serial.print(distancia1);
    Serial.print(" cm - ");
    Serial.println(espacio1Ocupado ? "OCUPADO" : "LIBRE");
    
    Serial.print("Espacio 2: ");
    Serial.print(distancia2);
    Serial.print(" cm - ");
    Serial.println(espacio2Ocupado ? "OCUPADO" : "LIBRE");
    
    Serial.print("Espacio 3: ");
    Serial.print(distancia3);
    Serial.print(" cm - ");
    Serial.println(espacio3Ocupado ? "OCUPADO" : "LIBRE");
    
    // Mostrar tiempo acumulado de ocupación
    Serial.println("\n----- Tiempo de ocupación acumulado -----");
    Serial.print("Espacio 1: ");
    Serial.print(tiempoOcupado1 / 1000.0);
    Serial.println(" segundos");
    
    Serial.print("Espacio 2: ");
    Serial.print(tiempoOcupado2 / 1000.0);
    Serial.println(" segundos");
    
    Serial.print("Espacio 3: ");
    Serial.print(tiempoOcupado3 / 1000.0);
    Serial.println(" segundos");
  }
  
  // Notificar cambios de estado cuando ocurran
  if (cambioEstado1) {
    Serial.print("Cambio de estado en Espacio 1: ");
    Serial.println(espacio1Ocupado ? "OCUPADO" : "LIBRE");
  }
  
  if (cambioEstado2) {
    Serial.print("Cambio de estado en Espacio 2: ");
    Serial.println(espacio2Ocupado ? "OCUPADO" : "LIBRE");
  }
  
  if (cambioEstado3) {
    Serial.print("Cambio de estado en Espacio 3: ");
    Serial.println(espacio3Ocupado ? "OCUPADO" : "LIBRE");
  }
  
  // Subir datos a Firebase cada 10 segundos
  if (currentMillis - lastUploadTime >= uploadInterval) {
    lastUploadTime = currentMillis;
    
    // Actualizar datos en tiempo real
    if (Firebase.ready()) {
      Serial.println("\n----- Enviando datos a Firebase -----");
      
      // Subir estados actuales
      bool result1 = Firebase.setBool(fbdo, "/estacionamientos/espacios/1/ocupado", espacio1Ocupado);
      bool result2 = Firebase.setBool(fbdo, "/estacionamientos/espacios/2/ocupado", espacio2Ocupado);
      bool result3 = Firebase.setBool(fbdo, "/estacionamientos/espacios/3/ocupado", espacio3Ocupado);
      
      if (result1 && result2 && result3) {
        Serial.println("Estados actualizados correctamente");
      } else {
        Serial.println("Error al actualizar estados: " + fbdo.errorReason());
      }
      
      // Subir distancias actuales (para depuración)
      Firebase.setFloat(fbdo, "/estacionamientos/espacios/1/distancia", distancia1);
      Firebase.setFloat(fbdo, "/estacionamientos/espacios/2/distancia", distancia2);
      Firebase.setFloat(fbdo, "/estacionamientos/espacios/3/distancia", distancia3);
      
      // Calcular disponibilidad
      int disponibles = (!espacio1Ocupado ? 1 : 0) + (!espacio2Ocupado ? 1 : 0) + (!espacio3Ocupado ? 1 : 0);
      Firebase.setInt(fbdo, "/estacionamientos/disponibles", disponibles);
      Firebase.setInt(fbdo, "/estacionamientos/ocupados", 3 - disponibles);
      
      // Timestamp de última actualización
      String timestamp = obtenerFechaActual() + " " + obtenerHoraActual();
      Firebase.setString(fbdo, "/estacionamientos/ultima_actualizacion", timestamp);
      
      Serial.print("Espacios disponibles: ");
      Serial.println(disponibles);
      Serial.print("Última actualización: ");
      Serial.println(timestamp);
    } else {
      Serial.println("Error en conexión con Firebase. Verificar credenciales y conexión.");
    }
  }
  
  // Actualizar datos históricos cada intervalo definido
  if (currentMillis - lastHistoricalUpdate >= historicalInterval) {
    lastHistoricalUpdate = currentMillis;
    
    if (Firebase.ready()) {
      String fecha = obtenerFechaActual();
      String hora = obtenerHoraSinMinutos();
      String path = "/estacionamientos/historico/" + fecha + "/" + hora;
      
      Serial.println("\n----- Actualizando datos históricos -----");
      Serial.print("Fecha: ");
      Serial.print(fecha);
      Serial.print(" Hora: ");
      Serial.println(hora);
      
      // Mostrar tiempo acumulado
      Serial.print("Tiempo ocupado Espacio1: ");
      Serial.print(tiempoOcupado1 / 1000.0);
      Serial.println(" segundos");
      
      Serial.print("Tiempo ocupado Espacio2: ");
      Serial.print(tiempoOcupado2 / 1000.0);
      Serial.println(" segundos");
      
      Serial.print("Tiempo ocupado Espacio3: ");
      Serial.print(tiempoOcupado3 / 1000.0);
      Serial.println(" segundos");
      
      // Porcentaje de ocupación en el período
      int porcentajeOcupado1 = (tiempoOcupado1 * 100) / historicalInterval;
      int porcentajeOcupado2 = (tiempoOcupado2 * 100) / historicalInterval;
      int porcentajeOcupado3 = (tiempoOcupado3 * 100) / historicalInterval;
      
      Serial.print("Porcentaje ocupación Espacio 1: ");
      Serial.print(porcentajeOcupado1);
      Serial.println("%");
      
      Serial.print("Porcentaje ocupación Espacio 2: ");
      Serial.print(porcentajeOcupado2);
      Serial.println("%");
      
      Serial.print("Porcentaje ocupación Espacio 3: ");
      Serial.print(porcentajeOcupado3);
      Serial.println("%");
      
      // Guardar datos históricos
      Firebase.setInt(fbdo, path + "/espacio1", porcentajeOcupado1);
      Firebase.setInt(fbdo, path + "/espacio2", porcentajeOcupado2);
      Firebase.setInt(fbdo, path + "/espacio3", porcentajeOcupado3);
      Firebase.setInt(fbdo, path + "/promedio", (porcentajeOcupado1 + porcentajeOcupado2 + porcentajeOcupado3) / 3);
      
      // También guardar el tiempo acumulado en milisegundos (para depuración)
      Firebase.setInt(fbdo, path + "/tiempo_ms1", tiempoOcupado1);
      Firebase.setInt(fbdo, path + "/tiempo_ms2", tiempoOcupado2);
      Firebase.setInt(fbdo, path + "/tiempo_ms3", tiempoOcupado3);
      
      Serial.println("Datos históricos actualizados correctamente");
      
      // Reiniciar contadores de tiempo después de guardar datos históricos
      tiempoOcupado1 = 0;
      tiempoOcupado2 = 0;
      tiempoOcupado3 = 0;
    }
  }
  
  // Pequeña pausa para estabilidad
  delay(200);
}