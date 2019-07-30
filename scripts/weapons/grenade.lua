RegisterClientScript("grenade.lua")

WEAPON.Cooldown = 1
WEAPON.Scale = 0.2
WEAPON.Sprite = "grenade.png"
WEAPON.SpriteOrigin = Vec2(60, 256) * WEAPON.Scale
WEAPON.WeaponOffset = Vec2(0, -40) -- This should not be here

RegisterClientAssets(WEAPON.Sprite)

if (SERVER) then
	function WEAPON:OnAttack()
		local projectile = GM:CreateEntity("entity_grenade", self:GetPosition() + self:GetDirection() * 32, {
			lifetime = math.random(1, 2)
		})

		projectile:SetVelocity(self:GetDirection() * 1000)
		self:GetOwner():RemoveWeapon(self.FullName)
	end
end
