print(ENTITY.Name)
ENTITY.IsNetworked = true
ENTITY.Sprite = "../resources/burger.png"
ENTITY.Scale = 0.33
ENTITY.CollisionType = 1 -- Player
ENTITY.PlayerControlled = true
ENTITY.MaxHealth = 1000
ENTITY.HasInputs = true

function ENTITY:Initialize()
	local size = {277 / 3, 253 / 3}

	local rect = Rect()
	rect.x = -size[1] / 2 + 5
	rect.y = -size[2] - 20
	rect.width = size[1] - 10
	rect.height = size[2] - 3 + 20

	self:SetCollider(rect)
	self:InitRigidBody(50, 10, false)
end
