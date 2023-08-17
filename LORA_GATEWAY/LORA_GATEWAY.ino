#include <ETH.h>
#include<ESPAsyncWebServer.h>
#include<DHT22.h>
#include<WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_SDA 15
#define I2C_SCL 4
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT 
#define ETH_POWER_PIN   -1                  
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        1
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
/*
#define led_wifi      12
#define led_eth       13
*/


#define SS 5
#define rst 32
#define dio0 2

#define MOSI 13
#define MISO  12
#define SCK  16


AsyncWebServer server(80);

IPAddress local_ip(192, 168, 137, 49);
IPAddress gateway(192, 168, 137, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2 = (uint32_t)0x00000000;

const char* ssid = "THE"; 
const char* password = "12345678";

String data_; 
String nhiet_do;
String do_am;


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html lang = "vi">
<head>
  <meta charset = "UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ĐO NHIỆT ĐỘ , ĐỘ ẨM ESP32</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Nhiệt độ : </span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Độ ẩm : </span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";


String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return nhiet_do;
  }
  else if(var == "HUMIDITY"){
    return do_am;
  }
  
  return String();
}


static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started"); Serial.print("\n");
      //set eth hostname here
      ETH.setHostname("ADMIN");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");Serial.print("\n");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      WiFi.softAPdisconnect();
      Serial.print("WIFI DISCONNECT \n");
      Serial.print("CONNECT ETHERNET \n");
      //digitalWrite(led_eth,HIGH);
     // digitalWrite(led_wifi,LOW);
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
        
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected \n");
      eth_connected = false;
      Serial.print("Ket noi thanh cong ETH : \n");
      Serial.print("Ket noi WIFI : \n");
      //digitalWrite(led_wifi,HIGH);
     // digitalWrite(led_eth,LOW);
      WiFi.softAP(ssid, password);
      WiFi.softAPConfig(local_ip, gateway, subnet);
      Serial.print("KET NOI WIFI THANH CONG \n");
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped \n");
      eth_connected = false;
      break;
    default:
      break;
  }
}
void ETH_init()
{
  Serial.print("Ket noi ETH : \n");
  WiFi.onEvent(WiFiEvent);
  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
  ETH.config(local_ip, gateway, subnet, dns1, dns2);
}
void OLED_init()
{
  Wire.begin(I2C_SDA, I2C_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.print("Hiển thị OLED \n");

}
void LORA_init()
{
  SPI.begin(SCK, MISO, MOSI, SS);
  while (!Serial);
  Serial.println("LoRa Receiver");
  LoRa.setPins(SS, rst, dio0);
  while (!LoRa.begin(433E6)) {
   Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("Ket noi thanh cong LORA \n");
  
}

void setup() {
  //pinMode(led_wifi,OUTPUT);
 // pinMode(led_eth,OUTPUT);
  Serial.begin(9600);
  pinMode(SS, OUTPUT);
  ETH_init();
  LORA_init();
  OLED_init();
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
  request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", nhiet_do.c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", do_am.c_str());
  });
  server.begin();
  
}
void hienthi_oled()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("nhiet do: ");
  display.setCursor(0,16);
  display.println("do am: ");
  display.setCursor(64,0);
  display.println(nhiet_do);
  display.setCursor(64,16);
  display.println(do_am); 
  display.display();
}
void loop()
{
  String data_ = "";
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Nhan goi tin: ");
    while (LoRa.available())
    {
      data_ += (char)LoRa.read();
    }
   if (data_ != "")
   {  
     nhiet_do = (data_.substring(0, data_.indexOf("a"))).toFloat();
     do_am = (data_.substring(data_.indexOf("a") + 1,data_.indexOf("b"))).toFloat();
    Serial.print(" RSSI : ");
    Serial.println(LoRa.packetRssi());
   }
    Serial.print("Nhiet do : "); Serial.print(nhiet_do); Serial.print("\t"); Serial.print("Do am: "); Serial.print(do_am); Serial.print("\n");
  }
  hienthi_oled();
  delay(100);
  }