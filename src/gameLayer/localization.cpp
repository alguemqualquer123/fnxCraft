#include <gameLayer/localization.h>
#include <gameplay/items.h>
#include <string.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <safeSave.h>

static Language currentLanguage = Language::English;

struct LangPair
{
	const char* english;
	const char* portuguese;
};

// Helper to pick the right string based on current language
static const char* pick(const LangPair& p)
{
	if (currentLanguage == Language::Portuguese_BR)
		return p.portuguese;
	return p.english;
}

// All localization pairs
static const LangPair s_JoinGame        = {"Join game", "Entrar no jogo"};
static const LangPair s_Settings        = {"Settings", "Configuracoes"};
static const LangPair s_Play            = {"Play", "Jogar"};
static const LangPair s_Exit            = {"Exit", "Sair"};

static const LangPair s_GameMenu        = {"Game Menu", "Menu do Jogo"};
static const LangPair s_BackToGame      = {"Back to Game", "Voltar ao Jogo"};
static const LangPair s_BackToMenu      = {"Back to Menu", "Voltar ao Menu"};
static const LangPair s_Chat            = {"Chat", "Chat"};
static const LangPair s_DeathMessage    = {"You died!", "Voce morreu!"};
static const LangPair s_Respawn         = {"Respawn", "Renasc"};

static const LangPair s_FPS             = {"FPS: ", "FPS: "};
static const LangPair s_IP              = {"IP: ", "IP: "};
static const LangPair s_CouldntJoin     = {"Couldn't join server", "Nao foi possivel entrar no servidor"};
static const LangPair s_CouldntFind     = {"Couldn't find server.", "Servidor nao encontrado."};
static const LangPair s_EnterIP         = {"Enter IP...", "Digite o IP..."};

static const LangPair s_Rendering       = {"Rendering", "Renderizacao"};
static const LangPair s_Volume          = {"Volume", "Volume"};
static const LangPair s_AudioSettings   = {"Audio Settings", "Configuracoes de Audio"};
static const LangPair s_TexturesPacks   = {"Texture Packs", "Pacotes de Textura"};
static const LangPair s_Skin            = {"Skin", "Pele"};
static const LangPair s_ChangeSkin      = {"Change Skin", "Trocar Pele"};
static const LangPair s_Language        = {"Language", "Idioma"};

static const LangPair s_RenderingSet    = {"Rendering Settings...", "Configuracoes de Renderizacao..."};
static const LangPair s_ViewDistance     = {"View Distance", "Distancia de Visao"};
static const LangPair s_LodStrength     = {"Lod Strength", "Forca do LOD"};
static const LangPair s_Tonemapper      = {"Tonemapper", "Mapeamento de Tom"};
static const LangPair s_Shadows         = {"Shadows", "Sombras"};
static const LangPair s_WaterType       = {"Water type", "Tipo de agua"};
static const LangPair s_WaterSett       = {"Water settings...", "Configuracoes de agua..."};
static const LangPair s_WaterColor      = {"Water color", "Cor da agua"};
static const LangPair s_UnderWaterColor = {"Under water color", "Cor debaixo d'agua"};
static const LangPair s_UnderFogStr     = {"Underwater Fog strength", "Forca da nevoa subaquatica"};
static const LangPair s_UnderFogDist    = {"Underwater Fog Distance", "Distancia da nevoa subaquatica"};
static const LangPair s_UnderFogGrad    = {"Underwater Fog Gradient", "Gradiente da nevoa subaquatica"};
static const LangPair s_PBR             = {"PBR", "PBR"};
static const LangPair s_SSR             = {"SSR", "SSR"};
static const LangPair s_SSRSett         = {"SSR Settings...", "Configuracoes SSR..."};
static const LangPair s_MaxLights       = {"Max Lights", "Maximo de Luzes"};
static const LangPair s_MaxLightsS      = {"Max l.", "Max lu."};
static const LangPair s_UseLights       = {"Use Lights", "Usar Luzes"};
static const LangPair s_LightsSett      = {"Lights settings...", "Configuracoes de luzes..."};
static const LangPair s_LightsStr       = {"Lights Strength", "Forca das Luzes"};
static const LangPair s_LightsStrS      = {"Lights str.", "Forca lu."};
static const LangPair s_Bloom           = {"Bloom", "Brilho"};
static const LangPair s_BloomSett       = {"Bloom settings...", "Configuracoes de brilho..."};
static const LangPair s_BloomMult       = {"Bloom Multiplier", "Multiplicador de Brilho"};
static const LangPair s_BloomThresh     = {"Bloom Threshold", "Limiar de Brilho"};
static const LangPair s_FXAA            = {"FXAA", "FXAA"};
static const LangPair s_Exposure        = {"Exposure", "Exposicao"};
static const LangPair s_FogGrad         = {"Fog Gradient", "Gradiente de Nevoa"};
static const LangPair s_FogGradTip      = {"How thick the fog is", "Espessura da nevoa"};
static const LangPair s_ColorPost       = {"Color post processing...", "Pos-processamento de cor..."};
static const LangPair s_ResetSettings   = {"Reset to Default", "Restaurar Padrao"};
static const LangPair s_Vignette        = {"Vignette", "Vinheta"};
static const LangPair s_Saturation      = {"Saturation", "Saturacao"};
static const LangPair s_Vibrance        = {"Vibrance", "Vibracao"};
static const LangPair s_Gamma           = {"Gamma", "Gamma"};
static const LangPair s_ShadowBoost     = {"Shadow Boost", "Realce de Sombras"};
static const LangPair s_HighlightBoost  = {"Highlight Boost", "Realce de Destaques"};
static const LangPair s_Lift            = {"Lift", "Elevacao"};
static const LangPair s_Gain            = {"Gain", "Ganho"};
static const LangPair s_ChunkThreads    = {"Chunk Building Threads", "Threads de Construcao de Blocos"};
static const LangPair s_ShadowsTip      = {"Lower values improve performance", "Valores menores melhoram desempenho"};

