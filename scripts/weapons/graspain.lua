RegisterClientScript()

local scale = 0.75

local weapon = ScriptedWeapon({
	Cooldown = 0.5,
	Scale = scale,
	Sprite = "placeholder/baguette.png",
	SpriteOrigin = Vec2(40, 20) * scale,
	WeaponOffset = Vec2(20, -60) -- This should not be here
})

RegisterClientAssets(weapon.Sprite)

local maxDist = 1000

if (SERVER) then
	function weapon:OnAttack()
		if (self.GrapplePull or self.GrappleSprite) then
			self:Release()
		else
			self:Launch()
		end
	end

	function weapon:Launch()
		local startOffset = Vec2(85 * self.Scale, 0)

		local startPos = self:ToGlobalPosition(startOffset)
		local endPos = startPos + self:GetDirection() * maxDist

		local nearestFraction = math.huge
		local nearestResult

		local traceResult = physics.TraceMultiple(self:GetLayerIndex(), startPos, endPos, function (result)
			if (result.fraction < nearestFraction) then
				if (result.hitEntity) then
					-- Ignore player
					if (result.hitEntity == self:GetOwnerEntity()) then
						return
					end
				end

				nearestResult = result
				nearestFraction = result.fraction
			end
		end)

		if (nearestFraction <= 0.001) then
			return
		end

		local targetEntity
		local targetOffset
		local duration = 0.5
		local hitPos

		if (nearestResult) then
			targetEntity = nearestResult.hitEntity
			targetOffset = targetEntity:ToLocalPosition(nearestResult.hitPos)
			duration = duration * nearestResult.fraction
			hitPos = nearestResult.hitPos
		else
			hitPos = endPos
			targetOffset = endPos
		end

		local direction = self:GetDirection()

		self.GrappleSprite = match.CreateEntity({
			Type = "entity_grapple_sprite",
			LayerIndex = self:GetLayerIndex(),
			LifeOwner = self,
			Properties = {
				duration = duration,
				source_entity = self,
				source_offset = startOffset,
				target_entity = targetEntity or NoEntity,
				target_offset = targetOffset
			}
		})

		timer.Sleep(math.floor(duration * 1000))

		if (targetEntity and targetEntity:IsValid()) then
			self.GrapplePull = match.CreateEntity({
				Type = "entity_grapple_pull",
				LayerIndex = self:GetLayerIndex(),
				LifeOwner = self,
				Properties = {
					source_entity = self:GetOwnerEntity(),
					source_offset = startOffset,
					target_entity = targetEntity or NoEntity,
					target_offset = targetOffset
				}
			})

			self:SetNextTriggerTime(match.GetMilliseconds())
		else
			self:Release()
		end
	end

	function weapon:Release()
		if (self.GrapplePull and self.GrapplePull:IsValid()) then
			self.GrapplePull:Remove()
		end
		self.GrapplePull = nil

		if (self.GrappleSprite and self.GrappleSprite:IsValid()) then
			self.GrappleSprite:Retract()
		end
		self.GrappleSprite = nil
	end
end
