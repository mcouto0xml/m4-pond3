# Importa as bibliotecas necessárias
from flask import Flask, request, jsonify
from flask_mqtt import Mqtt
import logging
logging.basicConfig(level=logging.DEBUG)

app = Flask(__name__)

# Configuração do Broker MQTT
app.config['MQTT_BROKER_URL'] = "broker.hivemq.com"
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = ''
app.config['MQTT_PASSWORD'] = ''
app.config['MQTT_KEEPALIVE'] = 60
app.config['MQTT_TLS_ENABLED'] = False

# Cria o banco de dados provisório
db = [
    {"id": 1, "state": "dia", "isensor": "sim"}
]

# Inicializa variáveis de Controle
payload = None
topic = None

# Especifica os tópicos a serem inscritos no MQTT
topic1 = "esp/state"
topic2 = "esp/color"

# Inicializa o objeto para uso dos métodos da biblioteca
mqtt_client = Mqtt(app)

# Função criada com intuito de armazenar o id do banco de dados
def getMaxId():
    lastId = max(item['id'] for item in db)
    return lastId

# Decorador acionado quando a conexão com o Broker é efetuada
@mqtt_client.on_connect()
# Callback para receber o pacote CONNAK enviado pelo Broker
def handle_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado com sucesso!")
        mqtt_client.subscribe(topic1)
        mqtt_client.subscribe(topic2)
        print(f"Inscrito nos tópicos {topic1}")
    else:
        print(f"Falha na conexão com o Broker, código de retorno: {rc}")

# Decorador acionado caso o receba uma mensagem
@mqtt_client.on_message()
# Callback para manusear a mensagem recebida
def handle_mqtt_message(client, userdata, message):
    global payload, topic
    payload = message.payload.decode()
    topic = message.topic
    print(f"Tópico: {topic} - Mensagem: {payload}")
    if db and topic == "flask/state":
        db[0]["state"] = payload

# Decorador para rota /
@app.route('/')
# Função de rota (endpoint)
def index():
    return "Servidor Flask com MQTT!"

# Decorador para rota /test e método GET 
@app.route('/teste', methods=['GET'])
# Função da rota (endpoint)
def teste():
    return "O teste funcionou!"

# Decorador para rota /noite-ou-dia e método GET 
@app.route('/noite-ou-dia', methods=['GET'])
# Função da rota (endpoint)
def noiteOuDia():
    state = db[0]["state"]
    print(state)
    return jsonify({f"O estado atual é: {state}"})

# Decorador para rota /noite-ou-dia e método PUT 
@app.route('/noite-ou-dia', methods=['PUT'])
# Função da rota (endpoint)
def change_state():
    state = request.json.get('state', 'Default')
    isensor = request.json.get('isensor', 'Default')

    if db:
        new_id = getMaxId()
    else:
        new_id = 1

    db.append({"id": new_id, "state": state, "isensor": isensor})

    print(db)

    # Publica as alterações nos tópicos MQTT
    mqtt_client.publish("flask/state", state)
    mqtt_client.publish("flask/isensor", isensor)

    return jsonify({'Mensagem': 'Estado Alterado com Sucesso!'})

# Inicia a aplicação com host e porta definidos
if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8000)
