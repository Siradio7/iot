# IoT Project

Projet IoT développé avec un ESP32 et Arduino IDE.

Le projet est organisé en plusieurs fichiers afin de séparer les différentes fonctionnalités et de faciliter le travail en groupe.

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
└── README.md
```

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

## Installation avec Arduino IDE

### 1. Installer le support ESP32

Dans Arduino IDE :

```text
Tools
→ Board
→ Boards Manager
```

Rechercher `esp32` et installer **esp32 by Espressif Systems**.

### 2. Installer les bibliothèques

Le projet utilise actuellement :

- OneWire
- DallasTemperature
- Adafruit NeoPixel

Les bibliothèques peuvent être installées depuis :

```text
Sketch
→ Include Library
→ Manage Libraries
```

### 3. Ouvrir le projet

Ouvrir `iot.ino`.

Le dossier et le fichier principal doivent avoir le même nom :

```text
Arduino/
└── iot/
    └── iot.ino
```

Les autres fichiers `.cpp` et `.h` doivent rester dans ce même dossier.

### 4. Sélectionner la carte

Brancher l'ESP32 puis aller dans :

```text
Tools
→ Board
```

Sélectionner le modèle correspondant à votre carte.

Pour une carte générique, `ESP32 Dev Module` peut être utilisé si cela correspond à votre carte.

### 5. Sélectionner le port

```text
Tools
→ Port
```

Choisir le port correspondant à l'ESP32.

### 6. Compiler

Cliquer sur `Verify`.

Avant l'upload, vérifier que la compilation se termine sans erreur.

### 7. Envoyer le programme

Une fois la compilation terminée, cliquer sur `Upload`.

### 8. Moniteur série

Le projet utilise actuellement :

```cpp
Serial.begin(9600);
```

Ouvrir :

```text
Tools
→ Serial Monitor
```

et sélectionner `9600 baud`.

Exemple de sortie :

```json
{"temperature":24.83,"luminosite":3500,"etat":"CHAUFFAGE","ventilateur":0,"incendie":false}
```

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

Exemples :

```bash
git checkout -b feature/temperature-sensor
git checkout -b feature/light-sensor
git checkout -b feature/fan
git checkout -b feature/wifi
```

### Envoyer son travail

```bash
git add .
git commit -m "feat: add temperature sensor"
git push -u origin feature/temperature-sensor
```

Une Pull Request peut ensuite être créée sur GitHub pour intégrer la fonctionnalité dans `main`.

Avant la Pull Request, vérifier que le projet compile correctement avec Arduino IDE.

### Convention de commit

```text
feat:     nouvelle fonctionnalité
fix:      correction
refactor: modification de structure
docs:     documentation
chore:    maintenance/configuration
```

Exemples :

```text
feat: add temperature sensor
fix: correct fire detection threshold
refactor: separate sensor service
docs: update README
```

La branche `main` doit rester dans un état fonctionnel autant que possible.

## Ajouter un nouveau capteur

Pour ajouter un nouveau capteur :

1. créer son fichier `.h` ;
2. créer son fichier `.cpp` ;
3. ajouter le GPIO dans `pins.h` si nécessaire ;
4. ajouter les paramètres dans `config.h` si nécessaire ;
5. intégrer le capteur dans `sensor_service`.

Exemple :

```text
pressure_sensor.h
pressure_sensor.cpp
```

## Ajouter un nouvel actionneur

Pour un nouvel actionneur, créer un module dédié.

Exemple :

```text
motor.h
motor.cpp
```

Le contrôle du matériel reste dans ce module. La décision de l'activer ou non reste dans le service concerné.

## Règles du projet

- Garder `iot.ino` aussi simple que possible.
- Une fonctionnalité doit avoir une responsabilité claire.
- Centraliser les GPIO dans `pins.h`.
- Centraliser les paramètres modifiables dans `config.h`.
- Garder les détails matériels dans les modules concernés.
- Tester une fonctionnalité avant de la fusionner.
- Éviter de mélanger plusieurs fonctionnalités dans un même commit.
- Vérifier que le projet compile avec Arduino IDE avant de créer une Pull Request.

## Principe général

```text
Configuration  → config.h / pins.h
Capteurs       → *_sensor.cpp
Actionneurs    → led.cpp / fan.cpp / led_strip.cpp
Logique        → sensor_service.cpp
Données        → sensor_data.h
Point d'entrée → iot.ino
```
