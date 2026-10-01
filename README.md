# IoT Project

Projet de régulation IoT autour d'un ESP32, développé avec Arduino IDE. Le dépôt contient le firmware, un bridge série/TCP pour macOS, un flow Node-RED sous Docker et un validateur du JSON émis par le firmware.

## Démarrage rapide (macOS)

1. Compiler et téléverser le sketch depuis Arduino IDE (voir [Firmware Arduino](#firmware-arduino)).
2. Fermer le moniteur série Arduino : le bridge doit être le seul programme qui ouvre le port USB.
3. Dans un terminal, lancer Node-RED :

    ```bash
    cd nodered
    docker compose up -d --build
    ```

4. Dans un autre terminal, configurer le port série et lancer le bridge :

    ```bash
    cd esp32-bridge
    ls /dev/cu.*
    # Renseigner le port détecté dans .env (voir Bridge et Node-RED)
    npm ci
    npm start
    ```

5. Ouvrir <http://localhost:1880/ui> pour le tableau de bord ou <http://localhost:1880> pour l'éditeur Node-RED.

Le chemin des données est `ESP32 --USB série--> bridge macOS --TCP:5001--> Node-RED (Docker)`. Le tableau de bord se met à jour lorsqu'une mesure JSON valide arrive. La compilation et le comportement électrique doivent être vérifiés avec la carte et les capteurs réellement utilisés.

Le projet n'implémente pas le Wi-Fi : les champs `net` du JSON valent `NOP`. `reporthost` est une métadonnée du modèle de cours et ne configure pas la connexion du bridge.

## Structure du projet

Les fichiers `.cpp` et `.h` sont placés à la racine du sketch afin de rester compatibles avec le fonctionnement d'Arduino IDE.

```text
iot/
│
├── iot.ino
├── config.h
├── pins.h
├── temperature_sensor.h
├── temperature_sensor.cpp
├── light_sensor.h
├── light_sensor.cpp
├── led.h
├── led.cpp
├── fan.h
├── fan.cpp
├── led_strip.h
├── led_strip.cpp
├── sensor_service.h
├── sensor_service.cpp
├── sensor_data.h
├── make_json.h
├── make_json.cpp
└── README.md
```

## Brochage et matériel

Les GPIO sont définis dans `pins.h`. Adapter ces valeurs si le montage diffère :

| Fonction | GPIO | Remarque |
| --- | ---: | --- |
| Données du DS18B20 | 23 | Bus OneWire ; prévoir la résistance de rappel requise par le capteur, généralement 4,7 kΩ entre DATA et 3,3 V |
| Entrée analogique de luminosité | 33 | Signal analogique dans la plage admissible de l'ESP32 (0 à 3,3 V) |
| LED témoin climatisation | 19 | Sortie logique, utiliser une résistance avec une LED |
| LED témoin chauffage | 21 | Sortie logique, utiliser une résistance avec une LED |
| Commande PWM du ventilateur | 27 | GPIO de commande uniquement ; piloter le moteur avec un étage de puissance adapté |
| Bande NeoPixel (5 LED) | 13 | Alimentation adaptée à la bande et masse commune avec l'ESP32 |
| LED d'alerte incendie | 2 | Sortie logique, utiliser une résistance avec une LED ; le comportement au démarrage dépend de la carte |

Ne branchez pas directement un moteur, un radiateur ou un appareil secteur sur un GPIO. Utilisez des drivers et alimentations adaptés, reliez les masses lorsque le montage le requiert et vérifiez les tensions avant mise sous tension. Le code commande des sorties témoins et un signal PWM ; il ne fournit pas à lui seul l'électronique de puissance.

La détection d'incendie est actuellement définie par `luminosite < 500` (`SEUIL_INCENDIE` dans `config.h`). Ce seuil dépend du capteur et du câblage : relevez les valeurs ADC de votre montage avant de vous y fier. L'entrée analogique utilise les valeurs 0 à 4095 de l'ESP32.

## Rôle des fichiers

### `iot.ino`

Point d'entrée du programme. Contient `setup()`, `loop()` et l'organisation générale du système. La logique détaillée reste dans les modules.

### `config.h`

Contient les paramètres du projet : seuils de température, hystérésis, paramètres PWM, seuil de détection d'incendie, période de mesure, etc.

### `pins.h`

Contient le brochage du projet. Les GPIO sont centralisés ici et ne doivent pas être répétés dans les autres fichiers.

### `temperature_sensor.h / temperature_sensor.cpp`

Gère le capteur de température DS18B20.

### `light_sensor.h / light_sensor.cpp`

Gère la lecture du capteur de luminosité.

### `led.h / led.cpp`

Gère les LEDs classiques utilisées pour la climatisation, le radiateur et l'alerte incendie.

### `fan.h / fan.cpp`

Gère le ventilateur et son signal PWM.

### `led_strip.h / led_strip.cpp`

Gère la bande de LEDs adressables et l'affichage de l'état de la régulation.

### `sensor_service.h / sensor_service.cpp`

Contient la logique principale de régulation : récupération des mesures, choix de l'état du système, calcul de la vitesse du ventilateur et détection d'incendie.

Les états possibles sont :

```text
CHAUFFAGE
REPOS
CLIMATISATION
```

### `sensor_data.h`

Contient les structures utilisées pour regrouper les données des capteurs.

Exemple :

```cpp
struct SensorData {
    float temperature;
    int luminosite;
};
```

### `make_json.h / make_json.cpp`

Construit le statut de l'ESP au format JSON imposé par le cours (sections `status`, `location`, `regul`, `info`, `net`, `reporthost`) avec la bibliothèque ArduinoJson. Les valeurs constantes (identifiant, groupe, localisation, reporthost) sont définies dans `config.h`.

Choix provisoires (à confirmer avec l'enseignant) :

- `status.regul` vaut `RUNNING` quand le chauffage ou la climatisation est actif, `HALT` sinon ;
- `status.fanspeed` est la valeur PWM du ventilateur (0 à 255) ;
- les champs de `net` valent `NOP` tant que le WiFi n'est pas utilisé ;
- `reporthost` reprend les constantes de l'exemple du cours (`127.0.0.1`, `1880`, `2`) afin de garder des nombres pour `target_port` et `sp`.

## Fonctionnement général

```text
                 iot.ino
                    │
                    ▼
             sensor_service
                    │
          ┌─────────┴─────────┐
          ▼                   ▼
 temperature_sensor       light_sensor
          │                   │
          └─────────┬─────────┘
                    ▼
               SensorData
                    │
                    ▼
             Décision système
                    │
        ┌───────────┼───────────┐
        ▼           ▼           ▼
    Chauffage      Repos    Climatisation
        │           │           │
        ▼           ▼           ▼
    Radiateur     LEDs       Ventilateur
                              + LED
```

La mesure de luminosité est également utilisée pour la détection d'incendie.

## Firmware Arduino

### Prérequis firmware

- Arduino IDE 2.x et le paquet **esp32 by Espressif Systems**, version 3.x (le code utilise l'API PWM `ledcAttach` du core ESP32 3.x).
- Bibliothèques Arduino : `OneWire`, `DallasTemperature`, `Adafruit NeoPixel` et `ArduinoJson` version 7.
- Une carte ESP32 compatible avec les GPIO déclarés dans `pins.h`.

Installer le paquet ESP32 depuis **Outils → Type de carte → Gestionnaire de cartes**, puis les bibliothèques depuis **Croquis → Inclure une bibliothèque → Gérer les bibliothèques**. Sélectionner le bon modèle de carte et le bon port dans le menu **Outils**.

### Compiler et téléverser

Ouvrir `iot.ino` dans Arduino IDE. Le dossier du sketch et le fichier `.ino` portent tous deux le nom `iot`. Cliquer sur **Vérifier**, puis **Téléverser**.

Le firmware ouvre le port série à **9600 bauds** et émet une ligne JSON par mesure, environ toutes les secondes. Pour vérifier la sortie, ouvrir le moniteur série à 9600 bauds. Le fermer avant de démarrer le bridge, car un seul processus peut utiliser le port à la fois.

Le DS18B20 est lu sur l'index 0 du bus. Si aucun capteur n'est détecté, le firmware écrit `{"erreur":"capteur de temperature non detecte"}` au lieu du statut complet ; cette ligne ne respecte pas le schéma JSON du validateur et le tableau de bord n'aura pas de statut exploitable.

### Régulation

- Chauffage demandé sous `SEUIL_BAS - HYSTERESIS` (24,7 °C par défaut).
- Climatisation demandée au-dessus de `SEUIL_HAUT + HYSTERESIS` (26,3 °C par défaut).
- Dans la zone intermédiaire, l'état précédent est conservé, sauf dans la zone neutre définie dans `sensor_service.cpp`.
- En climatisation, le PWM augmente de `VITESSE_MIN` à `VITESSE_MAX` selon la température ; il est limité à 255.
- L'alerte incendie s'active lorsque la lecture de luminosité est inférieure à `SEUIL_INCENDIE`.

Les seuils, la période de mesure, les GPIO et les champs d'identification du JSON se règlent dans `config.h` et `pins.h`.

## Validateur JSON

Le dossier `validator/` vérifie le statut avec Ajv et `schema.json`. Depuis ce dossier :

```bash
npm ci
npm test
npm run validate -- examples/valid_esp.json
```

Pour valider une mesure réelle, enregistrer une ligne complète du moniteur série dans un fichier `.json`, puis passer son chemin à `npm run validate -- chemin/vers/mesure.json`. Une ligne d'erreur de capteur n'est pas un statut conforme. Le schéma rejette les champs manquants ou supplémentaires, les mauvais types, les énumérations et valeurs hors limites, ainsi que les formats IP/MAC invalides.

## Bridge et Node-RED

Node-RED tourne dans Docker ; le bridge doit tourner sur le Mac car le conteneur n'accède pas directement au port USB. Le flow versionné dans `nodered/data/flows.json` se connecte en client TCP à `host.docker.internal:5001`, découpe le flux sur les retours à la ligne puis convertit le JSON. Docker Desktop pour macOS fournit cette adresse hôte.

### Prérequis Docker et Node.js

- Docker Desktop installé et démarré.
- Node.js récent (Node.js 20 LTS ou plus récent) et npm.
- L'ESP32 branché, firmware téléversé et moniteur série fermé.

### Lancer Node-RED

Depuis la racine du dépôt :

```bash
cd nodered
docker compose up -d --build
docker compose logs -f node-red
```

L'image installe le tableau de bord classique `node-red-dashboard` et `node-red-contrib-ui-led`. Les données sont montées depuis `nodered/data/`. Accès : <http://localhost:1880> (éditeur) et <http://localhost:1880/ui> (tableau de bord).

### Lancer le bridge série/TCP

Dans un autre terminal, repérer le port série macOS :

```bash
ls /dev/cu.*
```

Dans `esp32-bridge/.env` (fichier local ignoré par Git), définir le port correspondant à votre carte, par exemple :

```dotenv
SERIAL_PORT=/dev/cu.usbmodem12301
BAUD_RATE=9600
TCP_PORT=5001
```

Le nom exact dépend de la carte et du câble USB ; ne recopiez pas l'exemple sans vérifier la sortie de `ls`. Puis démarrer le bridge :

```bash
cd esp32-bridge
npm ci
npm start
```

Le bridge affiche la connexion série, puis les lignes reçues de l'ESP32. Il écoute le port TCP 5001 et relaie les lignes vers Node-RED. Les variables `SERIAL_PORT`, `BAUD_RATE` et `TCP_PORT` peuvent aussi être fournies dans l'environnement du processus.

### Arrêter les services

Arrêter le bridge avec `Ctrl+C`, puis arrêter Node-RED :

```bash
cd nodered
docker compose down
```

Les changements de flow déployés depuis l'éditeur sont persistés dans `nodered/data/flows.json`. Le `REPORT_TARGET_IP` et le port 1880 dans `config.h` appartiennent au JSON descriptif du cours ; le bridge actuel utilise le port TCP 5001 et ne consomme pas ces champs.

### Dépannage

| Symptôme | Vérifications |
| --- | --- |
| Le bridge ne peut pas ouvrir le port série | Fermer le moniteur série, vérifier `ls /dev/cu.*`, puis corriger `SERIAL_PORT` dans `esp32-bridge/.env`. |
| Le bridge indique une erreur à 9600 bauds ou ne reçoit rien | Vérifier le téléversement, le débit série à 9600 et que le port choisi correspond à l'ESP32. |
| Node-RED ne reçoit pas de données | Vérifier que le conteneur tourne (`docker compose ps`), que le bridge écoute sur 5001 et que son log signale une connexion Node-RED. Relancer le bridge après Node-RED si besoin. |
| Le tableau de bord ne montre pas de statut | Vérifier les lignes JSON du log du bridge ; une erreur de capteur n'a pas la structure attendue. Contrôler le DS18B20, son câblage et sa résistance de rappel. |
| La détection incendie est toujours active ou inactive | Lire la valeur `light` du JSON et ajuster `SEUIL_INCENDIE` dans `config.h` pour le capteur installé. |
| La bande LED ou le ventilateur ne réagit pas | Vérifier GPIO, alimentation, masse commune et étage de commande ; le GPIO ne peut pas fournir le courant d'un moteur ou d'une bande. |

## Collaboration avec GitHub

Le dépôt GitHub contient la version commune du projet. Chaque membre travaille de préférence sur sa propre branche.

### Récupérer le projet

```bash
git clone <URL_DU_REPOSITORY>
cd iot
```

### Commencer une nouvelle fonctionnalité

```bash
git checkout main
git pull
git checkout -b feature/nom-de-la-fonctionnalite
```

### Envoyer son travail

```bash
git add .
git commit -m "feat: add temperature sensor"
git push -u origin feature/nom-de-la-fonctionnalite
```

Avant la Pull Request, vérifier que le projet compile correctement avec Arduino IDE et lancer `npm test` dans `validator/`.

Conventions de commit : `feat` (fonctionnalité), `fix` (correction), `refactor` (structure), `docs` (documentation) et `chore` (maintenance).

## Ajouter un module

Pour un nouveau capteur ou actionneur, créer un fichier `.h` et `.cpp`, centraliser son GPIO dans `pins.h` et ses paramètres dans `config.h`, puis l'intégrer au service concerné. Garder `iot.ino` dédié à l'initialisation et à l'orchestration.
