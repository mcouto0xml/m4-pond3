# m4-pond3
Este projeto trata sobre a criação de uma miniatura de 2 semáforos inteligentes usando ESP32 e uma maquete de madeira.

## Especificações
Segue abaixo as especificações para atividade

| Componente               | Quantidade | Especificação                    |
|--------------------------|------------|----------------------------------|
| LED Verde                | 2          | Indicador de sinal               |
| LED Amarelo              | 2          | Indicador de sinal               |
| LED Vermelho             | 2          | Indicador de sinal               |
| Sensor LDR               | 1          | Leitura de Lumens                |
| Jumper Macho-Macho       | 13         | Conexão entre protoboard         |
| Jumper Macho-Fêma        | 8          | Conexão entre LED e protoboard   |
| Resistor 1 kOhm          | 1          | Limitar corrente para sensor LDR |
| Resistor 100 Ohm         | 6          | Limitar corrente para LED's      |
| Protoboard               | 1          | Montagem do circuito             |
| ESP32                    | 2          | Microcontrolador principal       |
| Cabo Micro USB           | 2          | Conexão e alimentação do ESP32   |

## Montagem do Circuito

A montagem do circuito foi realizada em seis etapas detalhadas a seguir:

### Conexão básica do ESP32:

- Conecte o ESP32 à protoboard.
- Conecte o pino 5V à linha de conexão positiva (+) e o GND à linha negativa (-) da protoboard.

### Conexão dos LEDs:

- Insira os terminais de entrada de energia dos LEDs em três linhas de conexão distintas na protoboard.
- Conecte os terminais de saída de energia dos LEDs à linha negativa da protoboard (GND) utilizando jumpers.

### Inserção de resistores e conexões ao ESP32:

- Coloque três resistores de 100Ω na mesma linha de conexão dos terminais de entrada dos LEDs.
- Adicione jumpers na outra ponta de cada resistor e conecte-os aos pinos 16, 17 e 18 do ESP32.

### Conexão do sensor LDR:

- Posicione os terminais do LDR em duas linhas de conexão distintas da protoboard.
- Em um dos terminais, conecte um resistor de 1kΩ a outra linha de conexão, e na ponta oposta do resistor, insira um jumper até a linha negativa (GND) da protoboard.

### Conexão do LDR ao ESP32:

- Na mesma linha de conexão onde está o terminal compartilhado pelo LDR e o resistor, conecte um jumper ao pino 32 do ESP32.

### Finalização do circuito:

- No terminal do LDR que ainda não possui conexão, adicione um jumper ligado à linha de conexão positiva (5V).

### Montagem do Semáforo Pai
![Montagem do Semáforo Pai](assets\fotos/montagemPai.jpg)

### Montagem do Semáforo Filho
![Montagem do Semáforo Filho](assets\fotos/montagemFilho.jpg)

## Descrição do Código

O código do projeto foi desenvolvido inteiramente utilizando Programação Orientada a Objetos (POO), resultando em uma estrutura modular e escalável. Essa abordagem facilita a inserção de novos semáforos (filhos) com poucas alterações.

O algoritmo possui duas principais classes que representam os estados do sistema:

- ```Day``` (Dia): Define o comportamento do semáforo durante o dia.
- ```Night``` (Noite): Define o comportamento do semáforo durante a noite.
Essas classes incluem métodos específicos para controlar as funcionalidades dos semáforos, garantindo a alternância de estados e cores conforme necessário.

## Estrutura do Semáforo Inteligente

O sistema é composto por três elementos principais:

### Semáforo Pai:

- Responsável por comandar o Semáforo Filho.
- Transmite o estado atual (dia ou noite) e a cor (verde, amarelo ou vermelho) para o Semáforo Filho por meio do protocolo MQTT, utilizando os tópicos ```esp/state``` e ```esp/color```.
- Recebe comandos da "Mesa de Jantar" (aplicação Flask) para:
  - Alterar o estado (dia ou noite).
  - Ativar ou desativar o uso do sensor de luz (LDR).

### Semáforo Filho:

- Apenas responde às instruções recebidas do Semáforo Pai.
- Altera seu estado e cor de acordo com as mensagens recebidas.
- Observação: Se o Semáforo Pai está fechado, o Semáforo Filho estará aberto e vice-versa.

### Mesa de Jantar:

- Uma aplicação Flask que:
  - Escuta mensagens publicadas pelo Semáforo Pai e atualiza um banco de dados provisório (dicionário em Python).
  - Fornece uma API com quatro endpoints para:
    - Status report da inicialização da aplicação
    - Testar a conexão.
    - Visualizar o estado atual dos semáforos.
    - Alterar o estado do semáforo e o uso do sensor LDR.
- Observação: Para interagir com os endpoints, recomenda-se o uso do Postman. 

## Variáveis Controladas em Tempo Real pelo Semáforo Pai

No Semáforo Pai, três variáveis principais são constantemente atualizadas:

- Color: Cor atual do Semáforo Pai, usada para atualizar o Semáforo Filho.
- State: Estado atual (dia ou noite) do Semáforo Pai, também enviado ao Semáforo Filho.
- iSensor: Define se o Semáforo Pai deve operar com base no sensor LDR, de acordo com as requisições recebidas pela Mesa de Jantar.

Com essa estrutura, o semáforo inteligente oferece flexibilidade, eficiência e escalabilidade para diferentes cenários.

## Vídeos
Nessa secção será tratado os vídeos gravados sobre o funcionamento do projeto.

O vídeo abaixo trata-se da lógica do semáforo em si, e do sensor LDR, para alteração de estado (noite ou dia).

<video controls width="600">
  <source src="assets\videos\funcionamentoLogicaAndLDR.mp4" type="video/mp4">
  Seu navegador não suporta o elemento de vídeo.
</video>


O vídeo abaixo trata-se da explicação da lógica entre Pai e Filho, e sua comunicação MQTT, retratando das dificuldades e complicações do funcionamento do projeto.

<video controls width="600">
  <source src="assets\videos\funcionamentoMQTT.mp4" type="video/mp4">
  Seu navegador não suporta o elemento de vídeo.
</video>

## Observações

Devido a complicações na conexão com o Broker MQTT, os dois semáforos não se comunicam instantaneamente, embora todos os comportamentos definidos no código funcionem corretamente. Além disso, para tentar otimizar a eficiência da conexão, foi testado o uso de um Broker hospedado na nuvem, mas os resultados não apresentaram melhorias significativas. 

Por outro lado, o uso da aplicação Flask torna-se inviável ao utilizar o Broker na nuvem, pois a aplicação não possui suporte a SSL, o que impede a conexão. Apesar dessas limitações, foram disponibilizados dois códigos distintos: um para o semáforo Pai e outro para o semáforo Filho, que contemplam tanto o uso quanto a ausência dos serviços em nuvem nos sistemas embarcados do ESP32.