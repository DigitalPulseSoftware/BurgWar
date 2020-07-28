RegisterClientScript()
RegisterClientAssets("box.png")

local entity = ScriptedEntity({
	IsNetworked = true,
	MaxHealth = 1000,
	Properties = {
		{ Name = "dynamic", Type = PropertyType.Boolean, Default = true, Shared = true },
		{ Name = "size", Type = PropertyType.Float, Default = 1.0, Shared = true }
	}
})

function entity:Initialize()
	local size = self:GetProperty("size")

	local colliderSize = Vec2(0.2, 0.2) * size * 256 / 2
	self:SetCollider(Rect(-colliderSize, colliderSize))

	if (self:GetProperty("dynamic")) then
		self:InitRigidBody(size * 50, 10)
	end

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.2, 0.2) * size,
			TexturePath = "box.png"
		})
	end
end
