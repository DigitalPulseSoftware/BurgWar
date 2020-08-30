include("cl_camera.lua")
include("cl_scoreboard.lua")

print("respawntime", GM:GetProperty("respawntime"))

GM:On("changelayer", function (self, oldLayer, newLayer)
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
