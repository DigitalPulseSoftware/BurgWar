    Class WEAPON
        float  Scale          -- Defini la taille de l'arme relativement à la taille de l'image qui lui est associé.
        str    Sprite         -- Defini le chemin vers l'image de l'arme.
        Vec2   SpriteOrigin   -- Defini l'origine du sprite, cest a dire le centre de rotation de l'arme.
        float  Cooldown       -- Defini l'intervale minimum entre deux utilisations de l'arme.
        Vec2   WeaponOffset   -- ??
        Entity Entity        -- Entité nazara de l'arme
        
        ??     PlaySound      -- Joue un son, disponible uniquement du cotè client.
                   str : fichier a jouer,
                   bool: ??,
                   bool: ??,
                   bool: ??
        ??     Shoot          -- Lance un projectile
                   Vec2: Origine du tir
                   Vec2: Direction du tir
                   int : Degats?
                   int : Vitesse?
        Vec2   GetDirection   -- Permet de recupérer la direction de l'arme, sous forme de vecteur directeur.
        Vec2   GetPosition    -- Recupère la position actuelle du personnage
        bool   IsLookingRight -- Renvoie vrai si le personnage est orienté à droite.
        bool   IsLookingLeft  -- Renvoie vrai si le personnage regarde à gauche
        ??     PlayAnim       -- Joue une animation
                   str : Nom de l'animation a jouer
        ??    DealDamage     -- Inflige des degats dans une zone.
                   int : Quantité
                   Rect: Zone où infliger les degats
                   int : ??
