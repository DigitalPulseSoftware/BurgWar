RegisterClientScript()
RegisterClientAssets("placeholder/baguette.png")

local entity = ScriptedEntity({
	IsNetworked = true
})

entity:On("init", function (self)
	self:SetCollider({ Collider = Circle(Vec2(0, 0), 128 * 0.3), IsTrigger = true })
	self:EnableCollisionCallbacks(true)

	if (CLIENT) then
		self:AddSprite({
			TexturePath = "placeholder/baguette.png"
		})
	end
end)

entity:On("collisionstart", function (self, other)
	if (SERVER and other.Name == "burger") then
		if (not other:HasWeapon("weapon_graspain")) then
			other:GiveWeapon("weapon_graspain")
			self:Kill()
			self.Parent:OnPowerupConsumed()
		end
	end

	return false
end)