static const LangPair s_MasterVol       = {"Master Volume", "Volume Principal"};
static const LangPair s_MusicVol        = {"Music Volume", "Volume da Musica"};
static const LangPair s_UIVol           = {"UI Volume", "Volume da Interface"};
static const LangPair s_SoundsVol       = {"Sounds Volume", "Volume dos Efeitos"};

static const LangPair s_SelectWorld     = {"Select World", "Selecionar Mundo"};
static const LangPair s_CreateWorld      = {"Create New World", "Criar Novo Mundo"};

static const LangPair s_AreYouSureExit  = {"Are you sure you want to exit?", "Tem certeza que deseja sair?"};
static const LangPair s_AreYouSureLeave = {"Are you sure you want to leave the world?", "Tem certeza que deseja sair do mundo?"};
static const LangPair s_Yes             = {"Yes", "Sim"};
static const LangPair s_No              = {"No", "Nao"};

static const LangPair s_SurvivalMode    = {"Survival Mode", "Modo Sobrevivencia"};
static const LangPair s_CreativeMode    = {"Creative Mode", "Modo Criativo"};

static const LangPair s_CreateGame      = {"Create game", "Criar jogo"};
static const LangPair s_CouldntCreate   = {"Couldn't create server", "Nao foi possivel criar o servidor"};
static const LangPair s_ExitToDesktop   = {"Exit to Desktop", "Sair do Jogo"};
static const LangPair s_Options         = {"Options", "Opcoes"};
static const LangPair s_Invent          = {"Inventory", "Inventario"};
static const LangPair s_Craft           = {"Crafting", "Fabricacao"};
static const LangPair s_Drop            = {"Drop", "Largar"};
static const LangPair s_Place           = {"Place", "Colocar"};

static const LangPair s_Saving          = {"Saving world...", "Salvando mundo..."};
static const LangPair s_Loading         = {"Loading world...", "Carregando mundo..."};
static const LangPair s_SavingChunks    = {"Saving chunks...", "Salvando blocos..."};
static const LangPair s_GenChunks       = {"Generating chunks...", "Gerando blocos..."};

static const LangPair s_Connected       = {"Connected!", "Conectado!"};
static const LangPair s_Disconnected    = {"Disconnected.", "Desconectado."};
static const LangPair s_Connecting      = {"Connecting...", "Conectando..."};
static const LangPair s_ServerFull      = {"Server Full", "Servidor Cheio"};
static const LangPair s_InvalidIP       = {"Invalid IP", "IP Invalido"};

// Language & common
static const LangPair s_Ok              = {"OK", "OK"};
static const LangPair s_Close           = {"Close", "Fechar"};
static const LangPair s_Back            = {"Back", "Voltar"};
static const LangPair s_Save            = {"Save", "Salvar"};
static const LangPair s_Delete          = {"Delete", "Excluir"};
static const LangPair s_Error           = {"Error", "Erro"};
static const LangPair s_Warning         = {"Warning", "Aviso"};
static const LangPair s_Success         = {"Success", "Sucesso"};

