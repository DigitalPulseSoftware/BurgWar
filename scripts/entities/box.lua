RegisterClientScript("box.lua")

ENTITY.IsNetworked = true
ENTITY.CollisionType = 2
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 1000

ENTITY.Properties = {
	{ Name = "size", Type = PropertyType.Float, Default = 1.0, Shared = true }
}

function ENTITY:Initialize()
	local size = self:GetProperty("size")

	local colliderSize = Vec2(0.2 * size, 0.2) * 256 / 2
	self:SetCollider(Rect(-colliderSize, colliderSize))
	self:InitRigidBody(size * 50, 10)

	if (CLIENT) then
		self:AddSprite("../resources/box.png", Vec2(0.2 * size, 0.2))
	end
end
