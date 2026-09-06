#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <multyPlayer/waterSimulation.h>
#include <multyPlayer/serverChunkStorer.h>
#include <blocks.h>
#include <unordered_set>
#include <vector>

static std::unordered_set<glm::ivec2, Ivec2Hash> activeWaterChunks;
static std::unordered_map<glm::ivec2, int, Ivec2Hash> dryTickCounter;
static bool initialized = false;
static std::uint64_t lastRescanTick = 0;

// Water can flow through air and non-collidable decorative blocks
// (torches, grass, flowers, ladders, vines, fences, etc.)
static bool isWaterPassable(BlockType type)
{
	return type == BlockTypes::air || !isColidable(type);
}

static Block waterBlockOfLevel(int level)
{
	Block b;
	b.setType(BlockTypes::water);
	b.setWaterLevel(level & 7);
	return b;
}

static void setWaterNext(std::unordered_map<glm::ivec3, Block> &next, glm::ivec3 pos, int level)
{
	auto it = next.find(pos);
	Block nb = waterBlockOfLevel(level);
	if (it == next.end()) next[pos]=nb;
	else if (it->second.getType()!=BlockTypes::water) it->second=nb;
	else if (it->second.getWaterLevel() > level) it->second.setWaterLevel(level);
}

static void setAirNext(std::unordered_map<glm::ivec3, Block> &next, glm::ivec3 pos)
{
	auto it = next.find(pos);
	if (it!=next.end() && it->second.getType()==BlockTypes::water) return;
	next[pos]=Block{};
}

static void activateChunksAroundWater(glm::ivec3 pos)
{
	const int cx = divideChunk(pos.x);
	const int cz = divideChunk(pos.z);
	for(int dx=-1;dx<=1;dx++) for(int dz=-1;dz<=1;dz++) activeWaterChunks.insert({cx+dx,cz+dz});
}

void markWaterSimulationArea(glm::ivec3 blockPos){ activateChunksAroundWater(blockPos); }

static void rescanSimulationChunks(ServerChunkStorer &chunkCache)
{
	for(auto &c: chunkCache.savedChunks)
	{
		if(!c.second->otherData.withinSimulationDistance) continue;
		bool hasWater=false;
		auto &cd=c.second->chunk;
		for(int x=0;x<CHUNK_SIZE&&!hasWater;x++) for(int z=0;z<CHUNK_SIZE&&!hasWater;z++) for(int y=CHUNK_HEIGHT-1;y>=0&&!hasWater;y--) if(cd.unsafeGet(x,y,z).getType()==BlockTypes::water) hasWater=true;
		if(hasWater) activeWaterChunks.insert(c.first);
	}
}

