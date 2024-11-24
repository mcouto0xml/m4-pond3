// Inicia as bibliotecas necessárias
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Variáveis de controle
const int ledGreen = 18;
const int ledYellow = 17;
const int ledRed = 16;

// Variáveis de conexão (Wifi e Broker)
const char* ssid = "iPhone";
const char* password = "murilo123";

const int mqtt_port = 1883;
const char* mqtt_server = "broker.hivemq.com";



// Realiza a conexão com o Wifi 
void setupWifi() {
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Realiza a conexão com o Broker e define o callback
void setupBroker() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

}

// Cria função de reconexão com o Broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão com o Broker...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado ao Broker!");
      client.subscribe("esp/state");
      client.subscribe("esp/color");
    } else {
      Serial.print("Falha ao conectar. Return Code: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// Cria variáveis de controle
String color;
String state;

// Cria função callback 
void callback(char* topic, byte* message, unsigned int length) {
  // Recebe mensagem dos tópicos inscritos
  Serial.print("Mensagem vinda do tópico: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  String messageReceived;

  // Constrói a mensagem recebida
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageReceived += (char)message[i];
  }
  Serial.println(messageReceived);

  // Atualiza as variáveis globais dependendo dos tópicos
  if (strcmp(topic, "esp/state") == 0) { 
    if (messageReceived == "noite") {
      state = "noite";
    } else if (messageReceived == "dia") {
      state = "dia";
    }
  } else if (strcmp(topic, "esp/color") == 0) { 
    color = messageReceived; 
  }
}
// Cria a classe Night 
class Night {
  public:
    Night();
    // Declara o método a ser construido
    void piscarAmarelo();
  private:
    unsigned long lastBlinkTime = 0;
    const unsigned long blinkInterval = 2000;
    bool ledOn = false;
};
// Construtor da classe Night 
Night::Night() {}

// Cria o método piscarAmarelo da classe Night
void Night::piscarAmarelo() {
  // Pisca o led Amarelo de 2 em 2 segundos
  unsigned long currentTime = millis();
  if (currentTime - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentTime;
    ledOn = !ledOn;
    digitalWrite(ledYellow, ledOn ? HIGH : LOW);
  }
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);
}

// Cria a classe Day
class Day {
    public:
        Day();
        // Declara o método a ser construido
        void semaforoFunciona();
};
// Construtor da classe Day
Day::Day() {}

// Cria o método semaforoFunciona da classe Day
void Day::semaforoFunciona() {

  // Liga os leds de acordo com a regra de negócio do semáforo
    if (color == "red") {
        digitalWrite(ledGreen, HIGH);
        digitalWrite(ledYellow, LOW);
        digitalWrite(ledRed, LOW);
    }
    if (color == "yellow") {
        digitalWrite(ledGreen, LOW);
        digitalWrite(ledYellow, HIGH);
        digitalWrite(ledRed, LOW);
    }
    if (color == "green") {
        digitalWrite(ledGreen, LOW);
        digitalWrite(ledYellow, LOW);
        digitalWrite(ledRed, HIGH);
    }
}

// Cria os objetos das classes
Night night;
Day day;

// Função de configuração do ESP32
void setup() {
  Serial.begin(115200);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  setupWifi();
  setupBroker();
  
  delay(1000);
}

void loop() {

    // Verifica se o cliente (ESP32) está conectado com o Broker
    if (!client.connected()) {
    reconnect();
    }
    client.loop();

    // 
    if (state == "noite") {
      night.piscarAmarelo();
    } else {
      day.semaforoFunciona();
    }
}





