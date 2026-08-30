#pragma once
#include <glm/vec3.hpp>
#include <unordered_map>
#include <multyPlayer/serverChunkStorer.h>

//server-side water simulation: flows down and spreads horizontally with
//decreasing levels (0 = full/source .. 7 = last thin layer), MC style.
//modifiedBlocks receives all block changes so the caller can broadcast them.
void updateWaterSimulation(ServerChunkStorer &chunkCache,
	std::unordered_map<glm::ivec3, Block> &modifiedBlocks);

//make sure the simulation processes the area around blockPos (called when a
//player places/removes water directly, e.g. with a bucket).
void markWaterSimulationArea(glm::ivec3 blockPos);