RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "duration", Type = PropertyType.Float, Default = 5 },
		{ Name = "scale", Type = PropertyType.FloatSize, Default = Vec2(1.0, 1.0), Shared = true },
		{ Name = "size", Type = PropertyType.FloatSize, Default = Vec2(1.0, 1.0), Shared = true },
		{ Name = "texture", Type = PropertyType.Texture, Default = "", Shared = true },
		{ Name = "textureCoords", Type = PropertyType.FloatRect, Default = Rect(Vec2(0.0, 0.0), Vec2(1.0, 1.0)), Shared = true }
	},
	EditorActions = {
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
	},
	CustomEvents = {
		{
			Name = "apply",
			ReturnType = "any",
			Combinator = function (a, b) 
				return a
			end
		},
		{
			Name = "unapply",
			ReturnType = nil
		}
	}
})

entity.Passthrough = true

entity:On("init", function (self)
	self.Duration = math.floor(self:GetProperty("duration") * 1000)

	local scale = self:GetProperty("scale")
	local size = self:GetProperty("size")
	local spriteSize = scale * size

	local colliderSize = spriteSize / 2
	self:SetColliders({
		Collider = Rect(-colliderSize, colliderSize),
		ColliderType = ColliderType.Callback
	})

	if (CLIENT) then
		self.Sprite = self:AddSprite({
			RenderOrder = 100,
			Scale = scale,
			Size = size,
			TextureCoords = self:GetProperty("textureCoords"),
			TexturePath = self:GetProperty("texture")
		})
	end
end)

if (SERVER) then
	entity:On("collisionstart", function (self, other)
		-- Disabling an entity won't have any effect until the next tick, but we may still be resolving collisions
		if (not self:IsEnabled()) then
			return false
		end

		if (other.Name == "burger") then
			local data = self:Trigger("apply", other)

			timer.Create(self.Duration, function ()
				if (other:IsValid()) then
					self:Trigger("unapply", other, data)
				end

				if (not self.Parent) then
					self:Kill()
				end
			end)

			if (self.Parent) then
				self.Parent:OnPowerupConsumed()
			else
				self:Disable()
			end
		end

		return false
	end)
end
