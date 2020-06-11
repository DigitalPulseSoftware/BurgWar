RegisterClientScript()

ENTITY.IsNetworked = true
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 0

ENTITY.Properties = {
	{ Name = "physical", Type = PropertyType.Boolean, Default = true, Shared = true },
	{ Name = "health", Type = PropertyType.Integer, Default = 0, Shared = true },
	{ Name = "mass", Type = PropertyType.Float, Default = 0, Shared = true },
	{ Name = "renderOrder", Type = PropertyType.Integer, Default = 0, Shared = true },
	{ Name = "scale", Type = PropertyType.FloatSize, Default = Vec2(1.0, 1.0), Shared = true },
	{ Name = "size", Type = PropertyType.FloatSize, Default = Vec2(1.0, 1.0), Shared = true },
	{ Name = "texture", Type = PropertyType.Texture, Default = "", Shared = true },
	{ Name = "textureCoords", Type = PropertyType.FloatRect, Default = Rect(Vec2(0.0, 0.0), Vec2(1.0, 1.0)), Shared = true }
}

if (EDITOR) then
	ENTITY.EditorActions = {
		{
			Name = "resizeSprite",
			Label = "Resize to texture size",
			OnTrigger = function (entityEditor)
				local texturePath = entityEditor:GetProperty("texture")
				local texture = assets.GetTexture(texturePath)
				if (not texture) then
					print("Invalid texture " .. texturePath)
					return
				end

				entityEditor:UpdateProperty("size", texture:GetSize())
			end
		}
	}
end

function ENTITY:Initialize()
	local mass = self:GetProperty("mass")
	local scale = self:GetProperty("scale")
	local size = self:GetProperty("size")
	local spriteSize = scale * size

	if (self:GetProperty("physical")) then
		local colliderSize = spriteSize / 2
		self:SetCollider(Rect(-colliderSize, colliderSize))
	end

	if (mass > 0) then
		self:InitRigidBody(mass, 10)
	end

	if (CLIENT) then
		self.Sprite = self:AddSprite({
			RenderOrder = self:GetProperty("renderOrder"),
			Scale = scale,
			Size = size,
			TextureCoords = self:GetProperty("textureCoords"),
			TexturePath = self:GetProperty("texture")
		})
	end
end
