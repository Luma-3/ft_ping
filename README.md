# 🌐 ft_ping

> Une implémentation personnalisée de l'utilitaire ping en C, utilisant des raw sockets ICMP pour diagnostiquer la connectivité réseau.

[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
![GitHub repo size](https://img.shields.io/github/repo-size/Luma-3/ft_ping)


## 📋 Description

**ft_ping** est une réimplémentation de l'outil classique `ping` qui envoie des paquets ICMP ECHO_REQUEST aux hôtes réseau et mesure le temps de réponse. Ce projet démontre une compréhension approfondie des protocoles réseau bas niveau, de la programmation système et de la manipulation de raw sockets.

### ✨ Fonctionnalités

- 📡 Envoi et réception de paquets ICMP ECHO_REQUEST/REPLY
- 🎯 Résolution DNS et support des adresses IP
- ⏱️ Mesure précise du RTT (Round-Trip Time)
- 📊 Statistiques détaillées (min/max/avg/stddev)
- 🔍 Détection des paquets dupliqués
- 🚦 Gestion des erreurs ICMP (Destination Unreachable, Time Exceeded, Redirect)
- ⚙️ Options configurables (TTL, timeout, intervalle, nombre de paquets)
- 📝 Mode verbose avec dump des paquets
- 🐳 Support Docker pour les tests en environnement contrôlé

## 🚀 Installation

### Prérequis

- GCC compiler
- Make
- Privilèges root (requis pour les raw sockets)

### Compilation

```bash
# Cloner le repository
git clone https://github.com/Luma-3/ft_ping.git
cd ft_ping

# Compiler le projet
make

# Le binaire ft_ping est créé dans le répertoire courant
```

## 📖 Utilisation

```bash
# Utilisation basique (nécessite root)
sudo ./ft_ping google.com

# Avec options
sudo ./ft_ping -c 5 -i 1 -v google.com
```

### Options disponibles

| Option | Argument | Description |
|--------|----------|-------------|
| `-v`, `--verbose` | - | Affichage détaillé avec dump des paquets |
| `-?`, `--help` | - | Affiche l'aide et quitte |
| `-i`, `--interval` | N | Attend N secondes entre chaque paquet |
| `-c`, `--count` | N | S'arrête après N paquets envoyés |
| `-w`, `--timeout` | N | S'arrête après N secondes |
| `-s`, `--linger` | N | Définit la taille du payload des paquets |
| `-t`, `--ttl` | N | Définit le Time-To-Live à N |

### Exemples

```bash
# Envoyer 10 paquets avec un intervalle de 2 secondes
sudo ./ft_ping -c 10 -i 2 8.8.8.8

# Ping avec TTL personnalisé
sudo ./ft_ping -t 64 example.com

# Mode verbose avec timeout
sudo ./ft_ping -v -w 30 github.com
```

## 🏗️ Architecture

Le projet est structuré comme suit :

```
ft_ping/
├── src/
│   ├── main.c          # Point d'entrée et boucle principale
│   ├── parsing.c       # Analyse des arguments
│   ├── packet.c        # Gestion des paquets ICMP/IP
│   ├── ionet.c         # Envoi/réception réseau
│   └── utils.c         # Utilitaires et affichage
├── inc/
│   ├── ping.h          # Structures principales
│   ├── packet.h        # Structures de paquets
│   └── ionet.h         # Interface réseau
├── Makefile            # Build system
├── Dockerfile          # Conteneur Docker
└── docker-compose.yml  # Orchestration Docker
```

### Composants clés

- **Raw Sockets ICMP** : Utilisation de `SOCK_RAW` avec `IPPROTO_ICMP`
- **Calcul de checksum** : Implémentation de l'algorithme de checksum ICMP
- **Statistiques en temps réel** : Calcul de la moyenne, min, max et écart-type
- **Gestion des signaux** : Capture de SIGINT pour un arrêt propre

## 🐳 Tests avec Docker

Le projet inclut un environnement Docker pour tester le comportement réseau :

```bash
# Construire et lancer l'environnement
make docker

# Entrer dans le conteneur
docker exec -it ft_ping-container /bin/bas

# Tester avec simulation de latence réseau
# 1st param % loss, 2nd param % corrupt, 3rd param % duplicated
./apply_netem.sh 
ft_ping google.com
```

## 📊 Exemple de sortie

```
PING google.com (142.250.185.46) 56 data bytes
64 bytes from 142.250.185.46: icmp_seq=0 ttl=117 time=14.235 ms
64 bytes from 142.250.185.46: icmp_seq=1 ttl=117 time=13.892 ms
64 bytes from 142.250.185.46: icmp_seq=2 ttl=117 time=14.156 ms
^C
--- 142.250.185.46 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max/stddev = 13.892/14.094/14.235/0.143 ms
```

## 🛠️ Détails techniques

- **Langage** : C
- **Raw Sockets** : Communication ICMP bas niveau
- **Protocoles** : IPv4, ICMP
- **Normes** : [RFC 792 (ICMP)](https://www.rfc-editor.org/rfc/rfc792.html), [RFC 791 (IP)](https://www.rfc-editor.org/rfc/rfc791.html)
- **Compilation** : GCC avec flags `-Wall -Wextra -Werror`

## 📝 Notes importantes

⚠️ **Ce programme nécessite les privilèges root** pour créer des raw sockets. Utilisez-le de manière responsable.

## 👤 Auteur

**Luma-3**

- GitHub: [@Luma-3](https://github.com/Luma-3)
- Repository: [ft_ping](https://github.com/Luma-3/ft_ping)
- Page Projet: [portfolio](https://portfolio.luma3.dev/projects?project=ping)

---

*Développé dans le cadre d'un projet de programmation système et réseau* 🚀
