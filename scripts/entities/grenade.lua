RegisterClientScript()
RegisterClientAssets("grenade.png")

local entity = ScriptedEntity({
	IsNetworked = true,
	MaxHealth = 50,
	Properties = {
		{ Name = "lifetime", Type = PropertyType.Float, Default = 1.0, Shared = true }
	}	
})

entity.ExplosionSounds = {
    "placeholder/explosion1.wav",
    "placeholder/explosion2.wav",
    "placeholder/explosion3.wav",
    "placeholder/explosion4.wav",
}
RegisterClientAssets(entity.ExplosionSounds)

function entity:Initialize()
	self.ExplosionTick = match.GetLocalTick() + self:GetProperty("lifetime") / match.GetTickDuration()
	self:SetCollider(Circle(Vec2(0, 0) * 0.2, 128 * 0.2))
	self:InitRigidBody(20, 10)

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.2, 0.2),
			TexturePath = "grenade.png"
		})
	end
end

function entity:OnTick()
	local currentTick = match.GetLocalTick()
	if (currentTick >= self.ExplosionTick) then
		self:Explode()

		if (SERVER) then
			self:Kill()
		end
	end
end

function entity:Explode()
	if (self.Exploded) then
		return
	end

	self.Exploded = true

	if (CLIENT) then
		self:PlaySound(self.ExplosionSounds[math.random(1, #self.ExplosionSounds)], false, false, true)

		local playerPosition = engine_GetPlayerPosition(0)
		if (playerPosition) then
			local distance = (self:GetPosition() - playerPosition):Length()
			local minDistance = 1000
			local strength = math.min(50, 250 / math.max(1, distance - minDistance))
			if (strength > 0.1) then
				GM:ShakeCamera(1, strength)
			end
		end

		match.CreateEntity({
			Type = "entity_effect_smoke",
			LayerIndex = self:GetLayerIndex(),
			Position = self:GetPosition(),
			Properties = {
				lifetime = math.random(2, 3)
			}
		})
	else
		local pos = self:GetPosition()
		local maxs = Vec2(256, 256)
		local mins = Vec2(-256, -256)

		self:DealDamage(self:GetPosition(), math.random(500, 1000), Rect(pos + mins, pos + maxs), 100000)
	end
end

if (CLIENT) then
	function entity:OnKilled()
		if (self:GetHealth() == 0) then
			self:Explode()
		end
	end
end
