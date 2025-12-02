# SYSTÈME DE DÉTECTION D'ACCIDENTS ET DE SUIVI VITAL IOT

**Université Sidi Mohamed Ben Abdellah de Fès** **École Supérieure de Technologie** **Département :** Génie Électrique & Systèmes Intelligents  
**Filière :** Systèmes Embarqués (2ème Année)  

**Année Universitaire :** 2024-2025  

**Encadré par :** Pr. Assad EL Makhloufi  

**Réalisé par :** 1. Mohammed Amine Darri  
2. Hamza Khadra  
3. Salma Drissi EL-Bouzaidi  

---

## Sommaire
1. [Introduction](#introduction)
2. [Objectifs du Projet (Mise à Jour)](#objectifs)
3. [Nouvelle Architecture Matérielle](#architecture)
4. [Logique de Fonctionnement et Algorithme](#fonctionnement)
5. [Intégration IoT et Dashboard](#iot)
6. [Liste des Composants Techniques](#composants)
7. [Conclusion](#conclusion)

---

<a name="introduction"></a>
## 1. Introduction
Les accidents de la route constituent une urgence de santé publique majeure. La survie des victimes dépend souvent de la rapidité de l'intervention médicale, concept connu sous le nom d'« Heure d'Or ».  
Ce projet propose une solution de **télématique embarquée IoT** qui dépasse la simple détection de choc. Notre système est conçu non seulement pour détecter les accidents graves, mais aussi pour analyser l'état vital du conducteur (rythme cardiaque, oxygénation) et transmettre ces données critiques en temps réel aux secours via une architecture hybride (GSM + Cloud).

<a name="objectifs"></a>
## 2. Objectifs du Projet
Le projet a évolué pour répondre aux exigences des systèmes de transport intelligents (ITS) modernes :

1.  **Détection Intelligente :** Utiliser la fusion de capteurs (Accéléromètre + Gyroscope) pour distinguer un vrai accident (choc, tonneau) d'un simple freinage brusque.
2.  **Suivi de Santé (Nouveau) :** Intégrer un capteur biométrique pour transmettre l'état de santé du conducteur (BPM et SpO2) post-accident, permettant aux secours de préparer l'intervention médicale adaptée.
3.  **Redondance de Communication :** Assurer l'envoi de l'alerte même en zone blanche (sans internet) grâce au réseau GSM (SMS), tout en privilégiant le Cloud (Wi-Fi/GPRS) pour le suivi en temps réel.
4.  **Fiabilité (Anti-Fausse Alerte) :** Intégrer une interface homme-machine (Buzzer + Bouton) permettant au conducteur d'annuler une fausse alerte avant l'envoi des secours.

<a name="architecture"></a>
## 3. Nouvelle Architecture Matérielle
Le système est articulé autour d'un microcontrôleur **ESP32**, choisi pour sa double connectivité et sa puissance de calcul.

### A. Module de Traitement (Le Cerveau)
* **Composant :** ESP32 Development Board.
* **Rôle :** Collecte les données des capteurs via le bus I2C et UART, exécute l'algorithme de décision, et gère les piles de protocole TCP/IP (IoT) et GSM.

### B. Module de Détection Cinétique
* **Composant :** MPU-6050 (Accéléromètre 3 axes + Gyroscope).
* **Fonction :** Mesure les forces G (impact) et la vitesse angulaire (renversement du véhicule).

### C. Module de Santé (Biométrie)
* **Composant :** MAX30102 (Oxymètre de Pouls).
* **Fonction :** Mesure la saturation en oxygène (SpO2) et la fréquence cardiaque. Il permet de détecter si le conducteur est en état de choc ou en arrêt cardiaque après l'impact.

### D. Module de Localisation
* **Composant :** NEO-6M GPS.
* **Fonction :** Fournit la latitude, la longitude, l'heure précise et la vitesse du véhicule.

### E. Module de Communication Hybride
* **Primaire (IoT) :** Wi-Fi intégré de l'ESP32 pour l'envoi de données riches (JSON) vers le Cloud.
* **Secondaire (Secours) :** Module GSM SIM800L pour l'envoi de SMS d'urgence en cas d'absence de couverture internet.

### F. Alimentation (Critique)
* **Composant :** Convertisseur Buck LM2596.
* **Rôle :** Abaisse la tension batterie (12V) vers une tension stable de 4.2V/5V nécessaire pour alimenter le module GSM qui consomme des pics de courant élevés (2A).

<a name="fonctionnement"></a>
## 4. Logique de Fonctionnement et Algorithme

Le système fonctionne selon une machine à états finis :

1.  **État de Veille (Monitoring) :** L'ESP32 lit le MPU-6050 en boucle.
2.  **État de Détection :** Si l'accélération dépasse le seuil critique (ex: > 4G) OU si l'inclinaison dépasse 60° (Tonneau) :
    * Le système passe en mode "Pré-Alerte".
    * L'alarme sonore (Buzzer) s'active.
    * Un minuteur de 10 secondes démarre.
3.  **État de Validation :**
    * **Si le conducteur appuie sur le bouton "Annuler" :** Le système retourne en veille (Fausse alerte).
    * **Si le minuteur expire :** L'accident est confirmé.
4.  **État d'Acquisition :** Le système lit les coordonnées GPS et active le capteur MAX30102 pour prendre les constantes vitales pendant 5 secondes.
5.  **État de Transmission :**
    * **Envoi SMS :** "SOS! Accident détecté. Lat: X, Lon: Y. Pouls: 90 BPM. SpO2: 98%."
    * **Envoi IoT :** Mise à jour du Dashboard en temps réel.

<a name="iot"></a>
## 5. Intégration IoT et Dashboard
Pour répondre aux exigences de modernisation, le système est connecté à une plateforme Cloud (Blynk / Firebase).

* **Visualisation Cartographique :** Affichage de la position exacte du véhicule sur une carte Google Maps intégrée.
* **Moniteur de Santé :** Jauges affichant en temps réel le pouls et l'oxygène du conducteur.
* **Historique :** Enregistrement des données de vitesse et d'accélération avant l'impact (fonction "Boîte Noire").

<a name="composants"></a>
## 6. Liste des Composants (BOM)

| Composant | Rôle Technique | Remarque |
| :--- | :--- | :--- |
| **ESP32 Dev Module** | Contrôleur Principal + Wi-Fi | Remplace l'Arduino pour l'IoT. |
| **MPU-6050** | Capteur de Choc | Protocole I2C. |
| **NEO-6M** | GPS | Protocole UART. Antenne requise. |
| **SIM800L** | GSM (SMS/Appel) | Nécessite une alim. dédiée. |
| **MAX30102** | Capteur Cardiaque / SpO2 | Protocole I2C. |
| **LM2596** | Régulateur de Tension | Convertit 12V -> 4V (2A). |
| **Bouton Poussoir** | Annulation / SOS Manuel | Interface Utilisateur. |
| **Buzzer Actif** | Alarme Sonore | Avertissement local. |

<a name="conclusion"></a>
## 7. Conclusion
Cette mise à niveau du projet transforme un simple détecteur de choc en un **système de télésurveillance médicale et routière**. En intégrant l'IoT (ESP32), la biométrie (MAX30102) et une logique de validation robuste, nous répondons aux limitations de connectivité et de fiabilité des systèmes actuels. Ce prototype représente une solution complète, à faible coût et hautement évolutive pour la sécurité routière moderne.