// Survival status
static const LangPair s_Hunger          = {"Hunger", "Fome"};
static const LangPair s_Thirst          = {"Thirst", "Sede"};
static const LangPair s_Starving        = {"Starving", "Passando Fome"};
static const LangPair s_Dehydrated      = {"Dehydrated", "Desidratado"};
static const LangPair s_Drowning        = {"Drowning", "Afogando"};

// Item names & descriptions
static const LangPair s_RawMeat         = {"Raw Meat", "Carne Crua"};
static const LangPair s_CookedMeat      = {"Cooked Meat", "Carne Cozida"};
static const LangPair s_Bread           = {"Bread", "Pao"};
static const LangPair s_Stew            = {"Stew", "Ensopado"};
static const LangPair s_WaterBottle     = {"Water Bottle", "Garrafa de Agua"};
static const LangPair s_Juice           = {"Juice", "Suco"};
static const LangPair s_Milk            = {"Milk", "Leite"};
static const LangPair s_Coffee          = {"Coffee", "Cafe"};
static const LangPair s_Tea             = {"Tea", "Cha"};
static const LangPair s_RawFish         = {"Raw Fish", "Peixe Cru"};
static const LangPair s_CookedFish      = {"Cooked Fish", "Peixe Cozido"};
static const LangPair s_ChickenSoup     = {"Chicken Soup", "Sopa de Frango"};
static const LangPair s_CookedChicken   = {"Cooked Chicken", "Frango Cozido"};
static const LangPair s_BakedPotato     = {"Baked Potato", "Batata Assada"};
static const LangPair s_RoastedCorn     = {"Roasted Corn", "Milho Assado"};
static const LangPair s_Cheese          = {"Cheese", "Queijo"};
static const LangPair s_ApplePie        = {"Apple Pie", "Torta de Maca"};
static const LangPair s_BoneMeal        = {"Bone Meal", "Fosfato"};
static const LangPair s_Compost         = {"Compost", "Composto"};
static const LangPair s_WateringCan     = {"Watering Can", "Regador"};
static const LangPair s_FishingRod      = {"Fishing Rod", "Vara de Pesca"};
static const LangPair s_FishSpawnEgg    = {"Fish Spawn Egg", "Ovo de Peixe"};
static const LangPair s_Seeds           = {"Seeds", "Sementes"};
static const LangPair s_WheatSeeds      = {"Wheat Seeds", "Sementes de Trigo"};
static const LangPair s_PotatoSeeds     = {"Potato Seeds", "Sementes de Batata"};
static const LangPair s_CornSeeds       = {"Corn Seeds", "Sementes de Milho"};
static const LangPair s_CarrotSeeds     = {"Carrot Seeds", "Sementes de Cenoura"};
static const LangPair s_Fertilizer      = {"Fertilizer", "Fertilizante"};
static const LangPair s_Default         = {"Default", "Padrao"};
static const LangPair s_LanguageName    = {"Language", "Idioma"};

