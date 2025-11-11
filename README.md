# **<span style="color: #197de1;">Car accident detection and emergency alert</span>**


### Détecter automatiquement les accidents de la route (à l'aide de capteurs), collecter les données GPS et celles du conducteur, et envoyer une alerte d'urgence (SMS/HTTP/appel) avec les coordonnées et les données pertinentes aux contacts/services d'urgence.
## Ce projet est divise à 6 phases qui sont: 
> ## *1. [Objectifs prinicpaux](#1)*
> ## *2. [Les entrées et les sorties](#2)*
> ## *3. [Blocs fonctionnels principaux](#3)*
> ## *4. [Matériaux](#4)*
> ## *5. [Partie Software](#())*

<a id="1">Les entrées et les sorties</a>

####Entrées :
    Accéléromètre (3 axes, plage de sensibilité élevée) — détection des décélérations/impacts brusques

    Gyroscope — détection de rotation

    GPS (latitude, longitude, horodatage)

    Signal d’alimentation/d’allumage du véhicule (optionnel)

    Capteur de ceinture de sécurité ou données CAN-bus (optionnel)

    Données du conducteur (nom, téléphone, informations médicales) — saisies une seule fois dans l’application ou sur l’appareil

    Bouton de confirmation utilisateur (annulation des fausses alertes)

####Sorties :
    Message d’alerte d’urgence (SMS, requête HTTP POST ou appel vocal) avec détails

    Alarme locale (avertisseur sonore/LED) et notification téléphonique

    Journal des événements (heure, données des capteurs, GPS) stocké localement/à distance

    Optionnel : appel automatique au numéro d’urgence et/ou transmission des données au serveur/tableau de bord


<a id="2">Blocs fonctionnels principaux</a>

<a id="3">Objectifs prinicpaux</a>

<a id="4">Objectifs prinicpaux</a>

<a id="5">Objectifs prinicpaux</a>

