# IoT Project

Projet de régulation de température autour d'un ESP32, développé avec Arduino IDE (groupe GR_F).

L'ESP32 mesure la température et la luminosité, pilote un chauffage, une climatisation, un ventilateur, une bande de LEDs et une alerte incendie, et envoie son état au format JSON du cours. Un tableau de bord Node-RED affiche cet état et permet de modifier les seuils de régulation à distance.

```text
ESP32 ──USB série──▶ esp32-bridge (Mac) ──TCP 5001──▶ Node-RED (Docker) ──▶ tableau de bord
      ◀─────────────────────────────────────────────────── seuils (curseurs)
```

Sommaire :

- [Démarrage rapide](#démarrage-rapide-macos)
- [Guide de test (critères d'évaluation)](#guide-de-test-critères-dévaluation)
- [Structure du projet](#structure-du-projet) et [rôle des fichiers](#rôle-des-fichiers)
- [Firmware Arduino](#firmware-arduino) : [régulation](#régulation), [détection d'incendie](#détection-dincendie), [JSON envoyé](#json-envoyé-par-lesp)
- [Validateur JSON](#validateur-json)
- [Bridge et Node-RED](#bridge-et-node-red) : [tableau de bord](#tableau-de-bord), [retour vers l'ESP](#retour-vers-lesp-seuils-depuis-le-dashboard)
- [Dépannage](#dépannage)

## Démarrage rapide (macOS)

Prérequis : Arduino IDE 2 avec le support ESP32 et les bibliothèques (voir [Prérequis firmware](#prérequis-firmware)), Docker Desktop démarré, Node.js 20 ou plus récent.

1. **Téléverser le firmware** : ouvrir `iot.ino` dans Arduino IDE, choisir la carte et le port, cliquer sur **Téléverser**.
2. **Fermer le moniteur série** d'Arduino IDE : un seul programme peut utiliser le port USB à la fois.
3. **Lancer Node-RED**, depuis la racine du dépôt :

    ```bash
    cd nodered
    docker compose up -d --build
    ```

4. **Lancer le bridge**, dans un autre terminal, depuis la racine du dépôt :

    ```bash
    cd esp32-bridge
    npm ci        # une seule fois
    npm start
    ```

    Le port série par défaut est `/dev/cu.usbserial-0001`. S'il est différent (voir `ls /dev/cu.*`), le définir dans `esp32-bridge/.env` (voir [Lancer le bridge](#lancer-le-bridge-sérietcp)).

5. **Ouvrir le tableau de bord** : <http://localhost:1880/ui> (éditeur Node-RED : <http://localhost:1880>).

Le terminal du bridge doit afficher `ESP32 connecté`, puis `Node-RED connecté`, puis une ligne `ESP32 → {"status":...}` par seconde. Le tableau de bord se remplit en quelques secondes.

## Guide de test (critères d'évaluation)

Ce guide reprend la grille d'évaluation, critère par critère. Il suppose que le [démarrage rapide](#démarrage-rapide-macos) est fait : firmware téléversé, Node-RED et bridge lancés, tableau de bord ouvert.

**Où observer :**

| Observation | Où |
| --- | --- |
| LEDs de la carte | LED rouge (GPIO 21, radiateur), LED verte (GPIO 19, clim), LED intégrée bleue (GPIO 2, incendie), bande de 5 LEDs (GPIO 13), ventilateur (GPIO 27) |
| JSON brut envoyé chaque seconde | terminal du bridge : lignes `ESP32 → {...}` |
| Tableau de bord | <http://localhost:1880/ui> |

**Méthode : tester la régulation sans chauffer ni refroidir.** Les curseurs « Seuil haut » et « Seuil bas » du tableau de bord (carte Seuils) modifient les seuils de l'ESP en direct. Il suffit donc de placer les seuils au-dessus ou en dessous de la température actuelle de la pièce, notée **T** ci-dessous (jauge Température). Les textes « Seuil bas » / « Seuil haut » affichent les seuils **réellement appliqués** par l'ESP.

> Le seuil bas doit toujours rester **inférieur** au seuil haut, sinon l'ESP refuse la commande. Pour monter les deux seuils, déplacer d'abord le seuil haut ; pour les descendre, déplacer d'abord le seuil bas.
>
> Au redémarrage de l'ESP (relance du bridge, téléversement), les seuils reviennent à 25 °C / 26 °C.

### 1. Régulation (radiateur LED rouge, climatisation LED verte)

| Étape | Curseurs (dans l'ordre) | Attendu sur la carte | Attendu sur le tableau de bord / JSON |
| --- | --- | --- | --- |
| Chauffage | seuil haut = T + 3, puis seuil bas = T + 2 | **LED rouge** allumée, LED verte éteinte, ventilateur arrêté | bannière verte `CHAUFFAGE`, led Radiateur allumée ; `"heat":"ON"`, `"regul":"RUNNING"` |
| Repos | seuil bas = T − 1, puis seuil haut = T + 1 | LEDs rouge et verte éteintes, ventilateur arrêté | bannière orange `REPOS` ; `"heat":"OFF"`, `"cold":"OFF"`, `"regul":"HALT"` |
| Climatisation | seuil bas = T − 4, puis seuil haut = T − 2 | **LED verte** allumée, LED rouge éteinte, ventilateur en marche | bannière rouge `CLIMATISATION`, led Clim allumée ; `"cold":"ON"`, `"regul":"RUNNING"` |

Les basculements utilisent une hystérésis de 0,3 °C (voir [Régulation](#régulation)) : près d'un seuil, l'état précédent est conservé.

### 2. Ventilateur : régulation progressive

En climatisation, la vitesse augmente avec l'écart entre T et le seuil haut. Partir de l'étape Climatisation ci-dessus, **descendre d'abord le seuil bas** à T − 6, puis changer le seuil haut :

| Écart T − seuil haut | PWM (`fanspeed` du JSON) | Jauge Ventilateur |
| --- | --- | --- |
| 0 (ou clim maintenue par l'hystérésis) | 80 | 31 % |
| 1 °C | 115 | 45 % |
| 2 °C | 150 | 59 % |
| 3 °C | 185 | 73 % |
| 4 °C | 220 | 86 % |
| 5 °C ou plus | 255 | 100 % |

Le ventilateur est à 0 hors climatisation et en cas d'incendie.

### 3. Bande de LEDs : trois couleurs selon la température

| État (étapes du test 1) | Couleur de la bande |
| --- | --- |
| Climatisation (T au-dessus du seuil haut) | **rouge** |
| Repos (T entre les seuils) | **orange** |
| Chauffage (T sous le seuil bas) | **vert** |

### 4. Incendie : LED 2 et arrêt du ventilateur

La règle et ses choix sont détaillés dans [Détection d'incendie](#détection-dincendie). Il faut de la **lumière forte et de la chaleur en même temps**. Après le démarrage de l'ESP, attendre environ 30 s.

| Test | Attendu |
| --- | --- |
| Lampe (téléphone) collée au capteur de lumière, seule | pas d'incendie : `"fire":false` |
| Capteur de température chauffé seul (le tenir dans la main) au-delà de 28 °C | pas d'incendie |
| Lampe **et** capteur de température chauffé, au moins 5 s | **LED bleue (GPIO 2) allumée**, bannière `INCENDIE` qui clignote, led Feu allumée, `"fire":true` |
| Même test pendant la climatisation (étape 3 du test 1) | le **ventilateur s'arrête** (`"fanspeed":0`) alors que `"cold"` reste `"ON"` |
| Flash d'une seconde | pas d'incendie (moyennes glissantes) |
| Retirer lampe et chaleur | l'alerte s'éteint après quelques secondes |

La lampe doit éclairer plus fort que la lumière ambiante : en plein soleil, faire le test à l'ombre.

**Méthode (moyenne)** : la luminosité et la température sont chacune moyennées sur les 5 dernières mesures (`light_sensor.cpp`, `sensor_service.cpp`), et la lumière est comparée à une référence de lumière ambiante.

### 5. JSON : bibliothèque et validateur

| Test | Attendu |
| --- | --- |
| Lire une ligne `ESP32 → {...}` dans le terminal du bridge | une ligne JSON par seconde au format du cours (voir [JSON envoyé](#json-envoyé-par-lesp)), construite avec ArduinoJson dans `make_json.cpp` |
| `cd validator && npm ci && npm test` | `Tous les tests passent.` : 2 JSON valides acceptés, 11 JSON non conformes refusés avec la raison |
| Copier une ligne réelle (sans `ESP32 → `) dans un fichier, puis `node validate.js chemin/fichier.json` dans `validator/` | `✅ ... : VALIDE` |
| Valider n'importe quel autre fichier JSON | `✅ VALIDE` ou `❌ NON VALIDE` avec la liste des erreurs ; aucun plantage (fichier absent, vide, mal formé) |

### 6. Tableau de bord : informations et retour vers l'ESP

| Test | Attendu |
| --- | --- |
| Ouvrir <http://localhost:1880/ui> | les cartes décrites dans [Tableau de bord](#tableau-de-bord) se mettent à jour chaque seconde |
| Déplacer le curseur Seuil bas, puis Seuil haut (seuil bas < seuil haut) | le terminal du bridge affiche `Node-RED → {"lt":...}` / `{"ht":...}`, puis les textes Seuil bas / Seuil haut prennent les nouvelles valeurs (`regul.lt` / `regul.ht` du JSON) |
| Mettre le seuil bas au-dessus du seuil haut | commande refusée : les textes ne changent pas, Node-RED affiche un avertissement `commande refusee` |

Test du retour vers l'ESP sans Node-RED : bridge arrêté, moniteur série d'Arduino IDE à 9600 bauds avec la fin de ligne « New Line », envoyer `{"lt":22,"ht":24}` puis `{"ht":"abc"}` (refusée).

### 7. Code : structure et commentaires

- Un module par capteur ou actionneur (`*_sensor`, `led`, `fan`, `led_strip`), la logique dans `sensor_service`, le JSON dans `make_json`, les commandes reçues dans `command` ; `iot.ino` ne fait qu'orchestrer (voir [Rôle des fichiers](#rôle-des-fichiers)).
- Tous les GPIO sont dans `pins.h`, tous les réglages (avec leur unité) dans `config.h`.
- Chaque fichier commence par un commentaire de rôle ; les choix non évidents (hystérésis, vitesse du ventilateur, détection d'incendie, lecture des commandes) sont expliqués dans le code.

## Structure du projet

Les fichiers `.cpp` et `.h` du firmware sont à la racine du sketch, comme l'exige Arduino IDE. Les autres dossiers ne sont pas compilés par Arduino IDE.

```text
iot/
├── iot.ino                  point d'entrée du firmware
├── config.h                 réglages (seuils, PWM, incendie, JSON...)
├── pins.h                   brochage
├── temperature_sensor.h/.cpp
├── light_sensor.h/.cpp
├── led.h/.cpp
├── fan.h/.cpp
├── led_strip.h/.cpp
├── sensor_service.h/.cpp
├── sensor_data.h
├── make_json.h/.cpp
├── command.h/.cpp
├── validator/               validateur JSON (Node.js, Ajv)
│   ├── schema.json
│   ├── validator.js
│   ├── validate.js
│   ├── test.js
│   └── examples/
├── esp32-bridge/            bridge série ⇄ TCP (Node.js)
│   └── bridge.js
├── nodered/                 Node-RED dans Docker
│   ├── docker-compose.yml
│   ├── Dockerfile
│   └── data/flows.json      flow et tableau de bord (seul fichier de data/ versionné)
└── README.md
```

## Brochage et matériel

Les GPIO sont définis dans `pins.h`. Adapter ces valeurs si le montage diffère :

| Fonction | GPIO | Remarque |
| --- | ---: | --- |
| Données du DS18B20 | 23 | Bus OneWire ; prévoir la résistance de rappel requise par le capteur, généralement 4,7 kΩ entre DATA et 3,3 V |
| Entrée analogique de luminosité | 33 | Signal analogique de 0 à 3,3 V, lu de 0 à 4095 |
| LED climatisation (verte) | 19 | Sortie logique, utiliser une résistance avec une LED |
| LED chauffage / radiateur (rouge) | 21 | Sortie logique, utiliser une résistance avec une LED |
| Commande PWM du ventilateur | 27 | GPIO de commande uniquement ; piloter le moteur avec un étage de puissance adapté |
| Bande NeoPixel (5 LED) | 13 | Alimentation adaptée à la bande et masse commune avec l'ESP32 |
| LED d'alerte incendie (LED 2, bleue intégrée) | 2 | Le comportement au démarrage dépend de la carte |

Ne branchez pas directement un moteur, un radiateur ou un appareil secteur sur un GPIO. Utilisez des drivers et alimentations adaptés, reliez les masses lorsque le montage le requiert et vérifiez les tensions avant mise sous tension.

Le capteur de luminosité est **inversé** : plus de lumière donne une valeur ADC plus **basse**.

## Rôle des fichiers

| Fichier | Rôle |
| --- | --- |
| `iot.ino` | Point d'entrée : `setup()` initialise les modules ; `loop()` lit les commandes, puis toutes les `PERIODE_MESURE` ms mesure, décide, pilote les actionneurs et envoie le JSON. |
| `config.h` | Tous les réglages : seuils de température au démarrage et bornes, hystérésis, PWM, détection d'incendie, période de mesure, constantes du JSON. |
| `pins.h` | Brochage ; les GPIO ne sont définis qu'ici. |
| `temperature_sensor` | Lecture du DS18B20. |
| `light_sensor` | Lecture de la photorésistance et moyenne glissante. |
| `led` | LEDs témoins climatisation, radiateur et alerte incendie. |
| `fan` | Ventilateur en PWM. |
| `led_strip` | Bande NeoPixel en vert, orange ou rouge. |
| `sensor_service` | Logique : mesures, décision de régulation avec hystérésis, vitesse du ventilateur, seuils modifiables, détection d'incendie. États : `CHAUFFAGE`, `REPOS`, `CLIMATISATION`. |
| `sensor_data.h` | Structure `SensorData` (température, luminosité moyennée). |
| `make_json` | Construction du statut JSON du cours avec ArduinoJson. |
| `command` | Lecture non bloquante des commandes JSON reçues sur le port série et application des seuils. |

## Fonctionnement général

```text
                         iot.ino (loop)
                               │
        ┌──────────────────────┼─────────────────────────┐
        ▼                      ▼                         ▼
     command          sensor_service_read          make_json
 (seuils reçus)     temperature + light (moy.)   (ligne JSON → série)
        │                      │
        ▼                      ▼
  seuil bas / haut ──▶  décision de régulation ──▶ CHAUFFAGE / REPOS / CLIMATISATION
                               │                     │          │           │
                               │                LED rouge   (éteint)   LED verte + ventilateur
                               │                bande verte bande orange bande rouge
                               ▼
                     détection d'incendie ──▶ LED 2 + arrêt du ventilateur
```

## Firmware Arduino

### Prérequis firmware

- Arduino IDE 2.x et le paquet **esp32 by Espressif Systems**, version 3.x (le code utilise l'API PWM `ledcAttach` du core ESP32 3.x).
- Bibliothèques Arduino : `OneWire`, `DallasTemperature`, `Adafruit NeoPixel` et `ArduinoJson` version 7.
- Une carte ESP32 compatible avec les GPIO déclarés dans `pins.h`.

Installer le paquet ESP32 depuis **Outils → Type de carte → Gestionnaire de cartes**, puis les bibliothèques depuis **Croquis → Inclure une bibliothèque → Gérer les bibliothèques**. Sélectionner le bon modèle de carte et le bon port dans le menu **Outils**.

### Compiler et téléverser

Ouvrir `iot.ino` dans Arduino IDE. Cliquer sur **Vérifier**, puis **Téléverser**. Arrêter le bridge (`Ctrl+C`) avant de téléverser : le port USB doit être libre.

Le firmware ouvre le port série à **9600 bauds** et émet une ligne JSON par mesure, environ toutes les secondes. Pour la voir sans Node-RED, ouvrir le moniteur série à 9600 bauds (bridge arrêté).

Si le DS18B20 n'est pas détecté, le firmware écrit `{"erreur":"capteur de temperature non detecte"}` au lieu du statut et ne pilote pas les actionneurs. Le tableau de bord ignore ces lignes.

### Régulation

Avec les seuils par défaut (25 °C et 26 °C, hystérésis 0,3 °C) :

```text
T < 24,7          → CHAUFFAGE      (LED rouge, bande verte)
24,7 ≤ T ≤ 25,3   → état précédent conservé
25,3 < T < 25,7   → REPOS          (LEDs éteintes, bande orange)
25,7 ≤ T ≤ 26,3   → état précédent conservé
T > 26,3          → CLIMATISATION  (LED verte, ventilateur, bande rouge)
```

- Conserver l'état précédent près des seuils évite que le chauffage ou la climatisation s'allument et s'éteignent à chaque petite variation de la mesure.
- **Ventilateur** (climatisation uniquement) : `PWM = VITESSE_MIN + (T − seuil haut) / ECART_VITESSE_MAX × (VITESSE_MAX − VITESSE_MIN)`, borné entre 80 et 255 ; il atteint 255 à 5 °C au-dessus du seuil haut. Tant que la climatisation est maintenue par l'hystérésis sous le seuil haut, il tourne à `VITESSE_MIN`. Il est arrêté en cas d'incendie.
- **Bande de LEDs** : rouge en climatisation, orange au repos, verte en chauffage ; elle suit les mêmes basculements avec hystérésis.
- `SEUIL_BAS` et `SEUIL_HAUT` (`config.h`) sont les seuils **au démarrage** : le tableau de bord peut les modifier ensuite (voir [Retour vers l'ESP](#retour-vers-lesp-seuils-depuis-le-dashboard)).
- La régulation utilise la température instantanée.

### Détection d'incendie

Un feu produit de la lumière **et** de la chaleur. L'incendie n'est déclaré que si les deux conditions sont vraies en même temps :

```text
incendie = lumière forte ET température moyenne > SEUIL_FEU_TEMPERATURE (28 °C)
```

| Condition | Calcul | Réglages (`config.h`) |
| --- | --- | --- |
| Chaleur | moyenne glissante des 5 dernières températures > 28 °C | `SEUIL_FEU_TEMPERATURE`, `NB_ECHANTILLONS_TEMPERATURE` |
| Lumière forte | moyenne glissante des 5 dernières luminosités au moins 30 % plus basse qu'une **référence** de lumière ambiante | `VARIATION_LUMIERE_FEU`, `NB_ECHANTILLONS_LUMIERE` |

Effets d'un incendie : la LED d'alerte (GPIO 2) s'allume, le ventilateur est arrêté pour ne pas attiser le feu, `status.fire` vaut `true` dans le JSON et la led Feu du tableau de bord s'allume.

**Pourquoi ces choix :**

- **Lumière ET chaleur** : une lampe seule ou une pièce chaude sans flamme ne déclenchent pas l'alerte.
- **Moyennes glissantes** : un pic isolé (flash, reflet, lecture parasite) ne suffit pas ; il faut que la situation dure quelques secondes.
- **Référence de lumière plutôt que seuil fixe** : la lumière ambiante varie énormément selon l'heure et le capteur (environ 100 en plein jour, 500 à 4000 à l'intérieur selon le montage). Le capteur étant inversé, « lumière forte » signifie une valeur nettement **plus basse** que la référence.

**La référence de lumière :**

- elle part de la première mesure, puis rattrape à chaque mesure 2 % de l'écart avec la lumière actuelle (`VITESSE_REFERENCE`) : le jour qui se lève ou la nuit qui tombe la déplacent sans alerte, alors qu'une lampe ou une flamme qui apparaît en quelques secondes crée un grand écart ;
- tant que la lumière est forte, elle reste figée : on peut éclairer le capteur puis le chauffer, le feu reste détecté. Ce gel est limité à `GEL_REFERENCE_MAX` (120 mesures, environ 2 min) pour qu'un changement durable de la lumière du jour finisse par être considéré comme normal.

Les tests sont décrits dans le [guide de test](#4-incendie--led-2-et-arrêt-du-ventilateur).

**Limites connues :**

- une lampe plus faible que la lumière du jour sur le capteur n'est pas détectée ;
- un changement brutal de la lumière du jour (nuage qui s'écarte) dans une pièce déjà au-dessus de 28 °C peut donner une fausse alerte ;
- 28 °C est volontairement bas pour pouvoir tester en chauffant le capteur à la main ; un détecteur réel utiliserait plutôt 45 à 50 °C, ou une hausse rapide de la température, et d'autres capteurs (fumée, flamme infrarouge).

### JSON envoyé par l'ESP

Chaque mesure produit une ligne JSON au format imposé par le cours, construite avec **ArduinoJson** (`make_json.cpp`). Exemple, mis en forme pour la lecture (l'ESP l'envoie sur une seule ligne) :

```json
{
  "status": {"temperature": 25.5, "light": 520, "regul": "HALT", "fire": false,
             "heat": "OFF", "cold": "OFF", "fanspeed": 0},
  "location": {"room": "312", "gps": {"lat": 43.62454, "lon": 7.050628}, "address": "Les lucioles"},
  "regul": {"lt": 25, "ht": 26},
  "info": {"ident": "ESP32", "user": "GR_F", "loc": "A Biot"},
  "net": {"uptime": "NOP", "ssid": "NOP", "mac": "NOP", "ip": "NOP"},
  "reporthost": {"target_ip": "127.0.0.1", "target_port": 1880, "sp": 2}
}
```

| Champ | Contenu |
| --- | --- |
| `status.temperature` | température instantanée (°C) |
| `status.light` | luminosité moyennée (ADC 0 à 4095, capteur inversé) |
| `status.regul` | `RUNNING` si chauffage ou climatisation actif, `HALT` au repos |
| `status.fire` | incendie détecté |
| `status.heat` / `status.cold` | `ON` / `OFF` |
| `status.fanspeed` | PWM du ventilateur (0 à 255) |
| `regul.lt` / `regul.ht` | seuils bas / haut **actuellement appliqués** |
| `location`, `info`, `reporthost` | constantes définies dans `config.h` |
| `net` | `NOP` : le Wi-Fi n'est pas utilisé |

Choix d'interprétation du modèle (à confirmer avec l'enseignant) : la signification de `status.regul`, `fanspeed` en PWM plutôt qu'en tours par minute, `net` à `NOP`, `reporthost` aux valeurs de l'exemple du cours (descriptif, non utilisé par le bridge).

## Validateur JSON

Le dossier `validator/` vérifie qu'un fichier JSON respecte le modèle du cours, avec la bibliothèque [Ajv](https://ajv.js.org/) et le schéma `schema.json`.

```bash
cd validator
npm ci                                   # une seule fois
npm test                                 # tous les exemples de examples/
node validate.js examples/valid_esp.json # un ou plusieurs fichiers
```

- `npm test` vérifie que les fichiers `valid_*.json` sont acceptés et `invalid_*.json` refusés.
- Pour une mesure réelle : copier une ligne complète du terminal du bridge (sans le préfixe `ESP32 → `) ou du moniteur série dans un fichier, puis `node validate.js chemin/vers/fichier.json`.
- Le schéma exige toutes les sections et tous les champs, refuse les champs supplémentaires, les mauvais types, les valeurs non autorisées (`ON`/`OFF`, `RUNNING`/`HALT`), les valeurs hors limites et les formats MAC/IP invalides (`NOP` accepté).
- Le programme ne plante jamais : fichier absent, dossier, fichier vide ou syntaxe invalide donnent un message clair. Code de sortie : 0 si tout est valide, 1 sinon.
- `validator.js` ne fait aucune entrée/sortie et pourra être réutilisé par un serveur (TP5).

## Bridge et Node-RED

Node-RED tourne dans Docker ; sur macOS, un conteneur n'accède pas au port USB. Le bridge (`esp32-bridge/bridge.js`) tourne donc sur le Mac : il lit les lignes de l'ESP32, les envoie à Node-RED en TCP sur le port 5001, et renvoie à l'ESP32 ce que Node-RED lui envoie.

### Lancer Node-RED

Depuis la racine du dépôt :

```bash
cd nodered
docker compose up -d --build
```

L'image (`nodered/Dockerfile`) ajoute à Node-RED les palettes `node-red-dashboard` et `node-red-contrib-ui-led`. Le flow est lu depuis `nodered/data/flows.json`. Accès : <http://localhost:1880> (éditeur) et <http://localhost:1880/ui> (tableau de bord).

> Node-RED ne lit `flows.json` qu'au démarrage. Après un `git pull` qui modifie le flow, lancer `docker compose restart` dans `nodered/` puis recharger l'éditeur **avant** de cliquer sur Déployer, sinon l'ancienne version écrase la nouvelle.

### Lancer le bridge série/TCP

Valeurs par défaut : port série `/dev/cu.usbserial-0001`, 9600 bauds, port TCP 5001. Pour les changer, créer `esp32-bridge/.env` (fichier local ignoré par Git) :

```dotenv
SERIAL_PORT=/dev/cu.usbserial-0001
BAUD_RATE=9600
TCP_PORT=5001
```

Le nom du port dépend de la carte et du câble : le vérifier avec `ls /dev/cu.*`. Puis :

```bash
cd esp32-bridge
npm ci
npm start
```

Le bridge affiche `ESP32 →` pour chaque ligne reçue de l'ESP32 et `Node-RED →` pour chaque commande envoyée vers l'ESP32. Il se reconnecte seul si l'ESP32 est débranché ou si le port est occupé.

> À l'ouverture du port série, l'ESP32 redémarre : ses seuils reviennent aux valeurs de `config.h` (25 °C / 26 °C).

### Tableau de bord

Le flow Node-RED : `tcp in` (connexion au bridge) → `json` (texte → objet) → fonction `Extraction statut` (une sortie par widget, ignore les lignes d'erreur) → widgets.

Les cartes forment une grille de 4 colonnes, sur 2 lignes de hauteur égale, dans un cadre :

| Ligne | Carte | Contenu |
| --- | --- | --- |
| 1 | État du système | bannière d'état, leds Feu, Clim, Radiateur |
| 1 | Température | jauge (°C) |
| 1 | Ventilateur | jauge en % (PWM × 100 / 255) |
| 1 | Luminosité | jauge (valeur moyennée, 0 à 4095) |
| 2 | Historique (2 colonnes) | courbe de la température |
| 2 | Seuils | seuil bas et seuil haut appliqués par l'ESP, chacun suivi de son curseur |
| 2 | Informations | pièce, adresse, identifiant de l'ESP et groupe, heure de la dernière mesure reçue |

Chaque widget a une taille fixe pour que les cartes d'une même ligne aient la même hauteur. La grille est prévue pour un écran d'au moins 1320 px de large.

L'interface est personnalisée avec deux nœuds `ui_template` :

- **Bannière d'état** : affiche le mode de régulation avec la même couleur que la bande LED (rouge `CLIMATISATION`, orange `REPOS`, vert `CHAUFFAGE`) et une explication ; en cas d'incendie, elle affiche `INCENDIE` en rouge foncé et clignote.
- **Style du tableau de bord** : feuille CSS ajoutée à la page (cadre autour de la grille, fond, barre de titre, cartes arrondies avec ombre, titres des cartes, couleurs de la bannière).

### Retour vers l'ESP (seuils depuis le dashboard)

```text
curseur → fonction Commande seuils → tcp out → bridge (TCP 5001) → port série → ESP32 (command.cpp)
```

Format d'une commande : une ligne JSON terminée par `\n`, avec les clés `lt` (seuil bas) et/ou `ht` (seuil haut) de la section `regul` du modèle du cours :

```json
{"ht":27}
{"lt":24}
{"lt":24,"ht":27}
```

Règles appliquées par `command.cpp` :

- les valeurs doivent être des nombres, compris entre `SEUIL_MIN` et `SEUIL_MAX` (15 et 35 °C) ;
- le seuil bas doit rester strictement inférieur au seuil haut ;
- toute autre clé, un JSON invalide ou une ligne de plus de `COMMAND_BUFFER_SIZE` caractères est refusé.

Une commande refusée ne modifie rien et l'ESP répond `{"erreur":"commande refusee : ..."}` (affiché comme avertissement dans Node-RED). Une commande acceptée apparaît dans `regul.lt` / `regul.ht` des lignes suivantes, donc dans les textes Seuil bas / Seuil haut du tableau de bord.

### Arrêter les services

Arrêter le bridge avec `Ctrl+C`, puis Node-RED :

```bash
cd nodered
docker compose down
```

Les changements de flow déployés depuis l'éditeur sont enregistrés dans `nodered/data/flows.json`.

## Dépannage

| Symptôme | Vérifications |
| --- | --- |
| Le bridge affiche `Resource busy` | Le moniteur série d'Arduino IDE (ou un téléversement) utilise le port : le fermer. |
| Le bridge ne trouve pas le port | Vérifier `ls /dev/cu.*` et corriger `SERIAL_PORT` dans `esp32-bridge/.env`. |
| Le téléversement échoue | Arrêter le bridge (`Ctrl+C`) avant de téléverser. |
| Tableau de bord vide, nœud `ESP32 (bridge)` « Déconnecté » | Le bridge n'est pas lancé, ou Docker Desktop est arrêté (`docker compose ps` dans `nodered/`). Node-RED se reconnecte seul toutes les 10 s. |
| Les curseurs n'ont aucun effet (pas de `Node-RED →` dans le bridge) | Le nœud `Vers ESP32` a perdu la connexion après une relance du bridge : cliquer sur Déployer (ou `docker compose restart`). |
| Une commande est refusée | Respecter seuil bas < seuil haut, entre 15 et 35 °C (voir le message `commande refusee` dans Node-RED). |
| Les seuils sont revenus à 25 / 26 | L'ESP a redémarré (relance du bridge, téléversement) : c'est normal. |
| L'éditeur Node-RED montre un ancien flow | Après un `git pull`, `docker compose restart` dans `nodered/` puis recharger la page. |
| La détection incendie ne se déclenche pas | Il faut une lumière forte **et** une température moyenne > 28 °C en même temps ; en plein soleil, tester à l'ombre. Ajuster `VARIATION_LUMIERE_FEU` et `SEUIL_FEU_TEMPERATURE` dans `config.h` si besoin. |
| `{"erreur":"capteur de temperature non detecte"}` | Vérifier le câblage du DS18B20 et sa résistance de rappel. |
| La bande LED ou le ventilateur ne réagit pas | Vérifier GPIO, alimentation, masse commune et étage de commande ; un GPIO ne peut pas fournir le courant d'un moteur ou d'une bande. |

## Collaboration avec GitHub

Le dépôt GitHub contient la version commune du projet ; la branche principale est `master`.

```bash
git clone <URL_DU_REPOSITORY>
cd iot
git checkout -b feature/nom-de-la-fonctionnalite   # travailler sur une branche
git add <fichiers>
git commit -m "feat: description"
git push -u origin feature/nom-de-la-fonctionnalite
```

Avant de fusionner : vérifier que le projet compile avec Arduino IDE et lancer `npm test` dans `validator/`. Après un `git pull` qui modifie `nodered/data/flows.json`, redémarrer Node-RED (voir [Lancer Node-RED](#lancer-node-red)).

Conventions de commit : `feat` (fonctionnalité), `fix` (correction), `refactor` (structure), `docs` (documentation) et `chore` (maintenance).

## Ajouter un module

Pour un nouveau capteur ou actionneur, créer un fichier `.h` et `.cpp`, centraliser son GPIO dans `pins.h` et ses paramètres dans `config.h`, puis l'intégrer au service concerné. Garder `iot.ino` dédié à l'initialisation et à l'orchestration.
