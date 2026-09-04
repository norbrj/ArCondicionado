# language: pt
Funcionalidade: Leitura DHT22 → MQTT (ESP-01S)

Como responsável pela infra de climatização
Eu quero que o sensor publique temperatura/umidade no broker periodicamente
Para monitorar o ambiente do ar-condicionado

Cenário: Publicação periódica
  Dado o ESP-01S conectado ao WiFi e ao broker MQTT
  Quando o loop lê o DHT22
  Então publica temperatura em CPD/BR/arcondicionado/temperatura e umidade em
  CPD/BR/arcondicionado/umidade a cada 30 segundos
  E os valores têm 2 casas decimais

Cenário: Watchdog de conexão/publicação
  Dado o dispositivo sem conseguir conectar ao WiFi em 30s
  Então reinicia
  Dado o dispositivo sem conseguir se reconectar ao MQTT em 60s
  Então reinicia
  Dado o dispositivo sem publicar por 2 minutos
  Então reinicia (watchdog de publicação)

Cenário: Credenciais fora do código
  Dado o projeto clonado
  Então as credenciais de WiFi/broker estão em src/config.h (gitignored)
  E o repositório só expõe o template src/config.example.h sem segredos
