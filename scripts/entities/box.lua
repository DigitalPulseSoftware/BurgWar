RegisterClientScript("box.lua")

print(ENTITY.Name)
ENTITY.IsNetworked = true
ENTITY.Sprite = "../resources/box.png"
ENTITY.Scale = 0.2
ENTITY.CollisionType = 2
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 1000

function ENTITY:Initialize()
	local size = 256 * 0.2 / 2
	self:SetCollider(Rect(Vec2(-size, -size), Vec2(size, size)))
	self:InitRigidBody(100, 10)
end
