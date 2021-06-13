RegisterClientScript()

RoundState = enums.Create({
	Finished = 0,
	Playing = 1,
	Starting = 2,
	Waiting = 3,
})

local gamemode = ScriptedGamemode()
gamemode.State = RoundState.Waiting

local fightSound = "placeholder/fight.wav"

RegisterClientAssets(fightSound)

if (SERVER) then
	network.RegisterPacket("SSB_RoundInit")
	network.RegisterPacket("SSB_RoundUpdate")

	gamemode:OnAsync("PlayerJoined", function (self, player)
		local packet = network.NewPacket("SSB_RoundInit")
		packet:WriteCompressedUnsigned(self.State)

		player:SendPacket(packet)
	end)
	
	function gamemode:UpdateState(newState)
		self:Trigger("RoundStateUpdate", self.State, newState)
		self.State = newState

		local packet = network.NewPacket("SSB_RoundUpdate")
		packet:WriteCompressedUnsigned(self.State)

		match.BroadcastPacket(packet)
	end
else
	network.SetHandler("SSB_RoundInit", function (packet)
		gamemode.State = packet:ReadCompressedUnsigned()
	end)

	network.SetHandler("SSB_RoundUpdate", function (packet)
		local newState = packet:ReadCompressedUnsigned()
		gamemode:Trigger("RoundStateUpdate", gamemode.State, newState)
		gamemode.State = newState

		if (gamemode.State == RoundState.Playing) then
			match.PlaySound({
				LayerIndex = engine_GetActiveLayer(), -- FIXME
				SoundPath = fightSound,
				Spatialized = false,
			})
		end
	end)
end
