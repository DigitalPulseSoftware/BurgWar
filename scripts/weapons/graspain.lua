RegisterClientScript()

local scale = 0.75

local weapon = ScriptedWeapon({
	Cooldown = 0.5,
	Scale = scale,
	Sprite = "placeholder/baguette.png",
	SpriteOrigin = Vec2(40, 20) * scale,
	WeaponOffset = Vec2(20, -60) -- This should not be here
})

local breakSound = "placeholder/grapple_break.wav"

RegisterClientAssets(weapon.Sprite)
RegisterClientAssets(breakSound)

local maxDist = 1000

if (SERVER) then
	network.RegisterPacket("GrasPain_BreakSound")

	weapon:OnAsync("attack", function (self)
		if (self.GrapplePull or self.GrappleSprite) then
			self:Release()
		else
			self:Launch()
		end
	end)

	weapon:On("switchoff", function (self)
		self:Release()
	end)

	function weapon:Launch()
		local startOffset = Vec2(85 * self.Scale, 0)

		local startPos = self:ToGlobalPosition(startOffset)
		local endPos = startPos + self:GetDirection() * maxDist

		local nearestFraction = math.huge
		local nearestResult

		physics.TraceMultiple(self:GetLayerIndex(), startPos, endPos, function (result)
			if (result.fraction < nearestFraction) then
				if (result.hitEntity) then
					-- Ignore player
					if (result.hitEntity == self:GetOwnerEntity()) then
						return
					end

					-- Ignore entities with Passthrough flag
					if (result.hitEntity.Passthrough) then
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

		if (nearestResult) then
			targetEntity = nearestResult.hitEntity
			targetOffset = targetEntity:ToLocalPosition(nearestResult.hitPos)
			duration = duration * nearestResult.fraction
		else
			targetOffset = endPos
		end

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

		local ownerEntity = self:GetOwnerEntity()
		if (ownerEntity and ownerEntity:IsValid() and targetEntity and targetEntity:IsValid()) then
			local layerIndex = self:GetLayerIndex()
			self.GrapplePull = match.CreateEntity({
				Type = "entity_grapple_pull",
				LayerIndex = layerIndex,
				LifeOwner = self,
				Properties = {
					source_entity = ownerEntity,
					source_offset = startOffset,
					target_entity = targetEntity or NoEntity,
					target_offset = targetOffset,
					force = ownerEntity:GetMass() * 10
				}
			})
			self.GrapplePull:On("Destroyed", function (pull)
				if (self.GrapplePull) then
					-- Premature destruction
					self:Release()

					local packet = network.NewPacket("GrasPain_BreakSound")
					packet:WriteCompressedUnsigned(layerIndex)
					packet:WriteVector2(self:GetPosition())

					self:GetOwner():SendPacket(packet)
				end
			end)

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
else
	network.SetHandler("GrasPain_BreakSound", function (packet)
		local layerIndex = packet:ReadCompressedUnsigned()
		local position = packet:ReadVector2()

		match.PlaySound({
			LayerIndex = layerIndex,
			Position = position,
			SoundPath = breakSound
		})
	end)
end
