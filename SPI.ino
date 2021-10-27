//bibliotecas
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

//utilizando o pino 27 do esp32 para ligar o sensor DHT22
#define DHTPIN 27
#define DHTTYPE DHT22

//nome da rede wifi e senha
const char* ssid     = "C3P0";
const char* password = "MayThe4BeWithU";

WiFiServer server(80);

IPAddress ip;
IPAddress ip1;
IPAddress ip2;

DHT dht(DHTPIN, DHTTYPE);

//armazena os valores lidos de fumaca, temperatura e umidade
String smoke;
String umid;
String temp;

//armazena os valores maximos de temperatura e fumaca e minimo de umidade
int comptemp = 60;
int compumid = 30;
int compsmoke = 3000;

//funcao que retorna o valor lido da temperatura
String rtemp() {
  float t = dht.readTemperature();
  float auxt;
  if (isnan(t)) {
    Serial.println("Falha ao ler a temperatura");
    return String(auxt);
  }
  else {
    auxt = t;
    return String(t);
  }
}

//funcao que retorna o valor lido de umidade
String rumid() {
  float h = dht.readHumidity();
  float auxh;
  if (isnan(h)) {
    Serial.println("Falha ao ler a umidade");
    return String(auxh);
  }
  else {
    auxh = h;
    return String(h);
  }
}

//setup
void setup()
{
  Serial.begin(115200);
  dht.begin();
  
  pinMode(2, OUTPUT); //usei esse pino para buzzer para testes, sera removido na versao final
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  ip2 = WiFi.subnetMask();
  ip1 = WiFi.gatewayIP();
  IPAddress iplocal(ip1[0], ip1[1], ip1[2], 91);
  ip = iplocal;
  Serial.println(ip);
  WiFi.config(ip, ip1, ip2);
  server.begin();
}

//funcao para ler os sensores
void sensores() {
  smoke = analogRead(35);
  umid = rumid();
  temp = rtemp();
  
  // a cada leitura printa o que foi lido no serial e os valores setados para min e max 
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("    Umid: ");
  Serial.print(umid);
  Serial.print("    Smoke: ");
  Serial.print(smoke);
  
  Serial.print("    comptemp: ");
  Serial.print(comptemp);
  Serial.print("    compumid: ");
  Serial.print(compumid);
  Serial.print("    compsmoke: ");
  Serial.println(compsmoke);
}

int value = 0;

void loop() {
  sensores();
  WiFiClient client = server.available();
  
  if (client) {
    sensores();
    Serial.println("New Client.");
    String currentLine = "";
    
    while (client.connected()) {

      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("<!DOCTYPE html></html>");
            client.println("<style>html { font-family: Arial; font-size: 3.0rem; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<body bgcolor=#9C0705>");
            client.println("<title><h1>SMI</h1>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
        
        //quando acessado printa a respectiva informacao
        if (currentLine.endsWith("GET /A")) {
          client.println("<h1>" + umid + " </h1>");
          client.println("<style>html { font-family: Arial; font-size: 3.0rem; display: inline-block; margin: 0px auto; text-align: center;}");
        }
        if (currentLine.endsWith("GET /B")) {
          client.println("<h1>" + temp + "</h1>");
          client.println("<style>html { font-family: Arial; font-size: 3.0rem; display: inline-block; margin: 0px auto; text-align: center;}");
        }
        if (currentLine.endsWith("GET /C")) {
          client.println("<h1>" + smoke + "</h1>");
          client.println("<style>html { font-family: Arial; font-size: 3.0rem; display: inline-block; margin: 0px auto; text-align: center;}");
        }
        
        //deteccao dos valores enviados pelo app para a configuracao dos valores de min e max       
        if (currentLine.indexOf("/X") >= 0) {
          String valor = currentLine.substring(currentLine.indexOf("/X") + 3);
          comptemp = valor.toInt();
        }
        if (currentLine.indexOf("/Y") >= 0) {
          String valor = currentLine.substring(currentLine.indexOf("/Y") + 3);
          compumid = valor.toInt();
        }
        if (currentLine.indexOf("/Z") >= 0) {
          String valor = currentLine.substring(currentLine.indexOf("/Z") + 3);
          compsmoke = valor.toInt();
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");

  }

  //testes dos limites de min e max usando o buzzer e o led no pino 2
  if ((temp.toInt() >= comptemp) || (umid.toInt() <= compumid) || (smoke.toInt() >= compsmoke)) {
    digitalWrite(2, HIGH);
  }
  else {
    digitalWrite(2, LOW);
  }

}
