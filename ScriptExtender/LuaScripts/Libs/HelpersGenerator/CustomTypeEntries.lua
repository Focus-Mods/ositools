---@diagnostic disable

return {
Ext_ClientUI = {
After=[[--- @alias BuiltinUISWFName "actionProgression" | "addContent" | "addContent_c" | "areaInteract_c" | "arenaResult" | "book" | "bottomBar_c" | "buttonLayout_c" | "calibrationScreen" | "campaignManager" | "characterAssign" | "characterAssign_c" | "characterCreation" | "characterCreation_c" | "characterSheet" | "chatLog" | "combatLog" | "combatLog_c" | "combatTurn" | "connectionMenu" | "connectivity_c" | "containerInventory" | "containerInventoryGM" | "contextMenu" | "contextMenu_c" | "craftPanel_c" | "credits" | "dialog" | "dialog_c" | "dummyOverhead" | "encounterPanel" | "enemyHealthBar" | "engrave" | "equipmentPanel_c" | "examine" | "examine_c" | "feedback_c" | "formation" | "formation_c" | "fullScreenHUD" | "gameMenu" | "gameMenu_c" | "giftBagContent" | "giftBagsMenu" | "gmInventory" | "GMItemSheet" | "GMJournal" | "GMMetadataBox" | "GMMinimap" | "GMMoodPanel" | "GMPanelHUD" | "GMRewardPanel" | "GMSkills" | "hotBar" | "installScreen_c" | "inventorySkillPanel_c" | "itemAction" | "itemGenerator" | "itemSplitter" | "itemSplitter_c" | "journal" | "journal_c" | "journal_csp" | "loadingScreen" | "mainMenu" | "mainMenu_c" | "menuBG" | "minimap" | "minimap_c" | "mods" | "mods_c" | "monstersSelection" | "mouseIcon" | "msgBox" | "msgBox_c" | "notification" | "optionsInput" | "optionsSettings" | "optionsSettings_c" | "overhead" | "overviewMap" | "panelSelect_c" | "partyInventory" | "partyInventory_c" | "partyManagement_c" | "pause" | "peace" | "playerInfo" | "playerInfo_c" | "possessionBar" | "pyramid" | "pyramid_c" | "reputationPanel" | "reward" | "reward_c" | "roll" | "saveLoad" | "saveLoad_c" | "saving" | "serverlist" | "serverlist_c" | "skills" | "skillsSelection" | "sortBy_c" | "startTurnRequest" | "startTurnRequest_c" | "statsPanel_c" | "statusConsole" | "statusPanel" | "stickiesPanel" | "sticky" | "storyElement" | "surfacePainter" | "textDisplay" | "tooltip" | "trade" | "trade_c" | "tutorialBox" | "tutorialBox_c" | "uiCraft" | "uiFade" | "userProfile" | "vignette" | "voiceNotification_c" | "watermark" | "waypoints" | "waypoints_c" | "worldTooltip"

--- @overload fun(string:BuiltinUISWFName):integer
Ext_ClientUI.TypeID = {
	actionProgression = 0,
	addContent = 57,
	addContent_c = 81,
	areaInteract_c = 68,
	arenaResult = 125,
	book = 2,
	bottomBar_c = 59,
	buttonLayout_c = 95,
	calibrationScreen = 98,
	campaignManager = 124,
	characterAssign = 52,
	characterAssign_c = 92,
	characterCreation = 3,
	characterCreation_c = 4,
	characterSheet = 119,
	chatLog = 6,
	combatLog = 7,
	combatLog_c = 65,
	combatTurn = 8,
	connectionMenu = 33,
	connectivity_c = 34,
	containerInventory = { Default = 9, Pickpocket = 37 },
	containerInventoryGM = 143,
	contextMenu = { Default = 10, Object = 11 },
	contextMenu_c = { Default = 12, Object = 96 },
	craftPanel_c = 84,
	credits = 53,
	dialog = 14,
	dialog_c = 66,
	dummyOverhead = 15,
	encounterPanel = 105,
	enemyHealthBar = 42,
	engrave = 69,
	equipmentPanel_c = 64,
	examine = 104,
	examine_c = 67,
	feedback_c = 97,
	formation = 130,
	formation_c = 135,
	fullScreenHUD = 100,
	gameMenu = 19,
	gameMenu_c = 77,
	giftBagContent = 147,
	giftBagsMenu = 146,
	gmInventory = 126,
	GMItemSheet = 107,
	GMJournal = 139,
	GMMetadataBox = 109,
	GMMinimap = 113,
	GMMoodPanel = 108,
	GMPanelHUD = 120,
	GMRewardPanel = 131,
	GMSkills = 123,
	hotBar = 40,
	installScreen_c = 80,
	inventorySkillPanel_c = 62,
	itemAction = 86,
	itemGenerator = 106,
	itemSplitter = 21,
	itemSplitter_c = 85,
	journal = 22,
	journal_c = 70,
	journal_csp = 140,
	loadingScreen = 23,
	mainMenu = 28,
	mainMenu_c = 87, -- Still mainMenu.swf, but this is used for controllers after clicking "Options" in the gameMenu_c
	menuBG = 56,
	minimap = 30,
	minimap_c = 60,
	mods = 49,
	mods_c = 103,
	monstersSelection = 127,
	mouseIcon = 31,
	msgBox = 29,
	msgBox_c = 75,
	notification = 36,
	optionsInput = 13,
	optionsSettings = { Default = 45, Video = 45, Audio = 1, Game = 17 },
	optionsSettings_c = { Default = 91, Video = 91, Audio = 88, Game = 89 },
	overhead = 5,
	overviewMap = 112,
	panelSelect_c = 83,
	partyInventory = 116,
	partyInventory_c = 142,
	partyManagement_c = 82,
	pause = 121,
	peace = 122,
	playerInfo = 38,
	playerInfo_c = 61, --Still playerInfo.swf, but the ID is different.
	possessionBar = 110,
	pyramid = 129,
	pyramid_c = 134,
	reputationPanel = 138,
	reward = 136,
	reward_c = 137,
	roll = 118,
	saveLoad = 39,
	saveLoad_c = 74,
	saving = 99,
	serverlist = 26,
	serverlist_c = 27,
	skills = 41,
	skillsSelection = 54,
	sortBy_c = 79,
	startTurnRequest = 145,
	startTurnRequest_c = 144,
	statsPanel_c = 63,
	statusConsole = 117,
	statusPanel = 128,
	stickiesPanel = 133,
	sticky = 132,
	storyElement = 71,
	surfacePainter = 111,
	textDisplay = 43,
	tooltip = 44,
	trade = 46,
	trade_c = 73,
	tutorialBox = 55,
	tutorialBox_c = 94,
	uiCraft = 102,
	uiFade = 16,
	userProfile = 51,
	vignette = 114,
	voiceNotification_c = 93,
	watermark = 141,
	waypoints = 47,
	waypoints_c = 78,
	worldTooltip = 48,
}]]},
Ext_Types = {
	After = [[
--- Generate an ExtIdeHelpers file  
--- @param outputPath string|nil Optional path to save the generated helper file, relative to the `Documents\Larian Studios\Divinity Original Sin 2 Definitive Edition\Osiris Data` folder  
--- @param addOsi boolean|nil If true, all Osiris functions will be included in the Osi global table. This is optional, due to the possible performance cost of having so many functions  
--- @return string fileContents Returns the file contents, for use with Ext.IO.SaveFile
function Ext_Types.GenerateIdeHelpers(outputPath, addOsi) end
]]
}
}