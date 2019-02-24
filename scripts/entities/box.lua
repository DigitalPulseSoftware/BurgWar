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

	if (not EDITOR) then
		local colliderSize = 256 * 0.2 / 2 * size
		self:SetCollider(Rect(Vec2(-colliderSize, -colliderSize), Vec2(colliderSize, colliderSize)))
		self:InitRigidBody(100, 10)
	end

	if (CLIENT or EDITOR) then
		self:AddSprite("../resources/box.png", Vec2(0.2, 0.2) * size)
	end
end
