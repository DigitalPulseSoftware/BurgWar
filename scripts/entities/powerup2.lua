RegisterClientScript()
RegisterClientAssets("placeholder/socle.png")

ENTITY.IsNetworked = true

ENTITY.Properties = {
	{ Name = "respawntime", Type = PropertyType.Integer, Default = 30 },
	{ Name = "powerup_entity", Type = PropertyType.Entity, Default = NoEntity }
}

ENTITY.CanSpawn = true

function ENTITY:Initialize()
	if (SERVER) then
		self.Powerup = self:GetProperty("powerup_entity")
		if (self.Powerup and self.Powerup:IsValid()) then
			self.Powerup.Parent = self
		end
	else
		self:AddSprite({
			Scale = Vec2(0.5, 0.5),
			TexturePath = "placeholder/socle.png"
		})
	end
end

if (SERVER) then
	ENTITY.NextRespawn = os.time()

	function ENTITY:OnPowerupConsumed()
		self.Powerup:Disable()
		self.CanSpawn = true
		self.NextRespawn = os.time() + self:GetProperty("respawntime")
	end

	function ENTITY:OnTick()
		local now = os.time()
		if (now >= self.NextRespawn) then
			if (self.CanSpawn) then
				self.Powerup:Enable()
				self.CanSpawn = false
			end

			self.NextRespawn = os.time() + self:GetProperty("respawntime")
		end
	end
end