void updateWaterSimulation(ServerChunkStorer &chunkCache, std::unordered_map<glm::ivec3, Block> &modifiedBlocks)
{
	const std::uint64_t tick = lastRescanTick + 1;
	lastRescanTick=tick;
	if(!initialized){ initialized=true; rescanSimulationChunks(chunkCache); }
	else if((tick%600)==0) rescanSimulationChunks(chunkCache);

	std::unordered_map<glm::ivec3, Block> next;
	static const glm::ivec3 dirs[4]={{1,0,0},{-1,0,0},{0,0,1},{0,0,-1}};

	for(auto it=activeWaterChunks.begin(); it!=activeWaterChunks.end(); )
	{
		const glm::ivec2 chunkPos=*it;
		SavedChunk *sc=chunkCache.getChunkOrGetNull(chunkPos.x, chunkPos.y);
		if(!sc){ it=activeWaterChunks.erase(it); dryTickCounter.erase(chunkPos); continue; }
		auto &cd=sc->chunk;
		bool chunkHadWater=false;
		const int baseX=chunkPos.x*CHUNK_SIZE;
		const int baseZ=chunkPos.y*CHUNK_SIZE;

		for(int x=0;x<CHUNK_SIZE;x++) for(int z=0;z<CHUNK_SIZE;z++) for(int y=CHUNK_HEIGHT-1;y>=0;y--)
		{
			Block &b=cd.unsafeGet(x,y,z);
			if(b.getType()!=BlockTypes::water) continue;
			chunkHadWater=true;
			const int level=b.getWaterLevel();
			const glm::ivec3 pos={baseX+x,y,baseZ+z};

			SavedChunk *belowChunk=nullptr;
			Block *below=(y>0)? chunkCache.getBlockSafeAndChunk({pos.x,y-1,pos.z}, belowChunk):nullptr;
			SavedChunk *aboveChunk=nullptr;
			Block *above=(y+1<CHUNK_HEIGHT)? chunkCache.getBlockSafeAndChunk({pos.x,y+1,pos.z}, aboveChunk):nullptr;
			bool aboveIsWater=above && above->getType()==BlockTypes::water;

			if(below && isWaterPassable(below->getType()))
			{
				if(level==0) setWaterNext(next,{pos.x,y-1,pos.z},0);
				else { setAirNext(next,pos); setWaterNext(next,{pos.x,y-1,pos.z},level); }
				if(level==0)
				{
					if(level<7) for(auto &d: dirs){ glm::ivec3 np=pos+d; SavedChunk *nc=nullptr; Block *n=chunkCache.getBlockSafeAndChunk(np, nc); if(n && isWaterPassable(n->getType())) setWaterNext(next,np,1); }
				}
				continue;
			}

			if(below && below->getType()==BlockTypes::water && below->getWaterLevel()>level)
				setWaterNext(next,{pos.x,y-1,pos.z},level);

			if(level>0 && !aboveIsWater)
			{
				int sourceCnt=0;
				for(int dx=-1;dx<=1;dx++) for(int dz=-1;dz<=1;dz++){ if(dx==0&&dz==0) continue; glm::ivec3 pp=pos+glm::ivec3(dx,0,dz); SavedChunk *cc=nullptr; Block *bn=chunkCache.getBlockSafeAndChunk(pp,cc); if(bn && bn->getType()==BlockTypes::water && bn->getWaterLevel()==0) sourceCnt++; }
				if(sourceCnt>=2){ setWaterNext(next,pos,0); continue; }
				int minNeigh=8;
				for(auto &d: dirs){ glm::ivec3 np=pos+d; SavedChunk *nc=nullptr; Block *n=chunkCache.getBlockSafeAndChunk(np,nc); if(n && n->getType()==BlockTypes::water) minNeigh=std::min(minNeigh,(int)n->getWaterLevel()); }
				if(minNeigh==8)
				{
					setAirNext(next,pos);
					continue;
				}
				int expected=minNeigh+1;
				if(expected>=8) { setAirNext(next,pos); continue; }
				if(expected!=level)
				{
					if(expected>level)
					{
						setWaterNext(next,pos,expected);
						continue;
					}
					else
					{
						setWaterNext(next,pos,expected);
					}
				}
				if(expected==8) continue;
			}

			if(level<7)
			{
				for(auto &d: dirs)
				{
					glm::ivec3 np=pos+d;
					SavedChunk *nc=nullptr;
					Block *n=chunkCache.getBlockSafeAndChunk(np,nc);
					if(n && isWaterPassable(n->getType()))
					{
						int srcCnt=0;
						for(int dx=-1;dx<=1;dx++) for(int dz=-1;dz<=1;dz++){ if(dx==0&&dz==0) continue; glm::ivec3 pp=np+glm::ivec3(dx,0,dz); SavedChunk *cc=nullptr; Block *bn=chunkCache.getBlockSafeAndChunk(pp,cc); if(bn && bn->getType()==BlockTypes::water && bn->getWaterLevel()==0) srcCnt++; }
						if(srcCnt>=2) setWaterNext(next,np,0);
						else setWaterNext(next,np,level+1);
					}
					else if(n && n->getType()==BlockTypes::water && n->getWaterLevel()>level+1)
					{
						setWaterNext(next,np,level+1);
					}
				}
			}
			else if(level==7)
			{
				// edge of flow decays: check if isolated will be removed by above logic, don't spread
			}
		}

		if(chunkHadWater){ dryTickCounter[*it]=0; it++; }
		else { int dry=dryTickCounter.find(*it)!=dryTickCounter.end()? dryTickCounter[*it]:0; if(dry>=40){ it=activeWaterChunks.erase(it); dryTickCounter.erase(chunkPos);} else { dryTickCounter[*it]=dry+1; it++; } }
	}

	for(auto &change: next)
	{
		const glm::ivec3 &pos=change.first;
		const Block &blk=change.second;
		SavedChunk *sc=nullptr;
		Block *b=chunkCache.getBlockSafeAndChunk(pos, sc);
		if(!b||!sc) continue;
		if(b->typeAndFlags==blk.typeAndFlags && b->lightLevel==blk.lightLevel) continue;
		*b=blk;
		sc->otherData.dirty=true;
		modifiedBlocks[pos]=blk;
		activateChunksAroundWater(pos);
	}
}
