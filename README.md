# Monitor de Ar Condicionado (ESP-01S + DHT22 → MQTT)

Firmware para **ESP8266 (ESP-01S)** que lê temperatura e umidade de um
sensor **DHT22** e publica os valores em um broker **MQTT** a cada
30 segundos.

## Visão geral

O código em [`src/main.cpp`](src/main.cpp):

1. Conecta o ESP-01S ao WiFi.
2. Conecta ao broker MQTT (`test.mosquitto.org:1883`).
3. Lê o DHT22 (temp/umidade) e publica em tópicos separados a cada 30s.
4. Reinicia o dispositivo em caso de falha de conexão ou se ficar mais de
   2 minutos sem conseguir publicar (watchdog).

## Componentes e pinagem

| Componente | Pino |
|---|---|
| ESP-01S | — |
| DHT22 | GPIO2 (DHTPIN) |

> **Atenção:** o GPIO2 do ESP-01S normalmente tem um LED onboard. Em
> algumas placas pode ser necessário considerar o resistor de pull-up.
> Verifique a alimentação do DHT22 (3.3V).

## Tópicos MQTT

| Tópico | Conteúdo |
|---|---|
| `CPD/BR/arcondicionado/temperatura` | Temperatura (°C), 2 casas decimais |
| `CPD/BR/arcondicionado/umidade` | Umidade relativa (%), 2 casas decimais |

Client ID: `ESP01S_DHT22_<chip_id>` (único por dispositivo).

## Como funciona

- **Leitura:** cada iteração do `loop()` lê o sensor DHT22.
- **Publicação:** valores válidos são publicados imediatamente via MQTT.
- **Intervalo:** 30 segundos entre iterações (`delay(30000)`).
- **Timeouts:**
  - `WIFI_TIMEOUT = 30s` — estourou, reinicia (`timeout WiFi`).
  - `MQTT_RETRY_TIME = 60s` — estourou, reinicia (`timeout MQTT`).
  - `PUBLISH_TIMEOUT = 2min` — sem publicação no período, reinicia
    (`sem publicacao por 2min`).

## Build e upload

Requisitos: [PlatformIO Core](https://platformio.org/).

```bash
# Build
pio run

# Upload para o ESP-01S
pio run -t upload

# Monitor serial (115200)
pio device monitor
```

## Configuração do projeto

`platformio.ini`:

- Placa: `esp01_1m` (ESP8266, 1MB flash)
- Framework: Arduino
- Libs: `DHT sensor library`, `PubSubClient`

## Observações

- **Credenciais WiFi em texto plano:** `ssid`/`password` estão hardcoded em
  `src/main.cpp`. Para produção, considere usar
  [ESP8266WiFiManager](https://github.com/tzapu/WiFiManager) ou
  credenciais em LittleFS.
- **Broker público:** `test.mosquitto.org` é um broker de testes; os dados
  ficam acessíveis publicamente. Para uso real, configure um broker próprio.
