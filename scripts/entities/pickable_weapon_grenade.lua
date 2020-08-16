RegisterClientScript()
RegisterClientAssets("grenade.png")

local entity = ScriptedEntity({
	IsNetworked = true
})

entity:On("init", function (self)
	self:SetCollider({ Collider = Circle(Vec2(0, 0), 128 * 0.3), IsTrigger = true })
	self:EnableCollisionCallbacks(true)

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.25, 0.25),
			TexturePath = "grenade.png"
		})
	end
end)

entity:On("collisionstart", function (self, other)
	if (SERVER and other.Name == "burger") then
		local owner = other:GetOwner()
		if (not owner:HasWeapon("weapon_grenade")) then
			owner:GiveWeapon("weapon_grenade")
			self:Kill()

			if (self.Parent) then
				self.Parent:OnPowerupConsumed()
			end
		end
	end

	return false
end)

