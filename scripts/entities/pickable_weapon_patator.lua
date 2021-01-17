RegisterClientScript()
RegisterClientAssets("placeholder/patator.png")

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
			Scale = Vec2(0.33, 0.33),
			TexturePath = "placeholder/patator.png"
		})
	end
end)

entity:On("collisionstart", function (self, other)
	if (SERVER and other.IsPlayerEntity) then
		if (not other:HasWeapon("weapon_patator")) then
			other:GiveWeapon("weapon_patator")
			self:Kill()
			self.Parent:OnPowerupConsumed()
		end
	end

	return false
end)
