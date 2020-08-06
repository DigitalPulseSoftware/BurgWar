RegisterClientScript()
RegisterClientAssets("placeholder/patator.png")

local entity = ScriptedEntity({
	IsNetworked = true
})

entity:On("init", function (self)
	self:SetCollider({ Collider = Circle(Vec2(0, 0), 128 * 0.3), IsTrigger = true })
	self:EnableCollisionCallbacks(true)

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.33, 0.33),
			TexturePath = "placeholder/patator.png"
		})
	end
end)

entity:On("collisionstart", function (self, other)
	if (SERVER and other.Name == "burger") then
		local owner = other:GetOwner()
		if (not owner:HasWeapon("weapon_patator")) then
			owner:GiveWeapon("weapon_patator")
			self:Kill()
			self.Parent:OnPowerupConsumed()
		end
	end

	return false
end)
