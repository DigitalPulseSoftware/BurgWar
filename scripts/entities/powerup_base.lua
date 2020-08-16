RegisterClientScript()

ENTITY.IsNetworked = true

ENTITY.Properties = {
	{ Name = "duration", Type = PropertyType.Float, Default = 5 },
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
	self.Duration = math.floor(self:GetProperty("duration") * 1000)

	local scale = self:GetProperty("scale")
	local size = self:GetProperty("size")
	local spriteSize = scale * size

	local colliderSize = spriteSize / 2
	self:SetCollider(Rect(-colliderSize, colliderSize))
	self:EnableCollisionCallbacks(true)

	if (CLIENT) then
		self.Sprite = self:AddSprite({
			RenderOrder = 1000,
			Scale = scale,
			Size = size,
			TextureCoords = self:GetProperty("textureCoords"),
			TexturePath = self:GetProperty("texture")
		})
	end
end

if (SERVER) then
	function ENTITY:OnCollisionStart(other)
		-- Disabling an entity won't have any effect until the next tick, but we may still be resolving collisions
		if (not self:IsEnabled()) then
			return false
		end

		if (other.Name == "burger") then
			local data = self:Apply(other)

			timer.Create(self.Duration, function ()
				if (other:IsValid()) then
					self:Unapply(other, data)
				end
			end)

			self.Parent:OnPowerupConsumed()
		end

		return false
	end
end
