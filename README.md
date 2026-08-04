# Monitor de Ar Condicionado (ESP-01S + DHT22 → MQTT)

Firmware para **ESP8266 (ESP-01S)** que lê temperatura e umidade de um
sensor **DHT22** e publica os valores em um broker **MQTT** a cada
30 segundos.

## Visão geral

O código em [`src/main.cpp`](src/main.cpp):

1. Conecta o ESP-01S ao WiFi.
2. Conecta ao broker MQTT (`test.mosquitto.org:1883`).
3. Lê o DHT22 (temperatura/umidade) e publica em tópicos separados a cada 30s.
4. Reinicia o dispositivo em caso de falha de conexão ou se ficar mais de
   2 minutos sem conseguir publicar (watchdog).

## Componentes e pinagem

| Componente | Pino |
|---|---|
| ESP-01S | — |
| DHT22 (DATA) | GPIO2 (`DHTPIN`) |

> **Atenção:** o GPIO2 do ESP-01S normalmente tem um LED onboard. Em
> algumas placas pode ser necessário considerar o resistor de pull-up.
> Verifique a alimentação do DHT22 (3.3V).

## Tópicos MQTT

| Tópico | Conteúdo |
|---|---|
| `CPD/BR/arcondicionado/temperatura` | Temperatura (°C), 2 casas decimais |
| `CPD/BR/arcondicionado/umidade` | Umidade relativa (%), 2 casas decimais |

Client ID: `ESP01S_DHT22_<chip_id>` (único por dispositivo).

Intervalo de publicação: **30 segundos** (`delay(30000)` no `loop()`).

## Configuração

**Novidade importante:** desde a última mudança, as credenciais (WiFi e
MQTT) **não ficam mais no código principal**. Elas moram no arquivo
`src/config.h`, que:

- **NÃO é versionado** — está no `.gitignore`, então o Git ignora o arquivo;
- **nunca é enviado ao repositório** público.

No lugar dele, existe um template commitado: `src/config.example.h`, que
serve de modelo com placeholders.

### Por quê?

Com as credenciais dentro do código, qualquer pessoa que acessasse o
repositório público veria **o SSID e a senha do seu WiFi** (e as
configurações do broker). Isso é uma falha grave de segurança.

Ao manter as credenciais em um arquivo local **ignorado pelo Git**, você
expõe no repositório apenas o **modelo** (`config.example.h`), sem nenhum
segredo real. Assim, quem clonar o projeto sabe exatamente o que configurar,
mas não tem acesso às suas senhas.

### Como configurar

1. Copie o template para o arquivo real:

   ```bash
   cp src/config.example.h src/config.h
   ```

2. Edite `src/config.h` com as suas credenciais:

   ```c
   #define WIFI_SSID "SEU_WIFI_SSID"
   #define WIFI_PASSWORD "SUA_SENHA_WIFI"
   #define MQTT_SERVER "test.mosquitto.org"
   #define MQTT_PORT 1883
   ```

3. Salve e compile (veja **Build e upload** abaixo).

> ⚠️ **Nunca** commite nem faça upload do `src/config.h` — ele é o seu
> segredo. Confirme que ele continua listado no `.gitignore`.

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

> **Importante (ESP-01S):** para entrar em **modo de gravação**, segure o
> **GPIO0 conectado ao GND** ao ligar/resetar a placa e só solte depois que
> o upload começar. Sem isso, o upload falha.

## Observações

- **Timeouts e watchdog** (definidos em `src/main.cpp`):
  - `WIFI_TIMEOUT = 30s` — estourou, reinicia (`timeout WiFi`).
  - `MQTT_RETRY_TIME = 60s` — estourou, reinicia (`timeout MQTT`).
  - `PUBLISH_TIMEOUT = 2min` — sem publicação no período, reinicia
    (`sem publicacao por 2min`).
- **Broker público:** `test.mosquitto.org` é um broker de testes; os dados
  ficam acessíveis publicamente. Para uso real, configure um broker próprio.
- **⚠️ Segurança no histórico do Git:** se as credenciais já estiveram
  dentro do código em versões anteriores, elas **podem ter vazado pelo
  histórico do repositório** (o Git guarda todas as versões de todos os
  arquivos). Considere trocar a senha do WiFi e, se o broker usava senha,
  trocá-la também.
