RegisterClientScript()
RegisterClientAssets("smoke.png")

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "lifetime", Type = PropertyType.Float, Shared = true },
		{ Name = "size", Default = 100, Type = PropertyType.Float, Shared = true }
	}
})

entity:On("init", function (self)
	local lifetime = self:GetProperty("lifetime")
	self:SetLifeTime(lifetime)

	if (CLIENT) then
		local size = self:GetProperty("size")
		local sizeRandom = size / 10

		self.ParticleGroup = particle.CreateGroup(25, "billboard2d")
		self.ParticleGroup:AddController("alphaFromLife", { maxAlpha = 255, maxLife = lifetime })
		self.ParticleGroup:AddController("life")
		self.ParticleGroup:AddController("size", { size = Vec2(size, size) })
		self.ParticleGroup:AddController("velocity", { damping = 0.005 })
		self.ParticleGroup:AddGenerator("color", { min = { r = 0.4, g = 0.4, b = 0.4 }, max = { r = 0.4, g = 0.4, b = 0.4 } })
		self.ParticleGroup:AddGenerator("life", { min = 0, max = lifetime })
		self.ParticleGroup:AddGenerator("position", { origin = self:GetPosition(), maxDist = 10 })
		self.ParticleGroup:AddGenerator("rotation", { min = -180, max = 180 })
		self.ParticleGroup:AddGenerator("size", { min = Vec2(size - sizeRandom, size - sizeRandom), max = Vec2(size + sizeRandom, size + sizeRandom) })
		self.ParticleGroup:AddGenerator("velocity_unit_random", { minSpeed = 200, maxSpeed = 300 })
		self.ParticleGroup:SetRenderer("billboard", {
			texturePath = "smoke.png"
		})

		self.ParticleGroup:GenerateParticles(25)
	end
end)

entity:On("destroyed", function (self)
	self.ParticleGroup:Kill()
end)