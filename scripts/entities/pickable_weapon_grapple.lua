RegisterClientScript()
RegisterClientAssets("placeholder/baguette.png")

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
			TexturePath = "placeholder/baguette.png"
		})
	end
end)

entity:On("collisionstart", function (self, other)
	if (SERVER and other.IsPlayerEntity) then
		if (not other:HasWeapon("weapon_graspain")) then
			other:GiveWeapon("weapon_graspain")
			self:Kill()
		end
	end

	return false
end)
