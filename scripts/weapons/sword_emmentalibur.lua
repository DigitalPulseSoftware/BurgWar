WEAPON.Scale = 0.714
WEAPON.Sprite = "../resources/emmentalibur.png"
WEAPON.SpriteOrigin = Vec2(40, 284) / WEAPON.Scale
WEAPON.WeaponOffset = Vec2(60, 100) -- This should not be here
WEAPON.Cooldown = 0.3

function WEAPON:OnAttack()
	print("Hi", tostring(self))
	self:DealDamage(100, 100)
end
