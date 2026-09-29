# IoT Project

Projet IoT réalisé avec un ESP32 et Arduino IDE.

Le projet est organisé en plusieurs modules afin de séparer les différentes parties du système et de faciliter le travail en groupe.

## Structure du projet

```text
iot/
│
├── iot.ino
│
├── config/
│   ├── config.h
│   └── pins.h
│
├── sensors/
│   ├── light_sensor.h
│   ├── light_sensor.cpp
│   ├── temperature_sensor.h
│   └── temperature_sensor.cpp
│
├── actuators/
│   ├── led.h
│   └── led.cpp
│
├── communication/
│   ├── wifi.h
│   └── wifi.cpp
│
├── services/
│   ├── sensor_service.h
│   └── sensor_service.cpp
│
└── models/
    └── sensor_data.h
```

## Organisation

### `iot.ino`

C'est le fichier principal du projet. Il contient le `setup()`, le `loop()` et l'initialisation des différents modules.

On évite d'y mettre toute la logique du projet. Les fonctionnalités sont réparties dans les différents dossiers.

### `config/`

Contient les paramètres utilisés par le projet.

- `config.h` : paramètres généraux.
- `pins.h` : définition des GPIO utilisés par les composants.

Exemple :

```cpp
#define LED_PIN 5
#define TEMPERATURE_PIN 34
#define LIGHT_SENSOR_PIN 35
```

Les GPIO doivent être définis dans `pins.h` plutôt que d'être écrits directement dans plusieurs fichiers.

### `sensors/`

Contient le code lié aux capteurs.

Chaque capteur possède normalement un fichier `.h` et un fichier `.cpp`.

Par exemple :

```text
temperature_sensor.h
temperature_sensor.cpp
```

Le fichier `.h` contient les fonctions accessibles depuis les autres modules et le `.cpp` contient leur implémentation.

### `actuators/`

Contient le code permettant de contrôler les composants pilotés par l'ESP32.

Exemples : LED, ventilateur, moteur, buzzer, etc.

### `communication/`

Contient les fonctionnalités de communication avec l'extérieur.

Pour le moment, ce dossier contient la gestion du Wi-Fi. D'autres fonctionnalités comme HTTP ou MQTT pourront être ajoutées ici si nécessaire.

### `services/`

Contient la logique qui utilise plusieurs modules pour réaliser une fonctionnalité.

Par exemple, `sensor_service` peut récupérer les valeurs des différents capteurs et préparer les données à transmettre.

### `models/`

Contient les structures de données utilisées dans le projet.

Par exemple :

```cpp
struct SensorData {
    float temperature;
    int light;
};
```

## Quelques règles

- Un module doit avoir une responsabilité claire.
- Les GPIO sont centralisés dans `config/pins.h`.
- Éviter de mettre de la logique métier directement dans `iot.ino`.
- Pour un nouveau module, utiliser autant que possible un fichier `.h` et un fichier `.cpp`.
- Éviter les variables globales inutiles.
- Avant de créer un nouveau fichier, vérifier si la fonctionnalité peut être ajoutée à un module existant.

## Collaboration avec GitHub

Le projet est partagé sur GitHub afin que chaque membre puisse travailler sur sa partie sans modifier directement le travail des autres.

### Récupérer le projet

Après avoir cloné le dépôt :

```bash
git clone <URL_DU_REPOSITORY>
cd iot
```

### Créer une branche

Chaque fonctionnalité doit idéalement être développée sur une branche séparée.

Exemples :

```bash
git checkout -b feature/temperature-sensor
git checkout -b feature/light-sensor
git checkout -b feature/wifi
git checkout -b feature/led
```

### Envoyer son travail

Après les modifications :

```bash
git add .
git commit -m "feat: add temperature sensor"
git push -u origin feature/temperature-sensor
```

### Intégrer le travail

Une fois la fonctionnalité terminée, créer une Pull Request sur GitHub.

La branche `main` doit contenir une version fonctionnelle du projet.

Avant de commencer une nouvelle tâche, récupérer les dernières modifications :

