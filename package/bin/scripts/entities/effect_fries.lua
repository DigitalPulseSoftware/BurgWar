RegisterClientScript()
RegisterClientAssets("placeholder/frite_particle.png")

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "lifetime", Type = PropertyType.Float, Shared = true },
		{ Name = "size", Default = 30, Type = PropertyType.Float, Shared = true }
	}
})

entity:On("init", function (self)
	local lifetime = self:GetProperty("lifetime")
	self:SetLifeTime(lifetime)

	if (CLIENT) then
		local size = self:GetProperty("size")
		local sizeRandom = size / 10

		self.ParticleGroup = particle.CreateGroup(100, "billboard2d")
		self.ParticleGroup:AddController("alphaFromLife", { maxLife = lifetime })
		self.ParticleGroup:AddController("life")
		self.ParticleGroup:AddController("velocity", { damping = 0.5 })
		self.ParticleGroup:AddGenerator("color", { min = {  }, max = {  } })
		self.ParticleGroup:AddGenerator("life", { min = 0, max = lifetime })
		self.ParticleGroup:AddGenerator("position", { origin = self:GetPosition(), maxDist = 5 })
		self.ParticleGroup:AddGenerator("rotation", { min = -180, max = 180 })
		self.ParticleGroup:AddGenerator("size", { min = Vec2(size - sizeRandom, size - sizeRandom), max = Vec2(size + sizeRandom, size + sizeRandom) })
		self.ParticleGroup:AddGenerator("velocity_unit_random", { minSpeed = 100, maxSpeed = 300 })
		self.ParticleGroup:SetRenderer("billboard", {
			texturePath = "placeholder/frite_particle.png"
		})

		self.ParticleGroup:GenerateParticles(100)
	end
end)

entity:On("destroyed", function (self)
	self.ParticleGroup:Kill()
end)