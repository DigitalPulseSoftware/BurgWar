RegisterClientScript()
RegisterClientAssets("rifle-30349_640.png")

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
			Scale = Vec2(0.3, 0.3),
			TexturePath = "rifle-30349_640.png"
		})
	end
end)

entity:On("collisionstart", function (self, other)
	if (SERVER and other.IsPlayerEntity) then
		if (not other:HasWeapon("weapon_rifle")) then
			other:GiveWeapon("weapon_rifle")
			self:Kill()
			self.Parent:OnPowerupConsumed()
		end
	end

	return false
end)