// Item names, indexed by (ItemTypes - ItemsStartPoint), keep in the same order as the enum!
static const LangPair s_itemNames[] =
{
	{"stick", "Vara"},
	{"cloth", "Pano"},
	{"fang", "Presa"},
	{"bone", "Osso"},

	{"copperIngot", "Barra de Cobre"},
	{"leadIngot", "Barra de Chumbo"},
	{"ironIngot", "Barra de Ferro"},
	{"silverIngot", "Barra de Prata"},
	{"goldIngot", "Barra de Ouro"},

	{"copper pickaxe", "Picareta de Cobre"},
	{"copper axe", "Machado de Cobre"},
	{"copper shovel", "Pa de Cobre"},
	{"lead pickaxe", "Picareta de Chumbo"},
	{"lead axe", "Machado de Chumbo"},
	{"lead shovel", "Pa de Chumbo"},
	{"iron pickaxe", "Picareta de Ferro"},
	{"iron axe", "Machado de Ferro"},
	{"iron shovel", "Pa de Ferro"},
	{"silver pickaxe", "Picareta de Prata"},
	{"silver axe", "Machado de Prata"},
	{"silver shovel", "Pa de Prata"},
	{"gold pickaxe", "Picareta de Ouro"},
	{"gold axe", "Machado de Ouro"},
	{"gold shovel", "Pa de Ouro"},

	{"copper sword", "Espada de Cobre"},
	{"lead sword", "Espada de Chumbo"},
	{"iron sword", "Espada de Ferro"},
	{"silver sword", "Espada de Prata"},
	{"gold sword", "Espada de Ouro"},

	{"trainingScythe", "Foice de Treino"},
	{"trainingSword", "Espada de Treino"},
	{"trainingWarHammer", "Martelo de Guerra de Treino"},
	{"trainingSpear", "Lanca de Treino"},
	{"trainingKnife", "Faca de Treino"},
	{"trainingBattleAxe", "Machado de Batalha de Treino"},

	{"Copper War Hammer", "Martelo de Guerra de Cobre"},
	{"Copper Spear", "Lanca de Cobre"},
	{"Copper Knife", "Faca de Cobre"},
	{"Copper Battle Axe", "Machado de Batalha de Cobre"},
	{"Lead WarHammer", "Martelo de Guerra de Chumbo"},
	{"Lead Spear", "Lanca de Chumbo"},
	{"Lead Knife", "Faca de Chumbo"},
	{"Lead BattleAxe", "Machado de Batalha de Chumbo"},
	{"Iron War Hammer", "Martelo de Guerra de Ferro"},
	{"Iron Spear", "Lanca de Ferro"},
	{"Iron Knife", "Faca de Ferro"},
	{"Iron Battle Axe", "Machado de Batalha de Ferro"},
	{"silver WarHammer", "Martelo de Guerra de Prata"},
	{"silver Spear", "Lanca de Prata"},
	{"silver Knife", "Faca de Prata"},
	{"silver BattleAxe", "Machado de Batalha de Prata"},
	{"gold WarHammer", "Martelo de Guerra de Ouro"},
	{"gold Spear", "Lanca de Ouro"},
	{"gold Knife", "Faca de Ouro"},
	{"gold BattleAxe", "Machado de Batalha de Ouro"},

	{"zombie spawn egg", "Ovo de Zumbi"},
	{"pig spawn egg", "Ovo de Porco"},
	{"cat spawn egg", "Ovo de Gato"},
	{"goblin spawn egg", "Ovo de Goblin"},
	{"posessed scarecrow spawn egg", "Ovo de Espantalho Possuido"},
	{"skeleton spawn egg", "Ovo de Esqueleto"},
	{"enderling spawn egg", "Ovo de Enderling"},
	{"bee spawn egg", "Ovo de Abelha"},
	{"queen bee spawn egg", "Ovo de Abelha Rainha"},
	{"creeper spawn egg", "Ovo de Creeper"},
	{"slime spawn egg", "Ovo de Slime"},
	{"cave spider spawn egg", "Ovo de Aranha de Caverna"},
	{"crystal bat spawn egg", "Ovo de Morcego Cristal"},
	{"capybara chef spawn egg", "Ovo de Capivara Chef"},
	{"river guardian spawn egg", "Ovo de Guardiao do Rio"},
	{"tree ent spawn egg", "Ovo de Ent Arboreo"},
	{"nomad trader spawn egg", "Ovo de Vendedor Nomade"},
	{"mimic chest spawn egg", "Ovo de Mimico"},
	{"light fairy spawn egg", "Ovo de Fada da Luz"},
	{"armored boar spawn egg", "Ovo de Javali Couracado"},
	{"sand serpent spawn egg", "Ovo de Serpente da Areia"},
	{"mist ghost spawn egg", "Ovo de Fantasma da Nevoa"},
	{"hermit crab spawn egg", "Ovo de Caranguejo Eremita"},
	{"honey bear spawn egg", "Ovo de Urso Melifero"},
	{"lava slug spawn egg", "Ovo de Lesma de Lava"},
	{"crystal sentinel spawn egg", "Ovo de Sentinela de Cristal"},
	{"blacksmith villager spawn egg", "Ovo de Aldeao Ferreiro"},
	{"herbalist villager spawn egg", "Ovo de Aldea Herborista"},
	{"skeleton pirate spawn egg", "Ovo de Pirata Esqueleto"},
	{"juvenile dragon spawn egg", "Ovo de Dragao Juvenil"},
	{"crystal golem spawn egg", "Ovo de Golem de Cristal"},
	{"hydra spawn egg", "Ovo de Hidra"},

	{"apple", "Maca"},
	{"blackBerrie", "Amora"},
	{"blueBerrie", "Mirtilo"},
	{"cherries", "Cerejas"},
	{"chilliPepper", "Pimenta"},
	{"cocconut", "Coco"},
	{"grapes", "Uvas"},
	{"lime", "Limao"},
	{"peach", "Pessego"},
	{"pinapple", "Abacaxi"},
	{"strawberry", "Morango"},

	{"Apple Pie", "Torta de Maca"},

	{"leather boots", "Botas de Couro"},
	{"leather ChestPlate", "Peitoral de Couro"},
	{"leather cap", "Capuz de Couro"},
	{"copper boots", "Botas de Cobre"},
	{"copper ChestPlate", "Peitoral de Cobre"},
	{"copper cap", "Capuz de Cobre"},
	{"lead boots", "Botas de Chumbo"},
	{"lead ChestPlate", "Peitoral de Chumbo"},
	{"lead cap", "Capuz de Chumbo"},
	{"iron boots", "Botas de Ferro"},
	{"iron ChestPlate", "Peitoral de Ferro"},
	{"iron cap", "Capuz de Ferro"},
	{"silver boots", "Botas de Prata"},
	{"silver ChestPlate", "Peitoral de Prata"},
	{"silver cap", "Capuz de Prata"},
	{"gold boots", "Botas de Ouro"},
	{"gold ChestPlate", "Peitoral de Ouro"},
	{"gold cap", "Capuz de Ouro"},

	{"soap", "Sabao"},
	{"white paint", "Tinta Branca"},
	{"lightGray paint", "Tinta Cinza Claro"},
	{"darkGray paint", "Tinta Cinza Escuro"},
	{"black paint", "Tinta Preta"},
	{"brown paint", "Tinta Marrom"},
	{"red paint", "Tinta Vermelha"},
	{"orange paint", "Tinta Laranja"},
	{"yellow paint", "Tinta Amarela"},
	{"lime paint", "Tinta Verde Clara"},
	{"green paint", "Tinta Verde"},
	{"turqoise paint", "Tinta Turquesa"},
	{"cyan paint", "Tinta Ciano"},
	{"blue paint", "Tinta Azul"},
	{"purple paint", "Tinta Roxa"},
	{"magenta paint", "Tinta Magenta"},
	{"pink paint", "Tinta Rosa"},

	{"copper coin", "Moeda de Cobre"},
	{"silver coin", "Moeda de Prata"},
	{"gold coin", "Moeda de Ouro"},
	{"diamond coin", "Moeda de Diamante"},

	{"wooden arrow", "Flecha de Madeira"},
	{"flaming arrow", "Flecha de Fogo"},
	{"goblin arrow", "Flecha de Goblin"},
	{"bone arrow", "Flecha de Osso"},
	{"wheat", "Trigo"},

	{"Healing Potion", "Pocao de Cura"},
	{"Mana Potion", "Pocao de Mana"},

	{"Fire Resistance Potion", "Pocao de Resistencia ao Fogo"},
	{"JumpBoost Potion", "Pocao de Super Salto"},
	{"Luck Potion", "Pocao de Sorte"},
	{"Mana Regeneration Potion", "Pocao de Regeneracao de Mana"},
	{"Poison Potion", "Pocao de Veneno"},
	{"Recall Potion", "Pocao de Retorno"},
	{"Regeneration Potion", "Pocao de Regeneracao"},
	{"Shielding Potion", "Pocao de Protecao"},
	{"Speed Potion", "Pocao de Velocidade"},
	{"Stealth Potion", "Pocao de Furtividade"},
	{"Strength Potion", "Pocao de Forca"},
	{"Venomus Potion", "Pocao Venenosa"},
	{"Bad Luck Potion", "Pocao de Azar"},

	{"Gum Box", "Caixa de Goma"},
	{"Bandage", "Atadura"},
	{"Fruit Peeler", "Descascador de Frutas"},
	{"Paw Keychain", "Chaveiro de Pata"},
	{"Vitamins", "Vitaminas"},

	{"Fish Spawn Egg", "Ovo de Peixe"},
	{"Cooked Fish", "Peixe Cozido"},
	{"Raw Fish", "Peixe Cru"},
	{"Fishing Rod", "Vara de Pesca"},

	{"Raw Meat", "Carne Crua"},
	{"Cooked Meat", "Carne Cozida"},
	{"Bread", "Pao"},
	{"Stew", "Ensopado"},
	{"Baked Potato", "Batata Assada"},
	{"Roasted Corn", "Milho Assado"},
	{"Cheese", "Queijo"},
	{"Cooked Chicken", "Frango Cozido"},
	{"Chicken Soup", "Sopa de Frango"},

	{"Water Bottle", "Garrafa de Agua"},
	{"Juice", "Suco"},
	{"Milk", "Leite"},
	{"Coffee", "Cafe"},
	{"Tea", "Cha"},

	{"Seeds", "Sementes"},
	{"Wheat Seeds", "Sementes de Trigo"},
	{"Potato Seeds", "Sementes de Batata"},
	{"Corn Seeds", "Sementes de Milho"},
	{"Carrot Seeds", "Sementes de Cenoura"},
	{"Bone Meal", "Farinha de Osso"},
	{"Fertilizer", "Fertilizante"},
	{"Watering Can", "Regador"},
	{"Compost", "Composto"},

	{"Wooden Bow", "Arco de Madeira"},
	{"Copper Bow", "Arco de Cobre"},
	{"Lead Bow", "Arco de Chumbo"},
	{"Iron Bow", "Arco de Ferro"},
	{"Silver Bow", "Arco de Prata"},
	{"Gold Bow", "Arco de Ouro"},
	{"Goblin Bow", "Arco Goblin"},
};

