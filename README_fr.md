# Burg'war [![France flag](.github/images/franceflag.png)](README_fr.md)

![Le burger iconique de Burgwar](.github/images/burger.png)

Burg'war est un jeu de plateforme multijoueur 2D orienté sur la thématique de la nourriture, où vous incarnez un hamburger se battant contre d'autres hamburgers à l'aide d'armes comme l'Emmentalibur, la Tomace, le lance-patate, et bien d'autres !

Mais en réalité derrière ce jeu en apparence simple se cache un jeu de sandbox (de la même façon que Garry's Mod), toute la partie gameplay du jeu est programmée en Lua et est ouverte à modification. 
Vous avez donc toute la liberté de modifier le jeu, rajouter vos armes, votre façon de jouer, vos assets et lancer un serveur ! Le jeu s'occupe du reste pour que les joueurs rejoignant votre serveur jouent selon les règles de votre créativité !

## Comment jouer ?

Téléchargez la dernière [release](https://github.com/DigitalPulseSoftware/BurgWar/releases) suffit pour jouer, vous avez également la possibilité de télécharger l'éditeur de map pour faire vos propres maps.

## Comment compiler le projet ?

Burg'war utilise trois bibliothèques :

- [NazaraEngine](https://github.com/DigitalPulseSoftware/NazaraEngine) - utilisé par l'entièreté du projet (téléchargez les nightlies ou compilez vous-même) ;
- [cURL](https://curl.haxx.se/) - requis par le client ([binaires pour Windows](https://curl.haxx.se/windows/)) ;
- [Qt](https://www.qt.io) - requis par l'éditeur de map.

Une fois les binaires de ces bibliothèques installées sur votre système, copiez-le fichier `build/config.default.lua` vers `build/config.lua` et remplissez-le pour indiquer où se trouvent vos dépendances.

Ce fichier est utilisé par le générateur ([Premake](https://premake.github.io)) pour savoir où se trouvent les binaires des bibliothèques (bin) et les headers pour l'inclusion (include).
Sous Windows, il faut également renseigner l'endroit où se trouve les .lib d'inclusion (lib).

Le format est le suivant :
```lua
NomDependance = {
  -- Vous pouvez spécifiez chaque dossier séparément
	BinPath = [[C:\Qt\5.12.6\msvc2017_64\bin]],
	IncludePath = [[C:\Qt\5.12.6\msvc2017_64\include]],
	LibPath = [[C:\Qt\5.12.6\msvc2017_64\lib]],
  
  -- Ou si, comme dans l'exemple ci-dessus, vous avez un dossiez contenant des sous-dossiers bin/include(/lib), vous pouvez utiliser le raccourci suivant :
  PackageFolder = [[C:\Qt\5.12.6\msvc2017_64]],
}
```

Si la dépendance est installée sur votre système (comme souvent sur Linux), vous pouvez l'indiquer avec :
```lua
NomDependance = {
  PackageFolder = ":system",
}
```

Les trois dépendances sont `cURL`, `Nazara` et `Qt`. 
À noter que `Qt` demande aussi l'option `MocPath`, chemin vers l'utilitaire moc.exe (normalement trouvé dans le dossier bin), ce n'est pas obligatoire mais recommandé si vous souhaitez effectuer des changements au projet.

Dans le cas où une dépendance ne serait pas renseignée, les projets qui en dépendent seront ignorés.

Une fois ce fichier rempli, vous n'avez qu'à exécuter `premake5.exe` (`premake5-linux64` sous Linux) suivi d'un espacement et de la cible que vous souhaitez générer, communément `vs2019` pour Visual Studio 2019, `gmake` pour un makefile, etc. (Appelez premake sans argument pour obtenir la liste des cibles supportées).

À partir de là, compilez le projet selon la cible générée, et vous devriez obtenir des exécutables pour le Client (Burgwar), le Serveur (BWServer) et l'Éditeur de map (BWMapEditor).
Chacun de ces projets utilise un fichier .lua pour sa configuration, veillez bien à ce qu'il soit présent (et bien configuré) dans le répertoire courant de l'exécutable avant de le démarrer.

Il ne vous manquera que les assets (images/sons/etc.) nécessaires, que vous pouvez trouver en téléchargeant les releases GitHub.