```bash
git checkout main
git pull
```

Puis créer sa nouvelle branche :

```bash
git checkout -b feature/nom-de-la-fonctionnalite
```

### Convention de commit

On peut utiliser des messages simples et cohérents :

```text
feat: add temperature sensor
fix: fix wifi connection
refactor: reorganize sensor service
docs: update README
chore: update configuration
```

## Lancer le projet avec Arduino IDE

### 1. Installer Arduino IDE

Installer Arduino IDE sur son ordinateur.

Le projet utilise un ESP32. Il faut donc avoir installé le support des cartes ESP32 dans Arduino IDE.

Dans Arduino IDE :

```text
Tools
→ Board
→ Boards Manager
```

Rechercher :

```text
esp32
```

Puis installer le package ESP32 fourni par Espressif.

### 2. Ouvrir le projet

Ouvrir le fichier :

```text
iot.ino
```

avec Arduino IDE.

Le dossier du projet doit garder le nom :

```text
iot
```

avec le fichier principal :

```text
iot.ino
```

### 3. Sélectionner la carte ESP32

Brancher l'ESP32 à l'ordinateur avec un câble USB adapté.

Dans Arduino IDE :

```text
Tools
→ Board
→ esp32
→ [modèle de votre carte]
```

Sélectionner ensuite le port correspondant à l'ESP32 :

```text
Tools
→ Port
→ port de l'ESP32
```

Le nom exact de la carte et du port peut être différent selon l'ordinateur et le modèle utilisé.

### 4. Installer les bibliothèques nécessaires

Si le projet utilise des bibliothèques externes, elles doivent être installées avant la compilation.

Dans Arduino IDE :

```text
Sketch
→ Include Library
→ Manage Libraries
```

Rechercher puis installer les bibliothèques nécessaires au projet.

Les bibliothèques utilisées devront être indiquées dans cette section au fur et à mesure de l'avancement du projet.

### 5. Vérifier le code

Cliquer sur :

```text
Verify
```

ou utiliser :

```text
Cmd + R
```

sur macOS.

Cela permet de compiler le projet sans encore l'envoyer sur l'ESP32.

### 6. Envoyer le programme

Une fois la compilation réussie, cliquer sur :

```text
Upload
```

Arduino IDE compile le projet puis l'envoie sur l'ESP32.

### 7. Vérifier les messages série

Pour afficher les informations envoyées par l'ESP32 :

```text
Tools
→ Serial Monitor
```

Utiliser le même débit que celui défini dans le code, par exemple :

```cpp
Serial.begin(115200);
```

Dans ce cas, sélectionner :

```text
115200 baud
```

## Ajouter un nouveau capteur

Pour ajouter un nouveau capteur, créer les fichiers correspondants dans `sensors/`.

Exemple pour un capteur de pression :

```text
sensors/
├── pressure_sensor.h
└── pressure_sensor.cpp
```

Ajouter ensuite le GPIO dans :

```text
config/pins.h
```

Puis intégrer le capteur dans le service concerné.

Le reste du projet ne doit pas avoir besoin de connaître les détails internes du capteur.

## Ajouter un nouvel actionneur

Le principe est similaire pour un nouvel actionneur.

Exemple :

```text
actuators/
├── motor.h
└── motor.cpp
```

Le contrôle du moteur reste dans son propre module.

## Organisation du travail

La répartition peut évoluer en fonction des besoins du projet. Par exemple :

```text
Sensors          → capteurs
Actuators        → actionneurs
Communication    → Wi-Fi / HTTP / MQTT
Services         → logique du système
Integration      → iot.ino
```

L'important est de garder les responsabilités séparées afin de faciliter les modifications et les tests.

---

## À retenir

Le principe général du projet est :

```text
iot.ino
      │
      ├── services/
      │       │
      │       ├── sensors/
      │       ├── actuators/
      │       └── communication/
      │
      ├── models/
      │
      └── config/
```

Chaque partie du code doit rester à l'endroit correspondant à sa responsabilité.