// ---- Getters ----
const char *loc_ItemName(int itemId)
{
	if (itemId >= ItemsStartPoint && itemId < lastItem) {
		size_t idx = itemId - ItemsStartPoint;
		if(idx < sizeof(s_itemNames)/sizeof(s_itemNames[0])) return pick(s_itemNames[idx]);
		return pick(s_Default);
	}
	return pick(s_Default);
}

// ---- Getters ----
const char *loc_JoinGame()        { return pick(s_JoinGame); }
const char *loc_Settings()        { return pick(s_Settings); }
const char *loc_Play()            { return pick(s_Play); }
const char *loc_Exit()            { return pick(s_Exit); }

const char *loc_GameMenu()        { return pick(s_GameMenu); }
const char *loc_BackToGame()      { return pick(s_BackToGame); }
const char *loc_BackToMenu()      { return pick(s_BackToMenu); }
const char *loc_Chat()            { return pick(s_Chat); }
const char *loc_DeathMessage()    { return pick(s_DeathMessage); }
const char *loc_Respawn()         { return pick(s_Respawn); }

const char *loc_FPS()             { return pick(s_FPS); }
const char *loc_IP()              { return pick(s_IP); }
const char *loc_CouldntJoinServer() { return pick(s_CouldntJoin); }
const char *loc_EnterIP()         { return pick(s_EnterIP); }

