RegisterClientScript()
RegisterClientAssets("placeholder/physgun.png")

local entity = ScriptedEntity({
	IsNetworked = true
})

entity:On("init", function (self)
	self:SetCollider({ Collider = Circle(Vec2(0, 0), 128 * 0.3), IsTrigger = true })
	self:EnableCollisionCallbacks(true)

	if (CLIENT) then
		self:AddSprite({
			TexturePath = "placeholder/physgun.png"
		})
	end
end)

entity:On("collisionstart", function (self, other)
	if (SERVER and other.Name == "burger") then
		local owner = other:GetOwner()
		if (not owner:HasWeapon("weapon_physics_bun")) then
			owner:GiveWeapon("weapon_physics_bun")
			self:Kill()
			self.Parent:OnPowerupConsumed()
		end
	end

	return false
end)

