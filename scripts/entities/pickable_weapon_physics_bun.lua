RegisterClientScript()
RegisterClientAssets("placeholder/physgun.png")

local entity = ScriptedEntity({
	IsNetworked = true
})

entity:On("init", function (self)
	self:SetColliders({ 
		Collider = Circle(Vec2(0, 0), 128 * 0.3), 
		ColliderType = ColliderType.Callback,
		IsTrigger = true
	})

	if (CLIENT) then
		self:AddSprite({
			TexturePath = "placeholder/physgun.png"
		})
	end
end)

entity:On("collisionstart", function (self, other)
	if (SERVER and other.IsPlayerEntity) then
		if (not other:HasWeapon("weapon_physics_bun")) then
			other:GiveWeapon("weapon_physics_bun")
			self:Kill()
			self.Parent:OnPowerupConsumed()
		end
	end

	return false
end)
