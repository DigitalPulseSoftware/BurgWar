RegisterClientScript()

ENTITY.Base = "entity_burger"

ENTITY.DetectionRange = 500

if (SERVER) then
	function ENTITY:Initialize()
		self.Base.Initialize(self)

		match.CreateWeapon({
			Type = "weapon_sword_emmentalibur",
			Owner = self
		})
	end

	ENTITY.OnTick = utils.OverrideFunction(ENTITY.OnTick, function (self)
		local pos = self:GetPosition()
		local closestEnemy
		local closestEnemyDist = math.huge
		for _, burger in pairs(match.GetEntitiesByClass("entity_burger")) do
			local distance = pos:SquaredDistance(burger:GetPosition())
			if (distance < self.DetectionRange * self.DetectionRange) then
				if (distance < closestEnemyDist) then
					closestEnemy = burger
					closestEnemyDist = distance
				end
			end
		end

		local target = closestEnemy

		local inputs = {}

		if (target) then
			local delta = target:GetPosition() - pos
			local distance = delta:Normalize()

			inputs.aimDirection = delta
			inputs.isAttacking = (distance < 200)

			if (delta.x < 0) then
				inputs.isMovingLeft = true
				inputs.isLookingRight = false
			else
				inputs.isMovingRight = true
			end
		end

		self:UpdateInputs(inputs)
	end)
end