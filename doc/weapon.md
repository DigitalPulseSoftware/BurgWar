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

Il y a aussi des propriétés non obligaroire, mais très utile:

- `WEAPON.Scale = 0.2`, qui permet de redimentionner l'arme.
- `WEAPON.Cooldown = 0.05`, qui defini l'intervale minimum entre chaque utilisation de l'arme.
- `WEAPON.WeaponOffset = Vec2(0, -40)`, qui visiblement ne devrais pas être ici.

Pour l'instant l'arme ne fait rien. Elle a la bonne taille, la bonne limite d'utilisation, mais n'effectue aucune action.

Pour que elle fonctionne il faut implementer la methode `WEAPON:OnAttack`:

    function WEAPON:OnAttack()
        if (CLIENT) then
            self:PlaySound(self.ShootSound[math.random(1, 3)], true, false, true)
        end

        local shootDir = self:GetDirection()
        local shootPos = self:GetPosition() + shootDir * 740 * self.Scale

        self:Shoot(shootPos, shootDir, 20, 10000)
    end

Decomposons cette fonction ensemble.

La methode commence par un test: `if (CLIENT) then`. Ce test permet de verifier que le script est exécuté du coté du client (il existe de même la variable `SERVER` qui est à `true` sur le serveur). Ici ce test est necessaire car on souhaite jouer un son, ce qui n'est possible que du coté client.
Le mot clef `self` correspond à l'arme actuelle, donc on peut utiliser des methodes spéciales, ici `srlf:PlaySound`. 

Ensuite on souhaite que l'arme tire un projectile, pour cela on utilise la methode `self:Shoot`, qui prend 4 paramétres, un point de depart du projectile, sa direction, les degats qu'il inflige et sa vitesse.
