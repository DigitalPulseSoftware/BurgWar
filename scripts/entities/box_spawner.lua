RegisterClientScript()
RegisterClientAssets("placeholder/socle.png")

ENTITY.IsNetworked = true

ENTITY.Properties = {
	{ Name = "respawntime", Type = PropertyType.Integer, Default = 30 }
}

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

	function ENTITY:OnTick()
		local now = os.time()

		if (self.SpawnedBox) then
			if (not self.SpawnedBox:IsValid() or not self.SpawnedBox:IsSleeping()) then
				self.SpawnedBox = nil
				self.NextRespawn = os.time() + self:GetProperty("respawntime")
			end
		end

		if (now >= self.NextRespawn and not self.SpawnedBox) then
			self.SpawnedBox = match.CreateEntity({
				Type = "entity_box",
				LayerIndex = self:GetLayerIndex(),
				Position = self:GetPosition() + Vec2(0, -10),
				Properties = {
					size = 2
				}
			})
			self.SpawnedBox:ForceSleep()
		end
	end
end
