local entity = ScriptedEntity({
	IsNetworked = true,
	HasInputs = true,
	PlayerControlled = true,
	MaxHealth = 100,
	Properties = {
		{ Name = "seed", Type = PropertyType.Integer, Shared = true },
	}
})

entity.Sprite = "burger2.png"
entity.Scale = 0.33
entity.Faces = {
	Attack = "faces/face_attack.png",
	Damage = "faces/face_damage.png",
	Default = "faces/face_default.png",
	Rampage = "faces/face_rampage.png",
	Victory = "faces/face_victory.png"
}

entity.Elements = {
	Cheeses = {
		Sprites = {
			"burger/cheeses/blue.png", 
			{
				OffsetScale = 0.4,
				Origin = Vec2(0.5, 0.75),
				Path = "burger/cheeses/cheddar.png",
			}, 
			"burger/cheeses/emmental.png", 
			"burger/cheeses/goat_cheese.png"
		},
		OffsetScale = 0.5
	},
	Bottoms = {
		Sprites = {
			"burger/bottoms/brioche.png", 
			"burger/bottoms/poppy.png", 
			"burger/bottoms/rustic.png", 
			"burger/bottoms/sesame.png"	
		},
		OffsetScale = 0.4
	},
	Tops = { 
		Sprites = {
			"burger/tops/brioche.png", 
			"burger/tops/poppy.png", 
			{
				FaceOrigin = Vec2(0.8, 0.55),
				Path = "burger/tops/rustic.png", 
			},
			"burger/tops/sesame.png"
		},
		FaceOrigin = Vec2(0.7, 0.5),
		Origin = Vec2(0.5, 0.8),
	},
	Meats = {
		Sprites = {
			"burger/meats/paned_chicken.png", 
			"burger/meats/potatoes.png", 
			"burger/meats/steak.png"	
		},
		OffsetScale = 0.4
	},
	Sauces = {
		Sprites = {
			"burger/sauces/barbecue.png", 
			"burger/sauces/ketchup.png", 
			"burger/sauces/mayonnaise.png", 
			"burger/sauces/mustard.png"
		},
		OffsetScale = 0.2,
		Origin = Vec2(0.5, 0.8)
	},
	Vegetables = {
		Sprites = {
			"burger/vegetables/bacon.png", 
			{
				OffsetScale = 0.1,
				Path = "burger/vegetables/onions.png",
			},
			{
				OffsetScale = 0.4,
				Path = "burger/vegetables/pickles.png",
			},
			{
				OffsetScale = 0.3,
				Origin = Vec2(0.5, 0.8),
				Path = "burger/vegetables/salad.png",
			},
			{
				OffsetScale = 0.4,
				Path = "burger/vegetables/tomatoes.png"
			}
		},
		OffsetScale = 0.35
	},
}

for type, element in pairs(entity.Elements) do
	for _, sprite in pairs(element.Sprites) do
		RegisterClientAssets(sprite)
	end
end

RegisterClientAssets(entity.Sprite)
for _, texturePath in pairs(entity.Faces) do
	RegisterClientAssets(texturePath)
end

local controller = BasicPlayerMovementController and BasicPlayerMovementController.new() or nil

function entity:ForEachElement(callback)
	local randomEngine = RandomEngine.new(self:GetProperty("seed"))

	local function AddElement(elementType)
		local elementData = self.Elements[elementType]
		local randomSprite = elementData.Sprites[randomEngine:Generate(1, #elementData.Sprites)]

		callback(elementData, randomSprite)
	end

	AddElement("Bottoms")
	AddElement("Vegetables")
	AddElement("Sauces")
	AddElement("Meats")
	AddElement("Sauces")
	AddElement("Vegetables")
	AddElement("Cheeses")
	AddElement("Tops")
end

function entity:Initialize()
	local cursor = 0
	local maxWidth = 0

	self:ForEachElement(function (elementData, randomSprite)
		local elementOffsetScale = elementData.OffsetScale or 1

		local offsetScale, texturePath
		if (type(randomSprite) == "table") then
			offsetScale = randomSprite.OffsetScale or elementOffsetScale
			texturePath = randomSprite.Path
		else
			assert(type(randomSprite) == "string")

			offsetScale = elementOffsetScale
			texturePath = randomSprite
		end

		local texture = assets.GetTexture(texturePath)
		local spriteSize = texture:GetSize() * self.Scale

		maxWidth = math.max(maxWidth, spriteSize.x)
		cursor = cursor - spriteSize.y * offsetScale
	end)

	self:InitWeaponWielder({
		WeaponOffset = Vec2(maxWidth * 0.65 - maxWidth / 2, cursor * 0.65)
	})
	local size = Vec2(maxWidth, -cursor)
	print(size)
--	local size = Vec2(277, 253) / 3

	local widthOffset = -10 -- reduce a bit
	local heightOffset = 60 * self.Scale -- for hopping animation

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

	if (controller) then
		self:UpdatePlayerMovementController(controller)
	end

	self:SetCollider(colliders)
	self:InitRigidBody(50, 1, false)
	self:SetMomentOfInertia(math.huge) -- Disable rotation
end
