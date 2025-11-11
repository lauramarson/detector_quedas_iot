# Sistema de Detecção de Quedas para Idosos com IoT (ESP32 + MQTT)

Este repositório contém todos os arquivos e a documentação do projeto de um dispositivo vestível (wearable) para detecção de quedas de idosos, utilizando um ESP32, um sensor MPU6050 e comunicação via protocolo MQTT para o envio de alertas.

Todo o projeto foi desenvolvido e validado utilizando o Simulador Wokwi, permitindo a prototipagem 100% digital.

## i) Descrição do Projeto e Funcionamento
O objetivo deste projeto é criar um sistema de baixo custo que monitora os movimentos de um usuário e, ao detectar um evento de queda, envia um alerta instantâneo pela internet para um familiar ou cuidador.

### Como Funciona
A lógica do sistema segue um fluxo contínuo:

1. Monitoramento: O sensor MPU6050 (acelerômetro + giroscópio) lê continuamente os dados de movimento do usuário.

2. Detecção: O ESP32 analisa esses dados em tempo real. Um algoritmo de duplo limiar procura por um padrão específico: um breve momento de aceleração zero (queda livre) seguido de um pico de alto impacto.

3. Alerta Local: Ao confirmar uma queda, o ESP32 aciona imediatamente um Buzzer local.

4. Alerta Remoto: Simultaneamente, o ESP32 se conecta à rede Wi-Fi e publica uma mensagem de alerta em um tópico MQTT.

5. Notificação: Um cliente MQTT (como um app no celular de um familiar) inscrito nesse tópico recebe a mensagem instantaneamente.

![Fluxograma](https://github.com/user-attachments/assets/c459cbfe-2f1a-46c0-9b74-1fbfc0ef70e3)

## ii) Software e Documentação de Código
O firmware do dispositivo foi desenvolvido em C/C++ no ambiente Arduino e é totalmente compatível com o simulador Wokwi.

Localização dos arquivos: /wokwi-simulation/

### Arquivos Principais
- sketch.ino: O código-fonte principal que contém toda a lógica de inicialização, leitura do sensor, detecção de queda e comunicação.

- PubSubClient.h / .cpp: Biblioteca para a comunicação MQTT.

- diagram.json: Arquivo de configuração do circuito para o simulador Wokwi.

### Bibliotecas Utilizadas
- Adafruit_MPU6050: Para comunicação com o sensor MPU6050.

- Adafruit_Unified_Sensor: Dependência do sensor.

- Wire.h: Para comunicação I2C.

- WiFi.h: Para a conectividade Wi-Fi do ESP32.

- PubSubClient.h: Para a comunicação MQTT.

## iii) Hardware Utilizado
O circuito foi projetado para ser simples e de baixo custo, utilizando componentes comuns.

Localização dos arquivos: /hardware/

### Lista de Componentes
- Plataforma de Desenvolvimento: Placa ESP32 DevKitC V4 (com módulo ESP32-WROOM-32).

- Sensor (Entrada): Módulo MPU6050 (Acelerômetro e Giroscópio de 6 eixos).

- Atuador (Saída): Buzzer Piezoelétrico Passivo.

- Outros: Protoboard e Jumper Wires.

### Diagrama de Montagem
O diagrama abaixo, gerado no Wokwi, detalha todas as conexões do circuito.

<img width="414" height="712" alt="Diagrama de Montagem" src="https://github.com/user-attachments/assets/4cf12882-7ef6-42d8-97e0-8600ae75ba97" />

## iv) Interfaces e Protocolos de Comunicação
O projeto utiliza um conjunto de interfaces de hardware e protocolos de rede para funcionar.

### Interfaces de Hardware
- I2C (Inter-Integrated Circuit): Utilizado para a comunicação de dados entre o ESP32 (mestre) e o sensor MPU6050 (escravo).
  * ESP32 GPIO 21 (SDA) → MPU6050 SDA
  * ESP32 GPIO 22 (SCL) → MPU6050 SCL
 
- GPIO (General Purpose Input/Output): Uma saída digital padrão do ESP32 é usada para enviar um sinal PWM ao buzzer.
  * ESP32 GPIO 23 → Buzzer (positivo)

### Módulos e Protocolos de Comunicação
- Módulo de Comunicação: O próprio chip ESP32, que possui um módulo de comunicação Wi-Fi 802.11 b/g/n integrado.
- TCP/IP: O ESP32 utiliza a pilha TCP/IP padrão para se conectar à rede local via Wi-Fi e acessar a internet.
- MQTT (Message Queuing Telemetry Transport): Este é o protocolo principal para a comunicação de alertas (Requisito v).
  * Broker: broker.hivemq.com (um broker público para testes)
  * Tópico de Alerta: mackenzie/fall_detector/alert
  * Funcionamento: O ESP32 atua como um "Publisher", enviando a mensagem de alerta para o broker. O celular do familiar atua como um "Subscriber", recebendo a mensagem desse mesmo broker.

## Como Reproduzir o Projeto (Simulação)
Você pode testar este projeto em 10 minutos, sem precisar de nenhum hardware físico, usando o Wokwi.

1. Abra o Wokwi: Vá para wokwi.com e inicie um novo projeto de ESP32.
2. Carregue os Arquivos:

  * Copie o conteúdo de /wokwi-simulation/sketch.ino para o arquivo sketch.ino no Wokwi.
  * Crie os arquivos PubSubClient.h e PubSubClient.cpp no Wokwi e copie o conteúdo dos arquivos correspondentes deste repositório.
  * Vá para a aba diagram.json no Wokwi e substitua seu conteúdo pelo conteúdo de /wokwi-simulation/diagram.json.

3. Execute a Simulação: Clique no botão verde "Start" (Play). O Monitor Serial mostrará os logs de conexão Wi-Fi e MQTT.

4. Teste o Alerta MQTT:
  * Abra um cliente MQTT em outra aba (ex:(http://www.hivemq.com/demos/websocket-client/)).
  * Conecte-se ao broker broker.hivemq.com (Porta 8884, com SSL ativado).
  * Inscreva-se no tópico mackenzie/fall_detector/alert.

5. Simule a Queda:
  * Volte ao Wokwi, clique no sensor MPU6050.
  * Faça um movimento rápido com o slider do eixo Z: de 1g para 0g (queda livre) e imediatamente para 2g (impacto).
  * Você ouvirá o buzzer na simulação e verá a mensagem de alerta aparecer no seu cliente MQTT.

## Resultados da Simulação
Os testes no Wokwi validaram o funcionamento completo do sistema:
