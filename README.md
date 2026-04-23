Mini GPS différentiel avec un ESP8266, un LC29HEA et une antenne SMA.

Petit projet personnel pour explorer la géolocalisation de précision sur matériel low-cost.
L'idée de départ était simple : est-ce qu'on peut atteindre une précision centimétrique avec un microcontrôleur ?

Le problème : 
Je voulais construire une tondeuse autonome avec des composants à bas coûts. Pour qu'elle se déplace de manière efficace, il faut qu'elle se repère dans l'espace. Un GPS classique est précis à 3–10 mètres. C'est suffisant pour naviguer en voiture, 
mais inutilisable pour mon usage.
La technique RTK (Real-Time Kinematic) corrige ces erreurs en temps réel grâce à une station de référence fixe dont la position est connue. La différence entre ce qu'elle devrait recevoir et ce qu'elle reçoit vraiment donne les corrections à 
appliquer au récepteur mobile.
En France, le réseau Centipède RTK met ces corrections à disposition gratuitement via internet.

Ce que fait ce projet : 
Un ESP8266 joue deux rôles simultanément :
  - Il télécharge les corrections depuis Centipède et les envoie au récepteur LC29HEA
  - Il lit les coordonnées corrigées renvoyées par le LC29HEA et les affiche sur une page web locale

Schéma de la situation : 
Internet (Centipède)
    ↓ corrections RTCM (fomat binaire des corrections)
  ESP8266
    ↕ UART (type de connexion RX/TX)
  LC29HEA ← antenne SMA
    → coordonnées corrigées
    → page web sur 192.168.4.1
Résultat : une précision de 1 à 2 cm en mode RTK FIX, contre 3 à 10 m sans correction.

Matériel :
Composant <-> Rôle
ESP8266 (NodeMCU) <-> WiFi + serveur web
LC29HEA breakout <-> Récepteur GPS RTK
Antenne SMA active <-> Réception satellite
Smartphone <-> Hotspot internet

Ce que j'ai appris : 
- Le parsing NMEA manuellement (sans bibliothèque) pour garder l'empreinte mémoire faible (texte provenant du GPS pour transmettre les coordonnées)
- Le protocole NTRIP, qui transporte les corrections RTK sur TCP (protocole pour récupérer le RTCM sur internet)
- La gestion du mode WIFI_AP_STA sur ESP8266 : se connecter à un réseau et en créer un autre simultanément
- Les contraintes matérielles du 3.3V et la gestion de l'alimentation sur microcontrôleur

Limites :
- Nécessite une connexion internet active
- Sensible à la qualité du ciel (nuages, obstacles)
- Pas de stockage des positions, tout est temps réel

Pistes pour aller plus loin :
- Ajouter un GPS basique en parallèle pour visualiser concrètement l'écart brut / corrigé
- Enregistrer les traces sur carte SD
- Afficher la position sur une carte OpenStreetMap embarquée
- utiliser les données RTK pour gérer le déplacement de la tondeuse.

Réseau de correction utilisé : Centipède RTK — open-source, communautaire.
