RegisterClientScript("box.lua")
RegisterClientAssets("box.png")

ENTITY.IsNetworked = true
ENTITY.CollisionType = 2
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 1000

ENTITY.Properties = {
	{ Name = "dynamic", Type = PropertyType.Boolean, Default = true, Shared = true },
	{ Name = "size", Type = PropertyType.Float, Default = 1.0, Shared = true }
}

function ENTITY:Initialize()
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
