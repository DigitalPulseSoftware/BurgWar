RegisterClientScript("powerup.lua")

ENTITY.IsNetworked = true
ENTITY.CollisionType = 2
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 0

ENTITY.Properties = {
	{ Name = "respawntime", Type = PropertyType.Integer, Default = 30 }
}

ENTITY.CanSpawn = true

function ENTITY:Initialize()
	if (CLIENT) then
		self:AddSprite("placeholder/socle.png", Vec2(0.5, 0.5))
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
				local powerup = GM:CreateEntity("entity_powerup_health", self:GetPosition() + Vec2(0, -10))
				powerup.Parent = self

				self.CanSpawn = false
			end

			self.NextRespawn = os.time() + self:GetProperty("respawntime")
		end
	end
end
