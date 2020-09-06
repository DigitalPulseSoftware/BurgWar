RegisterClientScript("cl_camera.lua")
RegisterClientScript("cl_init.lua")
RegisterClientScript("cl_scoreboard.lua")
RegisterClientScript("sh_scores.lua")
RegisterClientScript("shared.lua")

local gamemode = ScriptedGamemode()
print("Mode de jeu: " .. gamemode.Name)

include("sh_scores.lua")
