// Importa as bibliotecas necessárias
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

// Inicializando os objetos das classes das bibliotecas
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Constantes dos pinos do ESP32
const int ledGreen = 16;
const int ledYellow = 17;
const int ledRed = 18;
const int ldr = 32;

// Constante das informações para conexão Wi-Fi
const char* ssid = "iPhone";
const char* password = "murilo123";

// Constantes para conexão com o Broker MQTT
const int mqtt_port = 1883;
const char* mqtt_server = "broker.hivemq.com";

// Variáveis de controle de tempo
unsigned long lastMessageTime = 0; // Controle de tempo para mensagens MQTT
unsigned long lastLdrReadTime = 0; // Controle de tempo para leitura do LDR
const unsigned long interval = 2000; // Intervalo de 2 segundos

int luminosidade = 0; // Variável para armazenar o valor do LDR

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
      client.subscribe("flask/state");
      client.subscribe("flask/isensor");
    } else {
      Serial.print("Falha ao conectar. Código: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// Variáveis de controle de estado
String state;
String isensor = "sim";

// Cria função callback 
void callback(char* topic, byte* message, unsigned int length) {
  // Recebe mensagem dos tópicos inscritos
  Serial.print("Mensagem vinda do tópico: ");
  Serial.print(topic);
  Serial.print(". Payload: ");
  String messageReceived;

  // Constrói a mensagem recebida
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageReceived += (char)message[i];
  }
  Serial.println(messageReceived);

  // Atualiza as variáveis globais dependendo dos tópicos
  if (strcmp(topic, "flask/state") == 0) {
    state = messageReceived;
  } else if (strcmp(topic, "flask/isensor") == 0) {
    isensor = messageReceived;
  }
}

// Função resposável por ler dados de luminosidade captados pelo sensor LDR
int getLuminosidade() {
    int valorLDR = analogRead(ldr);

    int luminosidade = valorLDR;

    return luminosidade;
}

// Função responsável por enviar 2 mensagens ao Broker MQTT e esperar um intervalo de tempo
void sendMessage(String topic1, String msg1, String topic2, String msg2) {
  unsigned long currentTime = millis();
  
  // Verifica se já passou o intervalo definido
  if (currentTime - lastMessageTime >= interval) {
    // Envia a primeira mensagem
    if (client.publish(topic1.c_str(), msg1.c_str())) {
      Serial.print("Mensagem enviada: ");
      Serial.println(msg1);
    } else {
      Serial.println("Erro ao enviar a mensagem (1)!");
    }

    // Envia a segunda mensagem
    if (client.publish(topic2.c_str(), msg2.c_str())) {
      Serial.print("Mensagem enviada: ");
      Serial.println(msg2);
    } else {
      Serial.println("Erro ao enviar a mensagem (2)!");
    }

    // Atualiza o tempo do último envio após enviar ambas
    lastMessageTime = currentTime;
  }
}


// Cria a classe Night
class Night {
  public:      
    Night();
    // Define o método a ser construido
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
    sendMessage("esp/state", "noite", "esp/state", "noite");
  }
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);
}

// Cria a classe Day
class Day {
  public:
    Day();
    // Define o método a ser construído
    void semaforoFunciona();
  private:
    unsigned long lastChangeTime = 0;
    const unsigned long redInterval = 6000;
    const unsigned long greenInterval = 5000;
    const unsigned long yellowInterval = 3000;
    char currentLed = 'r';
};

// Construtor da classe Day
Day::Day() {}

// Cria o método semaforoFunciona da classe Day
void Day::semaforoFunciona() {
  // Realiza o funcionamento do farol de acordo com a regra de negócios e envia o estado e cor atual
  unsigned long currentTime = millis();
  if (currentLed == 'r' && currentTime - lastChangeTime >= redInterval) {
    lastChangeTime = currentTime;
    currentLed = 'g';
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);

    sendMessage("esp/state", "dia", "esp/color", "green");
    
  } else if (currentLed == 'g' && currentTime - lastChangeTime >= greenInterval) {
    lastChangeTime = currentTime;
    currentLed = 'y';
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledYellow, HIGH);
    sendMessage("esp/state", "dia", "esp/color", "yellow");

  } else if (currentLed == 'y' && currentTime - lastChangeTime >= yellowInterval) {
    lastChangeTime = currentTime;
    currentLed = 'r';
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledRed, HIGH);
    sendMessage("esp/state", "dia", "esp/color", "red");
    
  }
}

// Inicializa os objetos das classes
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
  // Verifica a conexão com o MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Variáveis de controle de tempo
  static unsigned long lastSensorRead = 0;
  unsigned long currentMillis = millis();

  // Lê o sensor de luminosidade a cada 2 segundos
  if (currentMillis - lastSensorRead >= 2000) {
    lastSensorRead = currentMillis;
    
    int luminosidade = getLuminosidade();
    Serial.print("Luminosidade: ");
    Serial.println(luminosidade);

    // Altera o estado do semáforo baseado nas variáveis de controle
    if (isensor == "sim") {
      if (luminosidade <= 300) {
        night.piscarAmarelo();
      } else {
        day.semaforoFunciona();
      }
    } else {
      if (state == "noite") {
        night.piscarAmarelo();
      } else {
        day.semaforoFunciona();
      }
    }
  }
}


