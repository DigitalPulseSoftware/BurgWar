ENTITY.IsNetworked = true
ENTITY.Sprite = "burger2.png"
ENTITY.Scale = 0.33
ENTITY.PlayerControlled = true
ENTITY.MaxHealth = 1000
ENTITY.HasInputs = true
ENTITY.Faces = {
	Attack = "faces/face_attack.png",
	Damage = "faces/face_damage.png",
	Default = "faces/face_default.png",
	Rampage = "faces/face_rampage.png",
	Victory = "faces/face_victory.png"
}

RegisterClientAssets(ENTITY.Sprite)
for _, texturePath in pairs(ENTITY.Faces) do
	RegisterClientAssets(texturePath)
end

local controller = BasicPlayerMovementController.new()

function ENTITY:Initialize()
	local size = {277 / 3, 253 / 3}

	local rect = Rect()
	rect.x = -size[1] / 2 + 5
	rect.y = -size[2] - 20
	rect.width = size[1] - 10
	rect.height = size[2] - 3 + 20

	self:UpdatePlayerMovementController(controller)
	self:SetCollider(rect)
	self:InitRigidBody(50, 10, false)
end
