include("cl_camera.lua")
include("cl_scoreboard.lua")

local gamemode = ScriptedGamemode()

gamemode:On("Init", function (self)
	local musicName = self.Musics[math.random(1, #self.Musics)]
	self.Music = sound.CreateMusicFromFile(musicName)
	if (self.Music) then
		self.Music:EnableLooping(true)
		self.Music:Play()
	end
end)

gamemode:On("ChangeLayer", function (self, oldLayer, newLayer)
	-- FIXME: This shouldn't be handled by this callback

	if (oldLayer ~= NoLayer) then
		for _, ent in pairs(match.GetEntitiesByClass("entity_visible_layer", oldLayer)) do
			ent:OnLeaveLayer(oldLayer)
		end
	end

	if (newLayer ~= NoLayer) then
		for _, ent in pairs(match.GetEntitiesByClass("entity_visible_layer", newLayer)) do
			ent:OnEnterLayer(newLayer)
		end
	end
end)
