RegisterClientScript()

local scale = 0.2

local weapon = ScriptedWeapon({
	Cooldown = 1,
	Scale = scale,
	Sprite = "grenade.png",
	SpriteOrigin = Vec2(60, 256) * scale,
	WeaponOffset = Vec2(20, -40) -- This should not be here
})

RegisterClientAssets(weapon.Sprite)

if (SERVER) then
	weapon:On("attack", function (self)
		local scale = self:GetScale()

		local projectile = match.CreateEntity({
			Type = "entity_grenade",
			LayerIndex = self:GetLayerIndex(),
			Owner = self:GetOwner(),
			Position = self:GetPosition() + self:GetDirection() * 32 * scale,
			--Scale = scale, -- TODO: Handle scale when creating entity
			Properties = {
				lifetime = math.random(1, 2),
			}
		})
		projectile:SetScale(scale)

		projectile:SetVelocity(self:GetDirection() * scale * 1000)
		self:GetOwnerEntity():RemoveWeapon(self.FullName)
	end)
end
