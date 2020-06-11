RegisterClientScript()
RegisterClientAssets("placeholder/frite_particle.png")

ENTITY.IsNetworked = true
ENTITY.Properties = {
	{ Name = "lifetime", Type = PropertyType.Float, Shared = true },
}

function ENTITY:Initialize()
	local lifetime = self:GetProperty("lifetime")
	self:SetLifeTime(lifetime)

	self.ParticleGroup = particle.CreateGroup(100, "billboard2d")
	self.ParticleGroup:AddController("alphaFromLife", { maxLife = lifetime })
	self.ParticleGroup:AddController("life")
	self.ParticleGroup:AddController("velocity", { damping = 0.5 })
	self.ParticleGroup:AddGenerator("color", { min = {  }, max = {  } })
	self.ParticleGroup:AddGenerator("life", { min = 0, max = lifetime })
	self.ParticleGroup:AddGenerator("position", { origin = self:GetPosition(), maxDist = 5 })
	self.ParticleGroup:AddGenerator("rotation", { min = -180, max = 180 })
	self.ParticleGroup:AddGenerator("size", { min = Vec2(20, 20), max = Vec2(40, 40) })
	self.ParticleGroup:AddGenerator("velocity_unit_random", { minSpeed = 100, maxSpeed = 300 })
	self.ParticleGroup:SetRenderer("billboard", {
		texturePath = "placeholder/frite_particle.png"
	})

	self.ParticleGroup:GenerateParticles(100)
end

function ENTITY:OnKilled()
	self.ParticleGroup:Kill()
end