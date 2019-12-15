RegisterClientScript()
RegisterClientAssets("placeholder/socle.png")

ENTITY.IsNetworked = true

ENTITY.Properties = {
	{ Name = "respawntime", Type = PropertyType.Integer, Default = 30 },
	{ Name = "powerup_type", Type = PropertyType.String, Default = "" }
}

ENTITY.CanSpawn = true

function ENTITY:Initialize()
	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.5, 0.5),
			TexturePath = "placeholder/socle.png"
		})
	end
end

if (SERVER) then
	ENTITY.NextRespawn = os.time()

	function ENTITY:OnPowerupConsumed()
		self.CanSpawn = true
		self.NextRespawn = os.time() + self:GetProperty("respawntime")
	end

	function ENTITY:OnTick()
		local now = os.time()
		if (now >= self.NextRespawn) then
			if (self.CanSpawn) then
				local powerup = match.CreateEntity({
					Type = self:GetProperty("powerup_type"),
					LayerIndex = self:GetLayerIndex(),
					Parent = self,
					Position = Vec2(0, -10)
				})

				powerup.Parent = self

				self.CanSpawn = false
			end

			self.NextRespawn = os.time() + self:GetProperty("respawntime")
		end
	end
end