const char *loc_Rendering()       { return pick(s_Rendering); }
const char *loc_Volume()          { return pick(s_Volume); }
const char *loc_AudioSettings()   { return pick(s_AudioSettings); }
const char *loc_TexturesPacks()   { return pick(s_TexturesPacks); }
const char *loc_Skin()            { return pick(s_Skin); }
const char *loc_ChangeSkin()      { return pick(s_ChangeSkin); }
const char *loc_Language()        { return pick(s_Language); }
const char *loc_LanguageName()    { return pick(s_LanguageName); }

const char *loc_RenderingSettings() { return pick(s_RenderingSet); }
const char *loc_ViewDistance()     { return pick(s_ViewDistance); }
const char *loc_LodStrength()     { return pick(s_LodStrength); }
const char *loc_Tonemapper()      { return pick(s_Tonemapper); }
const char *loc_Shadows()         { return pick(s_Shadows); }
const char *loc_WaterType()       { return pick(s_WaterType); }
const char *loc_WaterSettings()   { return pick(s_WaterSett); }
const char *loc_WaterColor()      { return pick(s_WaterColor); }
const char *loc_UnderWaterColor() { return pick(s_UnderWaterColor); }
const char *loc_UnderwaterFogStrength()  { return pick(s_UnderFogStr); }
const char *loc_UnderwaterFogDistance()  { return pick(s_UnderFogDist); }
const char *loc_UnderwaterFogGradient()  { return pick(s_UnderFogGrad); }
const char *loc_PBR()             { return pick(s_PBR); }
const char *loc_SSR()             { return pick(s_SSR); }
const char *loc_SSRSettings()     { return pick(s_SSRSett); }
const char *loc_MaxLights()       { return pick(s_MaxLights); }
const char *loc_MaxLightsShort()  { return pick(s_MaxLightsS); }
const char *loc_UseLights()       { return pick(s_UseLights); }
const char *loc_LightsSettings()  { return pick(s_LightsSett); }
const char *loc_LightsStrength()  { return pick(s_LightsStr); }
const char *loc_LightsStrengthShort() { return pick(s_LightsStrS); }
const char *loc_Bloom()           { return pick(s_Bloom); }
const char *loc_BloomSettings()   { return pick(s_BloomSett); }
const char *loc_BloomMultiplier() { return pick(s_BloomMult); }
const char *loc_BloomThreshold()  { return pick(s_BloomThresh); }
const char *loc_FXAA()            { return pick(s_FXAA); }
const char *loc_Exposure()        { return pick(s_Exposure); }
const char *loc_FogGradient()     { return pick(s_FogGrad); }
const char *loc_FogGradientTooltip() { return pick(s_FogGradTip); }
const char *loc_ColorPostProcessing() { return pick(s_ColorPost); }
const char *loc_ResetSettings()   { return pick(s_ResetSettings); }
const char *loc_Vignette()        { return pick(s_Vignette); }
const char *loc_Saturation()      { return pick(s_Saturation); }
const char *loc_Vibrance()        { return pick(s_Vibrance); }
const char *loc_Gamma()           { return pick(s_Gamma); }
const char *loc_ShadowBoost()     { return pick(s_ShadowBoost); }
const char *loc_HighlightBoost()  { return pick(s_HighlightBoost); }
const char *loc_Lift()            { return pick(s_Lift); }
const char *loc_Gain()            { return pick(s_Gain); }
const char *loc_ChunkBuildingThreads() { return pick(s_ChunkThreads); }
const char *loc_ShadowsPerformanceTip() { return pick(s_ShadowsTip); }

