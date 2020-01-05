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
	local size = Vec2(277, 253) / 3

	local widthOffset = -10 -- reduce a bit
	local heightOffset = 20 -- for hopping animation

	local topLeft = Vec2(-size.x / 2 - widthOffset / 2, -size.y - heightOffset)
	local topRight = topLeft + Vec2(size.x + widthOffset, 0)
	local bottomLeft = topLeft + Vec2(0, size.y + heightOffset)
	local bottomRight = topRight + Vec2(0, size.y + heightOffset)

	-- Note that some positions are adjusted to prevent getting stuck

	local colliderOffset = 2

	local colliders = {
		{
			-- Bottom
			-- Adjust offsets to prevent
			Collider = Segment(bottomLeft + Vec2(colliderOffset, 0), bottomRight - Vec2(colliderOffset, 0)),
			Friction = 0,
			FromNeighbor = topLeft,
			ToNeighbor = topRight
		},
		{
			-- Right side
			Collider = Segment(bottomRight - Vec2(0, colliderOffset), topRight),
			Friction = 0,
			FromNeighbor = bottomLeft,
			ToNeighbor = topLeft
		},
		{
			-- Top
			Collider = Segment(topRight - Vec2(colliderOffset, 0), topLeft + Vec2(colliderOffset, 0)),
			Friction = 1,
			FromNeighbor = bottomRight,
			ToNeighbor = bottomLeft
		},
		{
			-- Left side
			Collider = Segment(topLeft, bottomLeft - Vec2(0, colliderOffset)),
			Friction = 0,
			FromNeighbor = topRight,
			ToNeighbor = bottomRight
		},
		{
			-- Internal cube (helps with high-speed collisions)
			Collider = Rect(topLeft + Vec2(colliderOffset, colliderOffset) * 2, bottomRight - Vec2(colliderOffset, colliderOffset) * 2),
			Friction = 0
		}
	}

	self:UpdatePlayerMovementController(controller)
	self:SetCollider(colliders)
	self:InitRigidBody(50, 1, false)
	self:SetMomentOfInertia(math.huge) -- Disable rotation
end
