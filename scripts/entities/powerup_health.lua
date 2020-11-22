RegisterClientScript()
RegisterClientAssets("placeholder/healthpack.png")

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {}
})

entity:On("init", function (self)
	local colliderSize = Vec2(400, 200) / 4 / 2
	self:SetColliders({
		Collider = Rect(-colliderSize, colliderSize),
		IsTrigger = true,
		ColliderType = ColliderType.Callback
	})

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.25, 0.25),
			TexturePath = "placeholder/healthpack.png"
		})
	end
end)

if (SERVER) then
	entity:On("collisionstart", function (self, other)
		if (other.Name == "burger") then
			if (not other:IsFullHealth()) then
				other:Heal(250)
				self:Kill()
				self.Parent:OnPowerupConsumed()
			end
		end

		return false
	end)
end