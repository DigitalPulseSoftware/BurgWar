RegisterClientScript("box.lua")

print(ENTITY.Name)
ENTITY.IsNetworked = true
ENTITY.Sprite = "../resources/box.png"
ENTITY.Scale = 0.2
ENTITY.CollisionType = 2
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 1000

ENTITY.Properties = {
	{ Name = "size", Type = PropertyType.Float, Default = 1.0, Shared = true }
}

function ENTITY:Initialize()
	local size = self:GetProperty("size")
	local colliderSize = 256 * 0.2 / 2 * size
	self:SetCollider(Rect(Vec2(-colliderSize, -colliderSize), Vec2(colliderSize, colliderSize)))
	self:InitRigidBody(100, 10)

	if (CLIENT) then
		self:AddSprite("../resources/box.png", Vec2(0.2, 0.2) * self:GetProperty("size"))
	end
end
