#include <dht11.h>
#include "BluetoothSerial.h"

dht11 DHT11;

const char* ssid = "......";
const char* password = ".....";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
boolean confirmRequestPending = true;

void BTConfirmRequestCallback(uint32_t numVal) {
  confirmRequestPending = true;
  Serial.println(numVal);
}

void BTAuthCompleteCallback(boolean success) {
  confirmRequestPending = false;
  if (success) {
    Serial.println(F("Pairing success!!"));
  } else {
    Serial.println(F("Pairing failed, rejected by user!!"));
  }
}

void bluetoothSetup() {
  const String device_name = "ESP32-BT-Slave";
  SerialBT.enableSSP();
  SerialBT.onConfirmRequest(BTConfirmRequestCallback);
  SerialBT.onAuthComplete(BTAuthCompleteCallback);
  SerialBT.begin(device_name);
  Serial.println(F("The device started, now you can pair it with bluetooth!"));
}

void readDHT11TemperatureAndHumidity() {
  const int DHT11PIN = 25;
  int sensorStatus = DHT11.read(DHT11PIN);
  switch (sensorStatus) {
    case DHTLIB_OK:
      Serial.println(F("OK"));
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.println(F("Checksum error"));
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.println(F("Timeout"));
      break;
    default:
      Serial.println(F("Unexpected error"));
      break;
  }
  Serial.print(F("H (%): "));
  Serial.print((float)DHT11.humidity, 2);
  Serial.print(F("  "));
  Serial.print(F("T (C): "));
  Serial.println((float)DHT11.temperature, 2);
}

int readMicrophone() {
  int audioSignal = analogRead(A6);
  if (audioSignal > 1950) {
    Serial.println(audioSignal);
  }
  return audioSignal;
}

void readDHT11Task(void* pvParameters) {
  while (1) {
    readDHT11TemperatureAndHumidity();
    delay(1000);
  }
}


void setup() {
  Serial.begin(115200);

  bluetoothSetup();

  xTaskCreatePinnedToCore(
    readDHT11Task,    // Function to implement the task
    "readDHT11Task",  // Name of the task
    2000,             // Stack size in words
    NULL,             // Task input parameter
    0,                // Priority of the task
    NULL,             // Task handle.
    0                 // Core where the task should run
  );
}



void loop() {
	// runs on Core 1 by default
  if (confirmRequestPending) {
    SerialBT.confirmReply(true);
  } else {
    int audioSignal = readMicrophone();
    SerialBT.write(audioSignal);
  }
    delay(10);
}
