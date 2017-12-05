# Pendu2
A simple client/server pendu application written in C.

## Résumé

Je vous présente mon petit programme de modèle client-serveur avec le jeu du pendu. Il est fait
avec des sockets et des threads en langage C, dans lequel j'ai implémenté un serveur qui peut gérer
un nombre infini de client, chaque client est identifié par un pseudo, le numéro de sa socket et un
numéro de client gérée par le serveur.
Comment se passe la communication entre le serveur et le(s) clients lors d’une partie ?

![screenshot3](https://user-images.githubusercontent.com/26382145/33632807-35ee714e-d9e5-11e7-8308-dee4b2de8ae8.jpg)

Il est possible que plus d’un joueur communique avec le serveur en même temps.
Le serveur choisis aléatoirement un mot dans un dictionnaire lu dans un fichier.
Le serveur pourrait permettre la configuration d’une partie (nombre de coups d’essais, longueur
minimal ou maximale du mot à trouver, etc.).

## Utilisation

Il faut lancer le serveur puis le ou les clients.

      >./serveur service

Le service est le numéro de port à utiliser. Le paramètre est facultatif, la valeur par défaut est 1111.
Figure 1– Exemple communication client-serveur pour 1 connexion lors
d'une partie pour un mot entre le client et le serveur

      >./client serveur service

Le serveur est l’adresse IP du serveur, service est le numéro de port à utiliser. Ces paramètres sont
facultatifs, les valeurs par défaut sont, respectivement, localhost et 33016.
