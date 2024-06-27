byte sda = D4;
byte rst = D3;
#include<KRrfid.h>
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include<SoftwareSerial.h>
#include <ArduinoJson.h>

SoftwareSerial node(D1,D0); //RX, TX

String ruangan = "17";
String data = "";

int button = 15;
#define triger D0 
#define sensor A0
const char* ssid = "nig";
const char* password = "qwerty123";

WiFiServer server(80);


//URL ENCODE
String urlencode(String str)
{
  String encodedString="";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i =0; i < str.length(); i++){
    c=str.charAt(i);
    if (c == ' '){
      encodedString+= '+';
    } else if (isalnum(c)){
      encodedString+=c;
    } else{
      code1=(c & 0xf)+'0';
      if ((c & 0xf) >9){
          code1=(c & 0xf) - 10 + 'A';
      }
      c=(c>>4)&0xf;
      code0=c+'0';
      if (c > 9){
          code0=c - 10 + 'A';
      }
      code2='\0';
      encodedString+='%';
      encodedString+=code0;
      encodedString+=code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;
    
}
//BATAS URL ENCODE2862240640


void setup() {
  Serial.begin(9600);
  node.begin(9600);
  rfidBegin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  pinMode(button, INPUT_PULLUP);
  pinMode(triger, OUTPUT);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("Connecting to WiFi..");
    delay(100);
  }
  Serial.println("TERHUBUNG");
  Serial.println(WiFi.localIP());
  tutup();
}

void loop() {
  int data_sensor = map(analogRead(sensor), 0, 1023, 0, 100);
  Serial.println(data_sensor);
  
  if(data_sensor > 80)
  {
   getTAG();
   if (TAG != "") {
      Serial.print("TAG :");
      Serial.println(TAG);
      
      WiFiClient client = server.available();
      HTTPClient http;
      String url_api = "http://192.168.55.173/rfid-admin/index.php/Api/cek?kartu={val1}&ruangan={val2}";
      url_api.replace("{val1}", urlencode(TAG));
      url_api.replace("{val2}", ruangan);
      char url_api_char[100];
      url_api.toCharArray(url_api_char, 100);
      http.begin(url_api_char);
      int respon = http.GET();
      Serial.println(respon);
      if(respon > 0)
      {
        if(respon == HTTP_CODE_OK)
        {
          data = http.getString();
        }
        else
        {
          data = "Gagal";
        }
        delay(1000);
      }
      Serial.println(data);
      http.end();
      
      if(data == "Sukses")
      {
        Serial.println("PINTU TERBUKA");
        TAG = "";
        buka();
        delay(5000);
        int timer = 0;
        data_sensor = map(analogRead(sensor), 0, 1023, 0, 100);
        while (data_sensor < 20 and timer < 10000)
        {
          timer = timer+1000;
          Serial.println("Timer = "+String(timer));
          data_sensor = map(analogRead(sensor), 0, 1023, 0, 100);
          delay(1000);
        }
        tutup();
      }
    }
    
    if(digitalRead(button) == HIGH)
    {
        Serial.println("PINTU TERBUKA");
        buka();
        delay(5000);
        int timer = 0;
        data_sensor = map(analogRead(sensor), 0, 1023, 0, 100);
        while (data_sensor < 20 and timer < 10000)
        {
          timer = timer+1000;
          Serial.println("Timer = "+String(timer));
          data_sensor = map(analogRead(sensor), 0, 1023, 0, 100);
          delay(1000);
        }
        tutup();
    }
  }
  TAG="";
  delay(100);
}

void buka(){
   digitalWrite(triger,HIGH);
}

void tutup(){
   digitalWrite(triger,LOW);
}
