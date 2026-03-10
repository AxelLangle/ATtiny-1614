#include <SoftwareSerial.h>
#include <ModbusMaster.h>

// 1. Configuramos los pines para el puerto serie virtual
#define RX_PIN 2 // Conectar al pin TXD / RO del convertidor RS485
#define TX_PIN 3 // Conectar al pin RXD / DI del convertidor RS485

// Pin de control para el módulo RS485 (DE y RE conectados juntos)
// Nota: Si tu módulo verde no tiene pines DE/RE, simplemente ignora esta parte.
#define MAX485_RE_NEG  4 

SoftwareSerial modbusSerial(RX_PIN, TX_PIN);
ModbusMaster node;

// 2. Funciones de control de flujo (Walkie-Talkie)
// Le dicen al chip MAX485 cuándo debe transmitir y cuándo escuchar
void preTransmission() {
  digitalWrite(MAX485_RE_NEG, HIGH); // Modo Transmisión (Hablar)
}

void postTransmission() {
  digitalWrite(MAX485_RE_NEG, LOW);  // Modo Recepción (Escuchar)
}

void setup() {
  // Inicializamos el pin de control y lo ponemos en escucha por defecto    
  pinMode(MAX485_RE_NEG, OUTPUT);
  digitalWrite(MAX485_RE_NEG, LOW); 

  // Iniciamos la comunicación con la PC a 9600 baudios
  Serial.begin(9600);       
  
  // Iniciamos la comunicación con el sensor XY-MD03 a 9600 baudios
  modbusSerial.begin(9600); 

  // Configuramos el "nodo" (el esclavo). El XY-MD03 suele venir con el ID 1 de fábrica.
  node.begin(1, modbusSerial);
  
  // Asignamos las funciones de control de flujo a la librería
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  
  Serial.println("Iniciando lectura del sensor Modbus XY-MD03...");
}

void loop() {
  uint8_t result;
  
// 3. La Petición Modbus
  // La instrucción le pide al sensor que nos devuelva 2 registros a partir de la dirección 1.
  result = node.readHoldingRegisters(0, 2); 

  // 4. Verificamos si la respuesta fue exitosa
  if (result == node.ku8MBSuccess) {
    
    // getResponseBuffer(0) tiene el primer dato que pedimos (Registro 1 = humedad)
    float humedad = node.getResponseBuffer(0) / 10.0f;
    
    // getResponseBuffer(1) tiene el segundo dato que pedimos (Registro 2 = temperatura)
    float temperatura = node.getResponseBuffer(1) / 10.0f;

    // Imprimimos los resultados en el Monitor Serie
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.print(" °C  |  Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");
  } else {
    // Si hay interferencia o un cable suelto, imprimirá un código de error en formato Hexadecimal
    Serial.print("Error en la lectura Modbus. Código: ");
    Serial.println(result, HEX); 
  }

  // Esperamos 2 segundos completos antes de volver a preguntarle al sensor
  delay(2000); 
}