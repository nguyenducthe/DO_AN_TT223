#include <SPI.h>
#include <LoRa.h>
#include<DHT22.h>
int counter = 0;
#define data A1
DHT22 dht22(data);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Bat dau ket noi Lora : ");
  if (!LoRa.begin(433E6)) {
    Serial.println("Ket noi that bai !");
    while (1);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa bat dau gui  tap tin: ");
}

void loop() {
    float nhiet_do = dht22.getTemperature();
    float do_am =  dht22.getHumidity();
    
    Serial.print("Sending packet: ");
    Serial.println(counter);

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  
  LoRa.print(nhiet_do);
  LoRa.print('a');
  Serial.print(nhiet_do);Serial.print("\n");
  LoRa.print(do_am);
  LoRa.print('b');
  Serial.print(do_am);Serial.print("\n");
  LoRa.endPacket();

  counter++;

  delay(10000);
  
  
}