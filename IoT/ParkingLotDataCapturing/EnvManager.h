#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

// Clase para manejar variables de entorno
class EnvManager {
private:
    // Estructura para almacenar pares clave-valor
    struct EnvVar {
        String key;
        String value;
    };
    
    // Array para almacenar las variables cargadas
    EnvVar* variables;
    int varCount;
    int maxVars;
    
    // Método para extraer el valor de una línea de env
    String parseEnvLine(String line) {
        // Eliminar comentarios
        int commentPos = line.indexOf('#');
        if (commentPos >= 0) {
            line = line.substring(0, commentPos);
        }
        
        // Verificar si la línea tiene un formato clave=valor
        int equalsPos = line.indexOf('=');
        if (equalsPos < 0) {
            return "";
        }
        
        String key = line.substring(0, equalsPos);
        String value = line.substring(equalsPos + 1);
        
        // Eliminar espacios en blanco y comillas
        key.trim();
        value.trim();
        
        // Eliminar comillas del valor si existen
        if (value.startsWith("\"") && value.endsWith("\"")) {
            value = value.substring(1, value.length() - 1);
        }
        
        // Si la clave es válida, guardarla
        if (key.length() > 0) {
            addVariable(key, value);
        }
        
        return key;
    }
    
    // Añadir una variable al array
    void addVariable(String key, String value) {
        // Verificar si ya existe la clave
        for (int i = 0; i < varCount; i++) {
            if (variables[i].key == key) {
                variables[i].value = value;
                return;
            }
        }
        
        // Si no existe y hay espacio, añadirla
        if (varCount < maxVars) {
            variables[varCount].key = key;
            variables[varCount].value = value;
            varCount++;
        }
    }
    
public:
    EnvManager(int maxVariables = 20) {
        maxVars = maxVariables;
        variables = new EnvVar[maxVars];
        varCount = 0;
    }
    
    ~EnvManager() {
        delete[] variables;
    }
    
    // Método para cargar variables desde un archivo
    bool loadFromFile(const char* filename) {
        if (!SPIFFS.begin(true)) {
            Serial.println("Error al montar SPIFFS");
            return false;
        }
        
        if (!SPIFFS.exists(filename)) {
            Serial.println("El archivo .env no existe");
            return false;
        }
        
        File file = SPIFFS.open(filename, "r");
        if (!file) {
            Serial.println("Error al abrir el archivo .env");
            return false;
        }
        
        Serial.println("Cargando variables de entorno desde " + String(filename));
        
        while (file.available()) {
            String line = file.readStringUntil('\n');
            line.trim();
            
            // Ignorar líneas vacías o comentarios
            if (line.length() == 0 || line.startsWith("#")) {
                continue;
            }
            
            String key = parseEnvLine(line);
            if (key != "") {
                Serial.println("Variable cargada: " + key);
            }
        }
        
        file.close();
        return true;
    }
    
    // Obtener el valor de una variable
    String get(const String& key, const String& defaultValue = "") {
        for (int i = 0; i < varCount; i++) {
            if (variables[i].key == key) {
                return variables[i].value;
            }
        }
        return defaultValue;
    }
    
    // Método para imprimir todas las variables (para depuración)
    void printAll() {
        Serial.println("\n----- Variables de entorno cargadas -----");
        for (int i = 0; i < varCount; i++) {
            Serial.print(variables[i].key);
            Serial.print(" = ");
            Serial.println(variables[i].value);
        }
        Serial.println("----------------------------------------\n");
    }
};