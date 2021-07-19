# Burg'war [![France flag](.github/images/franceflag.png)](README_fr.md)

![Le burger iconique de Burgwar](.github/images/burger.png)

Burg'war est un jeu de plateforme multijoueur 2D orienté sur la thématique de la nourriture, où vous incarnez un hamburger se battant contre d'autres hamburgers à l'aide d'armes comme l'Emmentalibur, la Tomace, le lance-patate, et bien d'autres !

Mais en réalité derrière ce jeu en apparence simple se cache un jeu de sandbox (de la même façon que Garry's Mod), toute la partie gameplay du jeu est programmée en Lua et est ouverte à modification. 
Vous avez donc toute la liberté de modifier le jeu, rajouter vos armes, votre façon de jouer, vos assets et lancer un serveur ! Le jeu s'occupe du reste pour que les joueurs rejoignant votre serveur jouent selon les règles de votre créativité !

## Comment jouer ?

Téléchargez la dernière [release](https://github.com/DigitalPulseSoftware/BurgWar/releases) suffit pour jouer, vous avez également la possibilité de télécharger l'éditeur de map pour faire vos propres maps.

Vous pouvez également tester la dernière version de développement en suivant les instructions [sur le wiki](https://github.com/DigitalPulseSoftware/BurgWar/wiki/%5BFR%5D-T%C3%A9l%C3%A9charger-les-nightlies-pour-jouer-au-jeu-hors-releases).

## Comment compiler le projet ?

Burg'War se compile en utilisant [XMake](https://xmake.io), qui se chargera pour vous de télécharger / compiler toutes les dépendances manquantes, à l'exception de [Qt](https://www.qt.io) que vous devrez installer vous-même si vous souhaitez compiler l'éditeur de map.

**Attention**, en raison d'une limitation de XMake, la compilation échouera si Qt n'est pas présent sur votre machine, et ce même si vous ne compilez pas l'éditeur de map.  
Comme correctif temporaire, vous pouvez supprimer toutes les lignes du fichier **xmake.lua** à partir de `target("BurgWarMapEditor")` si vous n'avez pas Qt et désirez vous en passer.  
Dans une version ultérieure de XMake, ceci ne sera plus nécessaire.

**Si vous êtes sous Linux**, la version de Nazara utilisée par Burg'War ne fonctionnant pas avec XMake, certaines dépendances systèmes doivent être installées sous peine de voir la compilation du moteur échouer:
* Avec apt: `sudo apt-get install libopenal-dev libsndfile1-dev libfreetype6-dev libsdl2-dev libxcb-cursor-dev libxcb-ewmh-dev libxcb-keysyms1-dev libx11-dev mesa-common-dev libgl1-mesa-dev libassimp-dev`
* Avec pacman: `sudo pacman -S openal libsndfile freetype2 libxcb libx11 sdl2 mesa assimp`
* Avec portage: `sudo emerge media-libs/assimp media-libs/mesa media-libs/libsdl2 x11-libs/libX11 x11-libs/libxcb media-libs/freetype media-libs/libsndfile media-libs/openal`

Une fois que Burg'War sera passé à la dernière version de Nazara (utilisant XMake), cela ne sera plus nécessaire.

Une fois XMake téléchargé et installé (note: vous pouvez aussi obtenir [une version portable](https://github.com/xmake-io/xmake/releases) si vous ne souhaitez pas l'installer), il vous suffit ensuite d'exécuter la commande `xmake config --mode=releasedbg` dans le répertoire du projet (vous pouvez également préciser `--mode=debug` si vous désirez obtenir une build de débuggage).

XMake va ensuite essayer de trouver les dépendances du projet sur votre machine et vous proposer d'installer celles qu'il ne peut pas trouver (à l'exception de Qt).

### Compilation en ligne de commande (Méthode 1)

Une fois que vous êtes prêts à compiler le jeu lui-même, exécutez la commande `xmake` (ou `xmake -jX` si vous souhaitez réduire le nombre de threads utilisés, avec X pour le nombre de threads désiré) et regardez le jeu se compiler sous vos yeux ébahis.

### Génération de projet (Méthode 2)

XMake est également capable de générer un fichier projet pour un autre outil :
- Visual Studio: `xmake project -k vsxmake`
- CMakeLists.txt (ouvrable dans CLion et autre): `xmake project -k cmake`
- Makefile: `xmake project -k make`
- Ninja: `xmake project -k ninja`
- XCode: `xmake project -k xcode`

Vous pouvez ensuite compiler le jeu avec l'outil de votre choix.

### Lancer le jeu

Une fois la compilation terminée, vous devriez obtenir les binaires du jeu dans le dossier `bin/<config>` (où `<config>` équivaudra à votre plateforme/architecture/configuration, ex: `windows_x64_debug`).

Copiez les fichiers `clientconfig.lua`, `editconfig.lua` et `serverconfig.lua` à côté des exécutables de Burg'War. Vous nécessiterez également les assets (trouvables dans les [releases](https://github.com/DigitalPulseSoftware/BurgWar/releases) ou sur [sur le wiki](https://github.com/DigitalPulseSoftware/BurgWar/wiki/%5BFR%5D-T%C3%A9l%C3%A9charger-les-nightlies-pour-jouer-au-jeu-hors-releases)) pour exécuter correctement le jeu.

Vous pouvez maintenant lancer le jeu via XMake, avec la commande `xmake run <target>` (remplacez `<target>` par le nom de l'exécutable, par exemple `xmake run BurgWar` pour lancer le jeu).

**Note: vous devez passer par `xmake run` à cause des dépendances qui ne seraient pas trouvées autrement, si vous désirez pouvoir lancer les exécutables directement utilisez la commande `xmake install -o installed` pour que XMake copie tous les fichiers nécessaires dans le dossier `installed/bin`.**

## Quelles sont les technologies utilisées par Burg'War ?

Les technologies utilisées sont :
- [concurrentqueue](https://github.com/cameron314/concurrentqueue) : file d'attente lock-free utilisée pour transmettre et recevoir des messages avec les thread réseau
- [cURL](https://curl.haxx.se/) : utilisée par le client pour télécharger les assets depuis un serveur HTTP(S)
- [cxxopts](https://github.com/jarro2783/cxxopts) : pour parser la ligne de commande (pour l'outil maptool)
- [fmt](https://github.com/fmtlib/fmt) : bibliothèque de formattage incontournable en C++, utilisée pour le système de logs
- [hopscotch-map](https://github.com/Tessil/hopscotch-map) : une hashmap performante basée sur le [hopscotch hashing](https://en.wikipedia.org/wiki/Hopscotch_hashing)
- [Nazara Engine](https://github.com/DigitalPulseSoftware/NazaraEngine) : **mon propre moteur de jeu**, responsable du rendu, de l'audio, de la physique, etc.
- [nlohmann_json](https://json.nlohmann.me) : parseur de JSon simple d'utilisation
- [Qt](https://www.qt.io) : bibliothèque très connue permettant de faire des interfaces graphiques facilement en C++
- [sol](https://github.com/ThePhD/sol2) : interface C++ <=> Lua très pratique et rapide
- [tl_expected](https://github.com/TartanLlama/expected) : implémentation d'une classe Result-like pour le C++
- [tl_function_ref](https://github.com/TartanLlama/function_ref) : implémentation d'une référence vers une lambda ou fonction (plus efficace que std::function)
- [XMake](https://xmake.io) : système de build et de gestion de dépendances qui compile le projet et télécharge les dépendances requises, ce qui fait que vous n'avez pas à vous soucier de cette liste pour compiler le projet !
