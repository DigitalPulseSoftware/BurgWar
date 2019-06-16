# Weapon

On va étudier la mitraiellette pour comprendre comment fonctionne le srcipting d'arme.

Une arme est un script comme les autres, il se situe dans le dossier `script/weapons`.
Comme tout script il faut commencer par l'enregistrer:

    RegisterClientScript("rifle.lua")

Une fois le script enregistré on a a notre disposition la classe WEAPON, qui va nous permettre de créer une arme.
On est limité à une arme par script, donc si vous voulez en faire plusieurs il faut faire différent scripts.

Pour pouvoir être utilisé cette arme doit poséder au moins deux attributs:
    
    WEAPON.Sprite = "rifle-30349_640.png"
    WEAPON.SpriteOrigin = Vec2(-100, 105) * WEAPON.Scale

La première propriété, `WEAPON.Sprite` permet de dire au jeu d'afficher le fichier `rifle-30349_640.png`.
La seconde, `WEAPON.SpriteOrigin` permet de definir le centre de rotation de l'arme, par rapport au centre du personnage.

Il y a aussi des propriétés non obligaroire, mais très utile. La première que l'on voit dans le script `rifle.lua` est `WEAPON.Scale`.
Celle-ci permet de redimentionner l'entité qui représente l'arme.


