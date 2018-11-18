print(ENTITY.Name)
ENTITY.IsNetworked = true
ENTITY.Sprite = "../resources/burger.png"
ENTITY.Scale = 0.5
ENTITY.CollisionType = 1 -- Player
ENTITY.PlayerControlled = true
ENTITY.MaxHealth = 1000

function ENTITY:Initialize()
	local size = {277 / 2, 253 / 2}

	local rect = Rect()
	rect.x = -size[1] / 2
	rect.y = -size[2]
	rect.width = size[1]
	rect.height = size[2] - 3

	self:SetCollider(rect)
	self:InitRigidBody(300, 10, false)
end
