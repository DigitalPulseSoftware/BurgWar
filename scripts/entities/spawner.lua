RegisterClientScript()
RegisterClientAssets("placeholder/socle.png")

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "respawntime", Type = PropertyType.Integer, Default = 30 },
		{ Name = "spawn_entity", Type = PropertyType.Entity, Default = NoEntity }
	}
})

entity.CanSpawn = false

entity:On("Init", function (self)
	if (SERVER) then
		self.SpawnedEntity = self:GetProperty("spawn_entity")
		if (self.SpawnedEntity and self.SpawnedEntity:IsValid()) then
			self.SpawnData = self.SpawnedEntity:DumpCreationInfo()

			self.SpawnedEntity:On("destroyed", function ()
				if (self:IsValid()) then
					self:OnSpawnedEntityDestroyed()
				end
			end)
		end
	else
		self:AddSprite({
			Scale = Vec2(0.5, 0.5),
			TexturePath = "placeholder/socle.png"
		})
	end
end)

if (SERVER) then
	entity.NextRespawn = os.time()

	function entity:OnSpawnedEntityDestroyed()
		self.CanSpawn = true
		self.NextRespawn = os.time() + self:GetProperty("respawntime")
	end

	entity:On("Tick", function (self)
		if (not self.SpawnData or not self.CanSpawn) then
			return
		end

		local now = os.time()
		if (now >= self.NextRespawn) then
			self.NextRespawn = os.time() + self:GetProperty("respawntime")
			self.CanSpawn = false

			self.SpawnedEntity = match.CreateEntity(self.SpawnData)
			self.SpawnedEntity:On("Destroyed", function ()
				if (self:IsValid()) then
					self:OnSpawnedEntityDestroyed()
				end
			end)
		end
	end)
end
