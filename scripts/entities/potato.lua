RegisterClientScript()
RegisterClientAssets("placeholder/potato.png")

local entity = ScriptedEntity({
	IsNetworked = true
})

entity.ExplosionSounds = {
    "placeholder/explosion1.wav",
    "placeholder/explosion2.wav",
    "placeholder/explosion3.wav",
    "placeholder/explosion4.wav",
}
RegisterClientAssets(entity.ExplosionSounds)

entity:On("init", function (self)
	self.ExplosionTick = match.GetLocalTick() + 5 / match.GetTickDuration()
	self:SetColliders({ 
		Collider = Circle(Vec2(0, 0) * 0.2, 128 * 0.2),
		ColliderType = SERVER and ColliderType.Callback or ColliderType.Default
	})

	self:InitRigidBody(20, 10)

	if (CLIENT) then
		self:AddSprite({
			RenderOrder = -2,
			Scale = Vec2(0.1, 0.1),
			TexturePath = "placeholder/potato.png"
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

if (SERVER) then
	entity:On("collisionstart", function (self, other)
		if (other.Passthrough) then
			return false
		end

		self:Explode()
		self:Kill()
		return true
	end)
end

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
			Type = "entity_effect_fries",
			LayerIndex = self:GetLayerIndex(),
			Position = self:GetPosition(),
			Properties = {
				lifetime = math.random() + 0.5,
				size = scale * 30
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
