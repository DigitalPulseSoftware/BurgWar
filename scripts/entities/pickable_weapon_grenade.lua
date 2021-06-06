RegisterClientScript()
RegisterClientAssets("grenade.png")

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
			Scale = Vec2(0.25, 0.25),
			TexturePath = "grenade.png"
		})
	end
end)

entity:On("collisionstart", function (self, other)
	if (SERVER and other.IsPlayerEntity) then
		if (not other:HasWeapon("weapon_grenade")) then
			other:GiveWeapon("weapon_grenade")
			self:Kill()
		end
	end

	return false
end)
