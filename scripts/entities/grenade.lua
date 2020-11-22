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

entity:On("init", function (self)
	self.ExplosionTick = match.GetLocalTick() + self:GetProperty("lifetime") / match.GetTickDuration()
	self:SetColliders(Circle(Vec2(0, 0) * 0.2, 128 * 0.2))
	self:InitRigidBody(20, 10)

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.2, 0.2),
			TexturePath = "grenade.png"
		})
	end
end)

entity:On("tick", function (self)
	local currentTick = match.GetLocalTick()
	if (currentTick >= self.ExplosionTick) then
		self:Explode()

		if (SERVER) then
			self:Kill()
		end
	end
end)

function entity:Explode()
	if (self.Exploded) then
		return
	end

	self.Exploded = true

	local scale = self:GetScale()

	if (CLIENT) then
		self:PlaySound(self.ExplosionSounds[math.random(1, #self.ExplosionSounds)], false, false, true)

		local playerPosition = engine_GetPlayerPosition(0)
		if (playerPosition) then
			local distance = (self:GetPosition() - playerPosition):Length()
			local minDistance = 1000
			local strength = math.min(50, 250 / math.max(1, distance - minDistance))
			if (strength > 0.1) then
				match.GetGamemode():ShakeCamera(1, strength)
			end
		end

		match.CreateEntity({
			Type = "entity_effect_smoke",
			LayerIndex = self:GetLayerIndex(),
			Position = self:GetPosition(),
			Properties = {
				lifetime = math.random(2, 3),
				size = scale * 100
			}
		})
	else
		local pos = self:GetPosition()
		local maxs = Vec2(256, 256) * scale
		local mins = Vec2(-256, -256) * scale

		self:DealDamage(self:GetPosition(), math.random(50, 100), Rect(pos + mins, pos + maxs), 100000)
	end
end

if (CLIENT) then
	entity:On("destroyed", function (self)
		if (self:GetHealth() == 0) then
			self:Explode()
		end
	end)
end