const char *loc_MasterVolume()    { return pick(s_MasterVol); }
const char *loc_MusicVolume()     { return pick(s_MusicVol); }
const char *loc_UIVolume()        { return pick(s_UIVol); }
const char *loc_SoundsVolume()    { return pick(s_SoundsVol); }

const char *loc_SelectWorld()     { return pick(s_SelectWorld); }
const char *loc_CreateNewWorld()  { return pick(s_CreateWorld); }

const char *loc_AreYouSureExit()  { return pick(s_AreYouSureExit); }
const char *loc_AreYouSureLeave() { return pick(s_AreYouSureLeave); }
const char *loc_Yes()             { return pick(s_Yes); }
const char *loc_No()              { return pick(s_No); }

const char *loc_SurvivalMode()    { return pick(s_SurvivalMode); }
const char *loc_CreativeMode()    { return pick(s_CreativeMode); }

// ---- Internal helpers ----
Language &getCurrentLanguage()
{
	return currentLanguage;
}

void setCurrentLanguage(Language lang)
{
	currentLanguage = lang;
}

const char *getLanguageName(Language lang)
{
	switch (lang)
	{
	case Language::English:
		return "English";
	case Language::Portuguese_BR:
		return "Portugues (BR)";
	default:
		return "Unknown";
	}
}

static std::string getLanguageSettingsPath()
{
	return "playerSettings/language";
}

void saveLanguageSettings()
{
	std::string path = getLanguageSettingsPath();
	std::filesystem::create_directories("playerSettings");

	int langValue = static_cast<int>(currentLanguage);
	std::string data = std::to_string(langValue);

	sfs::writeEntireFile(data.c_str(), data.size(), path.c_str());
}

void loadLanguageSettings()
{
	std::string path = getLanguageSettingsPath();

	if (!std::filesystem::exists(path))
	{
		currentLanguage = Language::English;
		return;
	}

	std::vector<char> data;
	sfs::Errors err = sfs::readEntireFile(data, path.c_str());
	if (err != sfs::noError || data.empty())
	{
		currentLanguage = Language::English;
		return;
	}

	std::string str(data.begin(), data.end());
	try
	{
		int langValue = std::stoi(str);
		if (langValue >= 0 && langValue < static_cast<int>(Language::LanguageCount))
			currentLanguage = static_cast<Language>(langValue);
	}
	catch (...)
	{
		currentLanguage = Language::English;
	}
}


