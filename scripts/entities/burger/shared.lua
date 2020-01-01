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
ENTITY.Hat = "hats/santa_hat.png"

RegisterClientAssets(ENTITY.Sprite)
for _, texturePath in pairs(ENTITY.Faces) do
	RegisterClientAssets(texturePath)
end

RegisterClientAssets(ENTITY.Hat)

local controller = BasicPlayerMovementController.new()

function ENTITY:Initialize()
	local size = {277 / 3, 253 / 3}

	local rect = Rect()
	rect.x = -size[1] / 2 + 5
	rect.y = -size[2] - 20
	rect.width = size[1] - 10
	rect.height = size[2] - 3 + 20

	local topLeft = rect:GetCorner(false, false)
	local topRight = rect:GetCorner(true, false)
	local bottomLeft = rect:GetCorner(false, true)
	local bottomRight = rect:GetCorner(true, true)

	-- Note that some positions are adjusted to prevent getting stuck

	local colliders = {
		{
			-- Bottom
			-- Adjust offsets to prevent
			Collider = Segment(bottomLeft + Vec2(2, 0), bottomRight - Vec2(2, 0)),
			Friction = 0,
			FromNeighbor = topLeft,
			ToNeighbor = topRight
		},
		{
			-- Right side
			Collider = Segment(bottomRight - Vec2(0, 2), topRight),
			Friction = 0,
			FromNeighbor = bottomLeft,
			ToNeighbor = topLeft
		},
		{
			-- Top
			Collider = Segment(topRight - Vec2(2, 0), topLeft + Vec2(2, 0)),
			Friction = 1,
			FromNeighbor = bottomRight,
			ToNeighbor = bottomLeft
		},
		{
			-- Left side
			Collider = Segment(topLeft, bottomLeft - Vec2(0, 2)),
			Friction = 0,
			FromNeighbor = topRight,
			ToNeighbor = bottomRight
		},
	}

	self:UpdatePlayerMovementController(controller)
	self:SetCollider(colliders)
	self:InitRigidBody(50, 1, false)
	self:SetMomentOfInertia(math.huge) -- Disable rotation
end
