#include <type_traits>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // for glm::make_mat4
#include <glm/gtx/quaternion.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>
#include <rendering/model.h>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <fstream>
#include <filesystem>
#include <map>
#include <set>
#include <blocks.h>
#include <unordered_map>
#include <magic_enum.hpp>

glm::mat4 aiToGlm(const aiMatrix4x4 &matrix)
{
	return glm::make_mat4(&matrix.a1); // a1 represents the first element of the matrix
}

const aiNode *findNodeContainingMesh(const aiNode *node, const aiMesh *mesh)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		if (node->mMeshes[i] == mesh->mPrimitiveTypes)
		{
			return node;
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		const aiNode *foundNode = findNodeContainingMesh(node->mChildren[i], mesh);
		if (foundNode)
			return foundNode;
	}

	return nullptr;
}


bool areStringsSameToLower(const char *a, const char *b)
{
	int i = 0;
	while (a[i] != 0 && b[i] != 0)
	{
		if (tolower(a[i]) != tolower(b[i]))
		{
			return false;
		}
		
		i++;
	}
	if (a[i] == 0 && b[i] == 0) { return true; }
	return false;
}


static void buildSteveModel(Model &model)
{
	struct Data{ glm::vec3 position; glm::vec3 normal; glm::vec2 uv; short boneIndex; short textureIndex; };
	std::vector<Data> vertexes; vertexes.reserve(400);
	std::vector<unsigned int> indices; indices.reserve(400);
	auto addCube = [&](glm::vec3 origin, glm::vec3 size, glm::vec2 uvMap[6][4], short boneIdx, short texIdx){
		glm::vec3 c0 = origin;
		glm::vec3 c1 = origin + glm::vec3(size.x,0,0);
		glm::vec3 c2 = origin + glm::vec3(size.x,0,size.z);
		glm::vec3 c3 = origin + glm::vec3(0,0,size.z);
		glm::vec3 c4 = origin + glm::vec3(0,size.y,0);
		glm::vec3 c5 = origin + glm::vec3(size.x,size.y,0);
		glm::vec3 c6 = origin + glm::vec3(size.x,size.y,size.z);
		glm::vec3 c7 = origin + glm::vec3(0,size.y,size.z);
		struct Face{ glm::vec3 a,b,c,d; glm::vec3 n; glm::vec2 *uv; };
		glm::vec3 nBottom(0,-1,0), nTop(0,1,0), nFront(0,0,1), nBack(0,0,-1), nRight(1,0,0), nLeft(-1,0,0);
		auto pushFace = [&](glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 n, glm::vec2 uv[4]){
			unsigned int base = vertexes.size();
			Data v0{p0,n,uv[0],boneIdx,texIdx}; Data v1{p1,n,uv[1],boneIdx,texIdx}; Data v2{p2,n,uv[2],boneIdx,texIdx}; Data v3{p3,n,uv[3],boneIdx,texIdx};
			vertexes.push_back(v0); vertexes.push_back(v1); vertexes.push_back(v2); vertexes.push_back(v3);
			indices.push_back(base+0); indices.push_back(base+1); indices.push_back(base+2);
			indices.push_back(base+2); indices.push_back(base+3); indices.push_back(base+0);
		};
		pushFace(c0,c1,c2,c3,nBottom,uvMap[0]);
		pushFace(c4,c7,c6,c5,nTop,uvMap[1]);
		pushFace(c3,c2,c6,c7,nFront,uvMap[2]);
		pushFace(c1,c0,c4,c5,nBack,uvMap[3]);
		pushFace(c1,c5,c6,c2,nRight,uvMap[4]);
		pushFace(c0,c3,c7,c4,nLeft,uvMap[5]);
	};
	auto uvRect = [](float x0,float y0,float x1,float y1)->std::array<glm::vec2,4>{
		float u0=x0/64.f, v0=1.f - y1/64.f, u1=x1/64.f, v1=1.f - y0/64.f;
		return {glm::vec2(u0,v0), glm::vec2(u1,v0), glm::vec2(u1,v1), glm::vec2(u0,v1)};
	};
	const float s=1.f/16.f;
	float infHat=0.5f*s, infLayer=0.25f*s;
	auto makeUVs = [&](int part)->std::array<std::array<glm::vec2,4>,6>{
		std::array<std::array<glm::vec2,4>,6> r;
		if(part==0){
			auto t=uvRect(8,0,16,8); r[1]={t[0],t[1],t[2],t[3]};
			auto b=uvRect(16,0,24,8); r[0]={b[0],b[1],b[2],b[3]};
			auto f=uvRect(8,8,16,16); r[2]={f[0],f[1],f[2],f[3]};
			auto bk=uvRect(24,8,32,16); r[3]={bk[0],bk[1],bk[2],bk[3]};
			auto ri=uvRect(0,8,8,16); r[5]={ri[0],ri[1],ri[2],ri[3]};
			auto le=uvRect(16,8,24,16); r[4]={le[0],le[1],le[2],le[3]};
		}else if(part==1){
			auto t=uvRect(20,16,28,20); r[1]={t[0],t[1],t[2],t[3]};
			auto b=uvRect(28,16,36,20); r[0]={b[0],b[1],b[2],b[3]};
			auto f=uvRect(20,20,28,32); r[2]={f[0],f[1],f[2],f[3]};
			auto bk=uvRect(32,20,40,32); r[3]={bk[0],bk[1],bk[2],bk[3]};
			auto ri=uvRect(16,20,20,32); r[5]={ri[0],ri[1],ri[2],ri[3]};
			auto le=uvRect(28,20,32,32); r[4]={le[0],le[1],le[2],le[3]};
		}else if(part==2){
			auto t=uvRect(4,16,8,20); r[1]={t[0],t[1],t[2],t[3]};
			auto b=uvRect(8,16,12,20); r[0]={b[0],b[1],b[2],b[3]};
			auto f=uvRect(4,20,8,32); r[2]={f[0],f[1],f[2],f[3]};
			auto bk=uvRect(12,20,16,32); r[3]={bk[0],bk[1],bk[2],bk[3]};
			auto ri=uvRect(0,20,4,32); r[5]={ri[0],ri[1],ri[2],ri[3]};
			auto le=uvRect(8,20,12,32); r[4]={le[0],le[1],le[2],le[3]};
		}else if(part==3){
			auto t=uvRect(44,16,48,20); r[1]={t[0],t[1],t[2],t[3]};
			auto b=uvRect(48,16,52,20); r[0]={b[0],b[1],b[2],b[3]};
			auto f=uvRect(44,20,48,32); r[2]={f[0],f[1],f[2],f[3]};
			auto bk=uvRect(52,20,56,32); r[3]={bk[0],bk[1],bk[2],bk[3]};
			auto ri=uvRect(40,20,44,32); r[5]={ri[0],ri[1],ri[2],ri[3]};
			auto le=uvRect(48,20,52,32); r[4]={le[0],le[1],le[2],le[3]};
		}else if(part==4){
			auto t=uvRect(20,48,24,52); auto b=uvRect(24,48,28,52); auto f=uvRect(20,52,24,64); auto bk=uvRect(28,52,32,64); auto ri=uvRect(16,52,20,64); auto le=uvRect(24,52,28,64);
			r[1]={t[0],t[1],t[2],t[3]}; r[0]={b[0],b[1],b[2],b[3]}; r[2]={f[0],f[1],f[2],f[3]}; r[3]={bk[0],bk[1],bk[2],bk[3]}; r[5]={ri[0],ri[1],ri[2],ri[3]}; r[4]={le[0],le[1],le[2],le[3]};
		}else{
			auto t=uvRect(36,48,40,52); auto b=uvRect(40,48,44,52); auto f=uvRect(36,52,40,64); auto bk=uvRect(44,52,48,64); auto ri=uvRect(32,52,36,64); auto le=uvRect(40,52,44,64);
			r[1]={t[0],t[1],t[2],t[3]}; r[0]={b[0],b[1],b[2],b[3]}; r[2]={f[0],f[1],f[2],f[3]}; r[3]={bk[0],bk[1],bk[2],bk[3]}; r[5]={ri[0],ri[1],ri[2],ri[3]}; r[4]={le[0],le[1],le[2],le[3]};
		}
		return r;
	};
	auto buildPart = [&](int partId, glm::vec3 origin, glm::vec3 sz, short bone){
		auto uvs = makeUVs(partId);
		glm::vec2 uvArr[6][4];
		for(int i=0;i<6;i++) for(int j=0;j<4;j++) uvArr[i][j]=uvs[i][j];
		addCube(origin,sz,uvArr,bone,0);
	};
	model.cleanup();
	model.transforms.clear();
	for(int i=0;i<6;i++) model.transforms.push_back(glm::mat4(1.f));
	model.headIndex=0; model.bodyIndex=1; model.rLegIndex=2; model.lLefIndex=3; model.rArmIndex=4; model.lArmIndex=5;
	buildPart(0, glm::vec3(-4*s,24*s,-4*s), glm::vec3(8*s,8*s,8*s), 0);
	{
		glm::vec3 o(-4*s- infHat,24*s- infHat,-4*s- infHat); glm::vec3 sz(8*s+2*infHat,8*s+2*infHat,8*s+2*infHat);
		glm::vec2 uvArr2[6][4];
		{ auto t=uvRect(48,0,56,8); for(int j=0;j<4;j++) uvArr2[0][j]=t[j]; }
		{ auto t=uvRect(40,0,48,8); for(int j=0;j<4;j++) uvArr2[1][j]=t[j]; }
		{ auto t=uvRect(40,8,48,16); for(int j=0;j<4;j++) uvArr2[2][j]=t[j]; }
		{ auto t=uvRect(56,8,64,16); for(int j=0;j<4;j++) uvArr2[3][j]=t[j]; }
		{ auto t=uvRect(48,8,56,16); for(int j=0;j<4;j++) uvArr2[4][j]=t[j]; }
		{ auto t=uvRect(32,8,40,16); for(int j=0;j<4;j++) uvArr2[5][j]=t[j]; }
		addCube(o,sz,uvArr2,0,0);
	}
	buildPart(1, glm::vec3(-4*s,12*s,-2*s), glm::vec3(8*s,12*s,4*s), 1);
	buildPart(2, glm::vec3(-3.9f*s,0,-2*s), glm::vec3(4*s,12*s,4*s), 2);
	buildPart(4, glm::vec3(-0.1f*s,0,-2*s), glm::vec3(4*s,12*s,4*s), 3);
	buildPart(3, glm::vec3(-8*s,12*s,-2*s), glm::vec3(4*s,12*s,4*s), 4);
	buildPart(5, glm::vec3(4*s,12*s,-2*s), glm::vec3(4*s,12*s,4*s), 5);
	model.vertexCount = indices.size();
	glGenVertexArrays(1,&model.vao); glBindVertexArray(model.vao);
	glGenBuffers(1,&model.geometry); glGenBuffers(1,&model.indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model.geometry); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexes.size()*sizeof(Data), vertexes.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Data),0);
	glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Data),(void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(2); glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Data),(void*)(sizeof(glm::vec3)*2));
	glEnableVertexAttribArray(3); glVertexAttribIPointer(3,1,GL_SHORT,sizeof(Data),(void*)(sizeof(glm::vec3)*2+sizeof(glm::vec2)));
	glEnableVertexAttribArray(4); glVertexAttribIPointer(4,1,GL_SHORT,sizeof(Data),(void*)(sizeof(glm::vec3)*2+sizeof(glm::vec2)+sizeof(short)));
	glBindVertexArray(0);
}
static void buildHandModel(Model &model){
	struct Data{ glm::vec3 position; glm::vec3 normal; glm::vec2 uv; short boneIndex; short textureIndex; };
	std::vector<Data> v; v.reserve(24); std::vector<unsigned int> idx; idx.reserve(36);
	auto uvRect = [](float x0,float y0,float x1,float y1)->std::array<glm::vec2,4>{ float u0=x0/64.f, v0=1.f - y1/64.f, u1=x1/64.f, v1=1.f - y0/64.f; return {glm::vec2(u0,v0), glm::vec2(u1,v0), glm::vec2(u1,v1), glm::vec2(u0,v1)}; };
	const float s=1.f/16.f;
	glm::vec3 origin(-2*s,-6*s,-2*s); glm::vec3 sz(4*s,12*s,4*s);
	auto uvs = [&]()->std::array<std::array<glm::vec2,4>,6>{
		std::array<std::array<glm::vec2,4>,6> r;
		auto t=uvRect(44,16,48,20); r[1]={t[0],t[1],t[2],t[3]};
		auto b=uvRect(48,16,52,20); r[0]={b[0],b[1],b[2],b[3]};
		auto f=uvRect(44,20,48,32); r[2]={f[0],f[1],f[2],f[3]};
		auto bk=uvRect(52,20,56,32); r[3]={bk[0],bk[1],bk[2],bk[3]};
		auto ri=uvRect(40,20,44,32); r[5]={ri[0],ri[1],ri[2],ri[3]};
		auto le=uvRect(48,20,52,32); r[4]={le[0],le[1],le[2],le[3]};
		return r;
	}();
	glm::vec3 c0=origin, c1=origin+glm::vec3(sz.x,0,0), c2=origin+glm::vec3(sz.x,0,sz.z), c3=origin+glm::vec3(0,0,sz.z), c4=origin+glm::vec3(0,sz.y,0), c5=origin+glm::vec3(sz.x,sz.y,0), c6=origin+glm::vec3(sz.x,sz.y,sz.z), c7=origin+glm::vec3(0,sz.y,sz.z);
	auto push=[&](glm::vec3 p0,glm::vec3 p1,glm::vec3 p2,glm::vec3 p3,glm::vec3 n, std::array<glm::vec2,4> uv){ unsigned int b=v.size(); v.push_back({p0,n,uv[0],0,0}); v.push_back({p1,n,uv[1],0,0}); v.push_back({p2,n,uv[2],0,0}); v.push_back({p3,n,uv[3],0,0}); idx.push_back(b); idx.push_back(b+1); idx.push_back(b+2); idx.push_back(b+2); idx.push_back(b+3); idx.push_back(b); };
	push(c0,c1,c2,c3,glm::vec3(0,-1,0), uvs[0]); push(c4,c7,c6,c5,glm::vec3(0,1,0), uvs[1]); push(c3,c2,c6,c7,glm::vec3(0,0,1), uvs[2]); push(c1,c0,c4,c5,glm::vec3(0,0,-1), uvs[3]); push(c1,c5,c6,c2,glm::vec3(1,0,0), uvs[4]); push(c0,c3,c7,c4,glm::vec3(-1,0,0), uvs[5]);
	model.cleanup(); model.transforms.clear(); model.transforms.push_back(glm::mat4(1.f)); model.vertexCount=idx.size();
	glGenVertexArrays(1,&model.vao); glBindVertexArray(model.vao); glGenBuffers(1,&model.geometry); glGenBuffers(1,&model.indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model.geometry); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);
	glBufferData(GL_ARRAY_BUFFER, v.size()*sizeof(Data), v.data(), GL_STATIC_DRAW); glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Data),0);
	glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Data),(void*)sizeof(glm::vec3));
	glEnableVertexAttribArray(2); glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Data),(void*)(sizeof(glm::vec3)*2));
	glEnableVertexAttribArray(3); glVertexAttribIPointer(3,1,GL_SHORT,sizeof(Data),(void*)(sizeof(glm::vec3)*2+sizeof(glm::vec2)));
	glEnableVertexAttribArray(4); glVertexAttribIPointer(4,1,GL_SHORT,sizeof(Data),(void*)(sizeof(glm::vec3)*2+sizeof(glm::vec2)+sizeof(short)));
	glBindVertexArray(0);
}

void ModelsManager::loadAllModels(std::string path, bool reportErrors)
{

	if (!temporaryPlayerHandTexture.id)
	{
		temporaryPlayerHandTexture.loadFromFile(RESOURCES_PATH "skins/mage.png", true);
		temporaryPlayerHandBindlessTexture = glGetTextureHandleARB(temporaryPlayerHandTexture.id);
		glMakeTextureHandleResidentARB(temporaryPlayerHandBindlessTexture);
	}


	bool appendMode = texturesIds.empty();


	//load default texture
	if(appendMode)
	{
		unsigned char data[16] = {};

		{
			int i = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 255;

			data[i++] = 146;
			data[i++] = 52;
			data[i++] = 235;
			data[i++] = 255;

			data[i++] = 146;
			data[i++] = 52;
			data[i++] = 235;
			data[i++] = 255;

			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 0;
			data[i++] = 255;
		}

		gl2d::Texture t;
		t.createFromBuffer((char *)data, 2, 2, true, false);

		texturesIds.push_back(t.id);
		auto handle = glGetTextureHandleARB(t.id);
		glMakeTextureHandleResidentARB(handle);
		gpuIds.push_back(handle);
	}

	auto loadTexture = [&](const char *path, bool appendMode, int index, bool isPlayerSkin = 0)
	{

		if (!appendMode && texturesIds[index] != texturesIds[0]) { return; } //we already have the texture

		gl2d::Texture texture = {};
		GLuint64 handle = 0;

		if (isPlayerSkin)
		{
			texture = loadPlayerSkin(path);
		}
		else
		{
			texture.loadFromFile(path, true, true);
		}

		if (texture.id)
		{

			texture.bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 6.f);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 2.f);

			glGenerateMipmap(GL_TEXTURE_2D);

			handle = glGetTextureHandleARB(texture.id);
			glMakeTextureHandleResidentARB(handle);
		}
		else
		{
			//todo error report
			std::cout << "Error loading: " << path << "\n";

			handle = gpuIds[0];
			texture.id = texturesIds[0];
		}

		if (appendMode)
		{
			gpuIds.push_back(handle);
			texturesIds.push_back(texture.id);
		}
		else
		{
			gpuIds[index] = handle;
			texturesIds[index] = texture.id;
		}


	};


	//load textures
	{
		int index = 1;
		std::string stevePath = path + "steve3.png";
		if (!std::filesystem::exists(stevePath)) stevePath = path + "steve.png";
		loadTexture(stevePath.c_str(), appendMode, index++, true);
		loadTexture((path + "zombie.png").c_str(), appendMode, index++, true);
		loadTexture((path + "pig.png").c_str(), appendMode, index++);
		loadTexture((path + "cat.png").c_str(), appendMode, index++);
		loadTexture((path + "goblin.png").c_str(), appendMode, index++);
		loadTexture((path + "trainingDummy.png").c_str(), appendMode, index++);
		loadTexture((path + "scarecrow.png").c_str(), appendMode, index++);
		loadTexture((path + "slime.png").c_str(), appendMode, index++);
		loadTexture((path + "skeleton.png").c_str(), appendMode, index++);
		loadTexture((path + "creeper.png").c_str(), appendMode, index++);		loadTexture((path+ "helmetTest.png").c_str(), appendMode, index++);
		loadTexture((path+ "hydraFire.png").c_str(), appendMode, index++);
		loadTexture((path+ "hydraIce.png").c_str(), appendMode, index++);
		loadTexture((path+ "hydraPoison.png").c_str(), appendMode, index++);
		// New mob textures
		loadTexture((path+ "sheep.png").c_str(), appendMode, index++);
		loadTexture((path+ "cow.png").c_str(), appendMode, index++);
		loadTexture((path+ "wolf.png").c_str(), appendMode, index++);
		loadTexture((path+ "fox.png").c_str(), appendMode, index++);
		loadTexture((path+ "chicken.png").c_str(), appendMode, index++);
		loadTexture((path+ "crow.png").c_str(), appendMode, index++);
		loadTexture((path+ "bee.png").c_str(), appendMode, index++);
		loadTexture((path+ "manatee.png").c_str(), appendMode, index++);
		
	}


	Assimp::Importer importer;

	// Step 2: Specify Import Options
	unsigned int flags = aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GenUVCoords | aiProcess_TransformUVCoords 
		| aiProcess_FindInstances | aiProcess_GenNormals;

	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT); // Remove lines and points
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS);

	struct Data
	{
		glm::vec3 position = {};
		glm::vec3 normal = {};
		glm::vec2 uv = {};
		short boneIndex = 0;
		short textureIndex = 0;
	};

	std::vector<Data> vertexes;
	vertexes.reserve(400);

	std::vector<unsigned int> indices;
	indices.reserve(400);

	auto loadModel = [&](const char *path, Model &model, bool multipleTextures = 0)
	{
		vertexes.clear();
		indices.clear();

		const aiScene *scene = importer.ReadFile(path, flags);

		if (scene)
		{

			glGenVertexArrays(1, &model.vao);
			glBindVertexArray(model.vao);

			glGenBuffers(1, &model.geometry);
			glGenBuffers(1, &model.indexBuffer);

			glBindBuffer(GL_ARRAY_BUFFER, model.geometry);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);

			unsigned int vertexOffset = 0;

			int boneIndex = 0;
			for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; ++i)
			{

				const aiNode *node = scene->mRootNode->mChildren[i];

				const char *name = node->mName.C_Str();

				short textureIndex = 0;

				if (areStringsSameToLower(name, "Head")) { model.headIndex = i; }else
				if (areStringsSameToLower(name, "Body")) { model.bodyIndex = i; }else
				if (areStringsSameToLower(name, "RLeg")) { model.rLegIndex = i; }else
				if (areStringsSameToLower(name, "LLeg")) { model.lLefIndex = i; }else
				if (areStringsSameToLower(name, "RArm")) { model.rArmIndex = i; }else
				if (areStringsSameToLower(name, "LArm")) { model.lArmIndex = i; }else
				if (areStringsSameToLower(name, "Pupils")) { model.pupilsIndex = i; }else
				if (areStringsSameToLower(name, "LEye")) { model.lEyeIndex = i; }else
				if (areStringsSameToLower(name, "REye")) { model.rEyeIndex = i; }else
				if (areStringsSameToLower(name, "HeadArmour")) { model.headArmourIndex = i; textureIndex = 1; }else
				if (areStringsSameToLower(name, "BodyArmour")) { model.bodyArmourIndex = i; textureIndex = 2; }else
				if (areStringsSameToLower(name, "RLegArmour")) { model.rLegArmourIndex = i; textureIndex = 3; }else
				if (areStringsSameToLower(name, "LLegArmour")) { model.lLefArmourIndex = i; textureIndex = 3; }else
				if (areStringsSameToLower(name, "RArmArmour")) { model.rArmArmourIndex = i; textureIndex = 2; }else
				if (areStringsSameToLower(name, "LArmArmour")) { model.lArmArmourIndex = i; textureIndex = 2; }

				if (!multipleTextures) { textureIndex = 0; }

				for (int m = 0; m < node->mNumMeshes; m++)
				{
					const aiMesh *mesh = scene->mMeshes[node->mMeshes[m]];

					//const char *name2 = mesh->mName.C_Str();

					aiMatrix4x4 transform = node->mTransformation;
					model.transforms.push_back(glm::transpose(aiToGlm(transform)));

					for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
					{
						Data vertex;
						vertex.textureIndex = textureIndex;

						vertex.boneIndex = boneIndex;

						// Positions
						vertex.position.x = mesh->mVertices[v].x;
						vertex.position.y = mesh->mVertices[v].y;
						vertex.position.z = mesh->mVertices[v].z;

						// Normals
						vertex.normal.x = mesh->mNormals[v].x;
						vertex.normal.y = mesh->mNormals[v].y;
						vertex.normal.z = mesh->mNormals[v].z;

						// UVs (if present)
						if (mesh->HasTextureCoords(0))
						{
							vertex.uv.x = mesh->mTextureCoords[0][v].x;
							vertex.uv.y = mesh->mTextureCoords[0][v].y;
						}
						else
						{
							vertex.uv = glm::vec2(0.0f, 0.0f);
						}

						vertexes.push_back(vertex);
					}

					for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
					{
						const aiFace &face = mesh->mFaces[i];
						for (unsigned int j = 0; j < face.mNumIndices; ++j)
						{
							indices.push_back(face.mIndices[j] + vertexOffset);
						}
					}

					vertexOffset += mesh->mNumVertices;
				}

				if (node->mNumMeshes)
				{
					boneIndex++;
				}
			}


			//animations
			int animationsCount = scene->mNumAnimations;

			for (int animationTypes = 0; animationTypes < Animation::ANIMATIONS_COUNT; animationTypes++)
			{
				model.animationsIndex[animationTypes] = -1;
			}

			for (int i = 0; i < animationsCount; i++)
			{
				auto animation = scene->mAnimations[i];
					
				bool good = false;

				for (int animationTypes = 1; animationTypes < Animation::ANIMATIONS_COUNT; animationTypes++)
				{

					auto str = std::string(magic_enum::enum_name((Animation::AnimationType)animationTypes));

					if (animation->mName.C_Str() == str)
					{
						good = true;
						model.animationsIndex[animationTypes] = model.animations.size();
						break;
					}

				}

				if (!good)
				{
					continue;
				}

				//if (strstr(animation->mName.C_Str(), "running_loop") == nullptr)
				//{
				//	continue;
				//}

				Animation anim;
				anim.animationLength = static_cast<float>(animation->mDuration) / 1000.f;

				int nodeCount = scene->mRootNode->mNumChildren;
				anim.kayFrames.resize(nodeCount); // Ensure each node has a slot

				// Map node names to their indices in the model
				std::unordered_map<std::string, int> nodeIndexMap;
				for (int j = 0; j < nodeCount; j++)
				{
					nodeIndexMap[scene->mRootNode->mChildren[j]->mName.C_Str()] = j;
				}

				// Process each animation channel
				for (unsigned int channelIndex = 0; channelIndex < animation->mNumChannels; channelIndex++)
				{
					aiNodeAnim *nodeAnim = animation->mChannels[channelIndex];
					std::string nodeName = nodeAnim->mNodeName.C_Str();

					if (nodeIndexMap.find(nodeName) == nodeIndexMap.end())
					{
						continue; // Skip if the node doesn't exist in our order
					}

					int nodeIndex = nodeIndexMap[nodeName];

					// Read keyframes and store them
					for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++)
					{
						KeyFrame keyframe;
						keyframe.timestamp = static_cast<float>(nodeAnim->mPositionKeys[k].mTime) / 1000.f;

						// Position
						keyframe.pos = glm::vec3(nodeAnim->mPositionKeys[k].mValue.x,
							nodeAnim->mPositionKeys[k].mValue.y,
							nodeAnim->mPositionKeys[k].mValue.z);

						// Rotation
						keyframe.rotation = glm::quat(nodeAnim->mRotationKeys[k].mValue.w,
							nodeAnim->mRotationKeys[k].mValue.x,
							nodeAnim->mRotationKeys[k].mValue.y,
							nodeAnim->mRotationKeys[k].mValue.z);

						// Scale
						keyframe.scale = glm::vec3(nodeAnim->mScalingKeys[k].mValue.x,
							nodeAnim->mScalingKeys[k].mValue.y,
							nodeAnim->mScalingKeys[k].mValue.z);

						anim.kayFrames[nodeIndex].push_back(keyframe);
					}
				}

				// Fill missing keyframes for nodes that weren't animated
				for (int j = 0; j < nodeCount; j++)
				{
					if (anim.kayFrames[j].empty())
					{
						KeyFrame defaultKeyframe;
						defaultKeyframe.timestamp = 0.0f;

						aiVector3D scale, position;
						aiQuaternion rotation;
						scene->mRootNode->mChildren[j]->mTransformation.Decompose(scale, rotation, position);

						defaultKeyframe.pos = glm::vec3(position.x, position.y, position.z);
						defaultKeyframe.rotation = glm::quat(rotation.w, rotation.x, rotation.y, rotation.z);
						defaultKeyframe.scale = glm::vec3(scale.x, scale.y, scale.z);

						anim.kayFrames[j].push_back(defaultKeyframe);
					}
				}


				model.animations.push_back(std::move(anim));
			}



			model.vertexCount = indices.size();

			if (!model.vertexCount) { std::cout << "!!!!Error wrong model format!!!!!!!!!!!!!\n"; }

			glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(Data), vertexes.data(), GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void *)(sizeof(float) * 3));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void *)(sizeof(float) * 6));
			glEnableVertexAttribArray(3); //bone
			glVertexAttribIPointer(3, 1, GL_SHORT, sizeof(float) * 9, (void *)(sizeof(float) * 8));

			glEnableVertexAttribArray(4); //texture id
			glVertexAttribIPointer(4, 1, GL_SHORT, sizeof(float) * 9, (void *)(sizeof(float) * 8 + sizeof(short)));



			glBindVertexArray(0);

		}
		else if(reportErrors)
		{
			std::cout << "noSchene in" << path << "\n";
			std::cout << importer.GetErrorString() << "\n";
		}


	};


	if(!human.vertexCount)
		loadModel((path + "human.glb").c_str(), human, true);
	buildSteveModel(human);

	if (!pig.vertexCount)
		loadModel((path + "pig.glb").c_str(), pig);
	
	if (!cat.vertexCount)
		loadModel((path + "cat.glb").c_str(), cat);

	if (!cow.vertexCount)
		loadModel((path + "cow.glb").c_str(), cow);
	if (!sheep.vertexCount)
		loadModel((path + "sheep.glb").c_str(), sheep);
	if (!wolf.vertexCount)
		loadModel((path + "wolf.glb").c_str(), wolf);
	if (!fox.vertexCount)
		loadModel((path + "fox.glb").c_str(), fox);
	if (!chicken.vertexCount)
		loadModel((path + "chicken.glb").c_str(), chicken);
	if (!crow.vertexCount)
		loadModel((path + "crow.glb").c_str(), crow);
	if (!bee.vertexCount)
		loadModel((path + "bee.glb").c_str(), bee);
	if (!manatee.vertexCount)
		loadModel((path + "manatee.glb").c_str(), manatee);

	if (!rightHand.vertexCount)
		loadModel((path + "rightHand.glb").c_str(), rightHand);
	buildHandModel(rightHand);

	if (!goblin.vertexCount)
		loadModel((path + "goblin.glb").c_str(), goblin);

	if (!trainingDummy.vertexCount)
		loadModel((path + "trainingDummy.glb").c_str(), trainingDummy);

	if (!scareCrow.vertexCount)
		loadModel((path + "scareCrow.glb").c_str(), scareCrow);

	if (!slime.vertexCount)
		loadModel((path + "slime.glb").c_str(), slime);	if (!creeper.vertexCount)
		loadModel((path + "creeper.glb").c_str(), creeper);

	if (!hydra.vertexCount)
		loadModel((path + "hydra.glb").c_str(), hydra); // placeholder - use goblin model until custom hydra model is created

		
	flags = aiProcess_ImproveCacheLocality 
		| aiProcess_JoinIdenticalVertices 
		| aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_FindInstances;

	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT); // Remove lines and points
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS);


	struct Quad
	{
		int a = 0;
		int b = 0;
		int c = 0;
		int d = 0;
	};

	std::vector<Quad> quads;
	quads.reserve(50);

	auto loadBlockModel = [&](const char *path, BlockModel &blockModel)
	{
		blockModel.cleanup();

		const aiScene *scene = importer.ReadFile(path, flags);

		if (scene && scene->mRootNode->mNumChildren)
		{
			blockModel.maxPos = glm::vec3{-10000,-10000,-10000};
			blockModel.minPos = glm::vec3{10000,10000,10000};

			for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; ++i)
			{
				const aiNode *node = scene->mRootNode->mChildren[i];
				aiMatrix4x4 transform = node->mTransformation;

				for (int m = 0; m < node->mNumMeshes; m++)
				{
					const aiMesh *mesh = scene->mMeshes[node->mMeshes[m]];
					quads.clear();

					struct Triangle
					{
						unsigned int indices[3] = {};
						aiVector3D normal = {};
						int adjacentIndex = -1; // Store the index of the triangle it merges with
					};

					std::vector<Triangle> triangles;
					triangles.reserve(mesh->mNumFaces);

					// Step 1: Collect triangles and compute normals
					for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
					{
						const aiFace &face = mesh->mFaces[i];

						if (face.mNumIndices != 3)
						{
							std::cout << "ERROR wrong model format!\n";
							continue;
						}

						Triangle tri;
						tri.indices[0] = face.mIndices[0];
						tri.indices[1] = face.mIndices[1];
						tri.indices[2] = face.mIndices[2];

						// Compute normal using cross product
						aiVector3D v0 = mesh->mVertices[tri.indices[0]];
						aiVector3D v1 = mesh->mVertices[tri.indices[1]];
						aiVector3D v2 = mesh->mVertices[tri.indices[2]];
						tri.normal = (v1 - v0) ^ (v2 - v0); // Cross product
						tri.normal.Normalize();

						if (blockModel.maxPos.x < v0.x) { blockModel.maxPos.x = v0.x; }
						if (blockModel.maxPos.y < v0.y) { blockModel.maxPos.y = v0.y; }
						if (blockModel.maxPos.z < v0.z) { blockModel.maxPos.z = v0.z; }
						if (blockModel.maxPos.x < v1.x) { blockModel.maxPos.x = v1.x; }
						if (blockModel.maxPos.y < v1.y) { blockModel.maxPos.y = v1.y; }
						if (blockModel.maxPos.z < v1.z) { blockModel.maxPos.z = v1.z; }
						if (blockModel.maxPos.x < v2.x) { blockModel.maxPos.x = v2.x; }
						if (blockModel.maxPos.y < v2.y) { blockModel.maxPos.y = v2.y; }
						if (blockModel.maxPos.z < v2.z) { blockModel.maxPos.z = v2.z; }

						if (blockModel.minPos.x > v0.x) { blockModel.minPos.x = v0.x; }
						if (blockModel.minPos.y > v0.y) { blockModel.minPos.y = v0.y; }
						if (blockModel.minPos.z > v0.z) { blockModel.minPos.z = v0.z; }
						if (blockModel.minPos.x > v1.x) { blockModel.minPos.x = v1.x; }
						if (blockModel.minPos.y > v1.y) { blockModel.minPos.y = v1.y; }
						if (blockModel.minPos.z > v1.z) { blockModel.minPos.z = v1.z; }
						if (blockModel.minPos.x > v2.x) { blockModel.minPos.x = v2.x; }
						if (blockModel.minPos.y > v2.y) { blockModel.minPos.y = v2.y; }
						if (blockModel.minPos.z > v2.z) { blockModel.minPos.z = v2.z; }

						triangles.push_back(tri);
					}

					// Step 2: Find adjacent triangle pairs
					std::map<std::pair<unsigned int, unsigned int>, int> edgeMap; // Edge -> Triangle index

					for (size_t i = 0; i < triangles.size(); i++)
					{
						auto &tri = triangles[i];

						for (int e = 0; e < 3; e++)
						{
							unsigned int a = tri.indices[e];
							unsigned int b = tri.indices[(e + 1) % 3];

							if (a > b)
								std::swap(a, b);

							auto edge = std::make_pair(a, b);
							if (edgeMap.count(edge))
							{
								int otherIndex = edgeMap[edge];
								auto &otherTri = triangles[otherIndex];

								// Step 3: Check for planarity
								if (tri.normal * otherTri.normal > 0.9999f) // Almost parallel normals
								{
									// Step 4: Merge into quad
									tri.adjacentIndex = otherIndex;
									otherTri.adjacentIndex = i;
								}
							}
							else
							{
								edgeMap[edge] = i;
							}
						}
					}

					// Step 5: Store quads
					std::vector<char> merged(triangles.size(), false);

					for (size_t i = 0; i < triangles.size(); i++)
					{
						if (merged[i] || triangles[i].adjacentIndex == -1)
							continue;

						int j = triangles[i].adjacentIndex;
						if (merged[j])
							continue;

						// Find the shared edge
						std::vector<unsigned int> shared, unique;
						for (unsigned int v : triangles[i].indices)
							if (std::find(std::begin(triangles[j].indices), std::end(triangles[j].indices), v) != std::end(triangles[j].indices))
								shared.push_back(v);
							else
								unique.push_back(v);

						for (unsigned int v : triangles[j].indices)
							if (std::find(std::begin(triangles[i].indices), std::end(triangles[i].indices), v) == std::end(triangles[i].indices))
								unique.push_back(v);

						if (shared.size() == 2 && unique.size() == 2)
						{
							Quad q;

							// Check triangle winding order and adjust
							aiVector3D v0 = mesh->mVertices[unique[0]];
							aiVector3D v1 = mesh->mVertices[shared[0]];
							aiVector3D v2 = mesh->mVertices[shared[1]];

							aiVector3D edge1 = v1 - v0;
							aiVector3D edge2 = v2 - v0;
							aiVector3D normal = edge1 ^ edge2; // Cross product

							if (normal * triangles[i].normal < 0.0f)
							{
								// Flip to maintain consistency
								q.a = unique[1];
								q.b = shared[0];
								q.c = unique[0];
								q.d = shared[1];
							}
							else
							{
								// Keep order
								q.a = unique[0];
								q.b = shared[0];
								q.c = unique[1];
								q.d = shared[1];
							}

							quads.push_back(q);
							merged[i] = merged[j] = true;
						}

					}

					// Step 6: Push quads to blockModel
					for (const Quad &q : quads)
					{

						aiVector3D v = mesh->mVertices[q.a];
						v *= transform;
						blockModel.vertices.push_back(v.x);
						blockModel.vertices.push_back(v.y - 0.5);
						blockModel.vertices.push_back(v.z);

						v = mesh->mVertices[q.b];
						v *= transform;
						blockModel.vertices.push_back(v.x);
						blockModel.vertices.push_back(v.y - 0.5);
						blockModel.vertices.push_back(v.z);

						v = mesh->mVertices[q.c];
						v *= transform;
						blockModel.vertices.push_back(v.x);
						blockModel.vertices.push_back(v.y - 0.5);
						blockModel.vertices.push_back(v.z);

						v = mesh->mVertices[q.d];
						v *= transform;
						blockModel.vertices.push_back(v.x);
						blockModel.vertices.push_back(v.y - 0.5);
						blockModel.vertices.push_back(v.z);

						if (mesh->mTextureCoords[0]) // Has UVs
						{

							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.a].x);
							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.a].y);

							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.b].x);
							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.b].y);


							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.c].x);
							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.c].y);

							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.d].x);
							blockModel.uvs.push_back(mesh->mTextureCoords[0][q.d].y);

						}
					}
				}
			}


			if (blockModel.vertices.size() % 3 != 0 && 
				blockModel.vertices.size() % 12 != 0 &&
				blockModel.uvs.size() %2 != 0 &&
				(blockModel.vertices.size()/3) != (blockModel.uvs.size() /2)
				)
			{
				std::cout << "ERROR LOADING MODEL!\n";
				blockModel.cleanup();
			}


		}
	};


	const char *blockModelsNames[] = 
	{
		"chair.glb",
		"aleMug.glb",
		"goblet.glb",
		"wineBottle.glb",
		"skull.glb",
		"skullTorch.glb",
		"books.glb",
		"candleHolder.glb",
		"pot.glb",
		"jar.glb",
		"globe.glb",
		"keg.glb",
		"workBench.glb",
		"table.glb",
		"workItems.glb",
		"chairBig.glb",
		"cookingPot.glb",
		"chuckenCaracas.glb",
		"chuckenWingsPlate.glb",
		"fishPlate.glb",
		"ladder.glb",
		"vines.glb",
		"rock.glb",
		"chest.glb",
		"crate.glb",
		"torch.glb",
		"torchHolder.glb",
		"lamp.glb",
		"lampWall.glb",
		"slab.glb",
		"stairs.glb",
		"wall.glb",
		"trainingDummyBase.glb",
		"target.glb",
		"furnace.glb",
		"goblinWorkBench.glb",
		"goblinChair.glb",
		"goblinTable.glb",
		"goblinStitchingPost.glb",
		"fence.glb",
		"fence_front.glb",
		"fence_back.glb",
		"fence_left.glb",
		"fence_right.glb",
	};

	static_assert(sizeof(blockModelsNames) / sizeof(blockModelsNames[0]) == BLOCK_MODELS_COUNT);
	

	for (int i = 0; i < BLOCK_MODELS_COUNT; i++)
	{

		if (!blockModels[i].vertices.size())
			loadBlockModel((path + blockModelsNames[i]).c_str(), blockModels[i]);

	}




	//todo check if it frees all of them
	importer.FreeScene();


	setupSSBO();
}


//gets the block shape
int getDefaultBlockShapeForFurniture(unsigned int b)
{

	if (isChairMesh(b))
	{
		return ModelsManager::chairModel;
	}

	if (isGobletMesh(b))
	{
		return ModelsManager::gobletModel;
	}

	switch (b)
	{

		case mug: return ModelsManager::mugModel;

		case wineBottle:  return ModelsManager::wineBottleModel;
		case skull: return ModelsManager::skullModel ;
		case skullTorch: return ModelsManager::skullTorchModel ;
		case book: return ModelsManager::booksModel ;
		case candleHolder: return ModelsManager::candleHolderModel ;
		case pot: return ModelsManager::potModel ;
		case jar: return ModelsManager::jarModel ;
		case keg: return ModelsManager::keg;
		case cookingPot: return ModelsManager::cookingPotModel;
		case chickenCaracas: return ModelsManager::chickenCaracasModel;
		case chickenWingsPlate: return ModelsManager::chickenWingsPlateModel;
		case fishPlate: return ModelsManager::fishPlateModel;
		case workBench: return ModelsManager::workBenchModel;
		case oakTable: return ModelsManager::tableModel;
		case craftingItems: return ModelsManager::workItemsModel;
		case oakLogTable: return ModelsManager::tableModel;
		case oakBigChair: return ModelsManager::chairBigModel;
		case oakLogBigChair: return ModelsManager::chairBigModel;
		case smallRock: return ModelsManager::smallRockModel;
		case woddenChest: return ModelsManager::chestModel;
		case goblinChest: return ModelsManager::chestModel;
		case copperChest: return ModelsManager::chestModel;
		case ironChest: return ModelsManager::chestModel;
		case silverChest: return ModelsManager::chestModel;
		case goldChest: return ModelsManager::chestModel;
		case smallCrate: return ModelsManager::crateModel;
		case globe: return ModelsManager::globeModel;
		case lamp: return ModelsManager::lampModel;
		case torch: return ModelsManager::torchModel;
		case torchWood: return ModelsManager::torchModel;
		case goblinTorch: return ModelsManager::torchModel;
		case trainingDummy: return ModelsManager::trainingDummyBaseModel;
		case target: return ModelsManager::targetModel;
		case furnace: return ModelsManager::furnaceModel;
		case goblinWorkBench : return ModelsManager::goblinWorkBenchModel;
		case goblinChair: return ModelsManager::goblinChairModel;
		case goblinTable: return ModelsManager::goblinTableModel;
		case goblinStitchingPost: return ModelsManager::goblinStitchingPostModel;
		case woodenFence: return ModelsManager::fence;
		case woodenLogFence: return ModelsManager::fence;
		case spruceFence: return ModelsManager::fence;
		case spruceLogFence: return ModelsManager::fence;
		case birchFence: return ModelsManager::fence;
		case birchLogFence: return ModelsManager::fence;

	}
	return 0;
}

void ModelsManager::clearAllModels()
{
	human.cleanup();

	pig.cleanup();

	cat.cleanup();

	rightHand.cleanup();

	goblin.cleanup();

	trainingDummy.cleanup();

	slime.cleanup();
	creeper.cleanup();
	scareCrow.cleanup();
	hydra.cleanup();

	for (int i = 0; i < BLOCK_MODELS_COUNT; i++)
	{

		blockModels[i].cleanup();
	}


	if (texturesIds.size())
	{
		auto defaultTexture = texturesIds[0];

		for (int i = 1; i < texturesIds.size(); i++)
		{
			if (texturesIds[i] != defaultTexture)
			{
				glDeleteTextures(1, &texturesIds[i]);
			}
		}
		glDeleteTextures(1, &texturesIds[0]);
	}

	texturesIds.clear();
	gpuIds.clear();

}

void ModelsManager::setupSSBO()
{

	if (!texturesSSBO)
	{
		glGenBuffers(1, &texturesSSBO);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturesSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, gpuIds.size() * sizeof(gpuIds[0]), gpuIds.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, texturesSSBO); //todo add some constatns here

}

void animatePlayerLegs(glm::mat4 *poseVector,
	float &currentAngle, int &direction, float deltaTime)
{

	if (direction == 0)
	{
		if (currentAngle)
		{
			if (currentAngle > 0)
			{
				currentAngle -= deltaTime;
				if (currentAngle < 0)
				{
					currentAngle = 0;
				}
			}
			else
			{
				currentAngle += deltaTime;
				if (currentAngle > 0)
				{
					currentAngle = 0;
				}
			}
		
		}
	}
	{
		currentAngle += deltaTime * direction;

		if (direction == 1)
		{
			if (currentAngle >= glm::radians(40.f))
			{
				currentAngle = glm::radians(40.f);
				direction = -1;
			}
		}
		else
		{
			if (currentAngle <= glm::radians(-40.f))
			{
				currentAngle = glm::radians(-40.f);
				direction = 1;
			}
		}

	}

	
	poseVector[0]; //head
	poseVector[1]; //torso
	poseVector[2]; //right hand
	poseVector[3]; //left hand
	poseVector[4]; //right leg
	poseVector[5]; //left leg

	poseVector[4] = poseVector[4] * glm::rotate(currentAngle, glm::vec3{1.f,0.f,0.f});
	poseVector[5] = poseVector[5] * glm::rotate(-currentAngle, glm::vec3{1.f,0.f,0.f});

}

gl2d::Texture loadPlayerSkin(const char *path)
{
	//todo implement
	std::ifstream file(path, std::ios::binary);

	if (!file.is_open())
	{
		return {};
	}

	int fileSize = 0;
	file.seekg(0, std::ios::end);
	fileSize = (int)file.tellg();
	file.seekg(0, std::ios::beg);
	unsigned char *fileData = new unsigned char[fileSize];
	file.read((char *)fileData, fileSize);
	file.close();

	gl2d::Texture texture;
	{
		stbi_set_flip_vertically_on_load(true);

		int width = 0;
		int height = 0;
		int channels = 0;

		const unsigned char *decodedImage = stbi_load_from_memory(fileData, (int)fileSize, &width, &height, &channels, 4);

		if (width == height && width == PLAYER_SKIN_SIZE)
		{
			texture.createFromBuffer((const char *)decodedImage, width, height, true, true);
		}
		else if(width == height * 2 && width == PLAYER_SKIN_SIZE)
		{
			std::vector<char> newData;
			newData.resize(width * height * 4 * 2);
			memcpy(newData.data() + width * height * 4, decodedImage, width * height * 4);
			texture.createFromBuffer(newData.data(), width, height * 2, true, true);
		}
		else if(width == 64 && height == 64)
		{
			std::vector<unsigned char> upscaled(128*128*4);
			for(int y=0;y<64;y++) for(int x=0;x<64;x++){
				for(int dy=0;dy<2;dy++) for(int dx=0;dx<2;dx++){
					int s = (y*64+x)*4;
					int d = ((y*2+dy)*128 + (x*2+dx))*4;
					upscaled[d+0]=decodedImage[s+0];
					upscaled[d+1]=decodedImage[s+1];
					upscaled[d+2]=decodedImage[s+2];
					upscaled[d+3]=decodedImage[s+3];
				}
			}
			texture.createFromBuffer((const char*)upscaled.data(), 128, 128, true, true);
		}
		else if(width == 64 && height == 32)
		{
			std::vector<unsigned char> upscaled(128*64*4);
			for(int y=0;y<32;y++) for(int x=0;x<64;x++){
				for(int dy=0;dy<2;dy++) for(int dx=0;dx<2;dx++){
					int s = (y*64+x)*4;
					int d = ((y*2+dy)*128 + (x*2+dx))*4;
					upscaled[d+0]=decodedImage[s+0];
					upscaled[d+1]=decodedImage[s+1];
					upscaled[d+2]=decodedImage[s+2];
					upscaled[d+3]=decodedImage[s+3];
				}
			}
			std::vector<char> newData;
			newData.resize(128*128*4);
			memcpy(newData.data() + 128*64*4, upscaled.data(), 128*64*4);
			texture.createFromBuffer(newData.data(), 128, 128, true, true);
		}

		STBI_FREE(decodedImage);

	}

	delete[] fileData;

	return texture;
}

void Model::cleanup()
{
	glDeleteBuffers(1, &indexBuffer);
	glDeleteBuffers(1, &geometry);
	glDeleteVertexArrays(1, &vao);

	*this = std::decay_t<decltype(*this)>{};
}

glm::mat4 BoneTransform::getPoseMatrix()
{
	auto poseMatrix = glm::mat4(1.f);
	poseMatrix = glm::translate(position) 
		* glm::toMat4(rotation);

	return poseMatrix;
}

bool BoneTransform::goTowards(BoneTransform &other, float speed)
{
	// 1. Move position towards the target at a constant speed
	glm::vec3 direction = other.position - position;
	float distanceToMove = speed;
	float distanceToTarget = glm::length(direction);

	bool ret = false;

	if (distanceToMove >= distanceToTarget)
	{
		position = other.position; // Clamp to target
		ret = true;
	}
	else
	{
		position += glm::normalize(direction) * distanceToMove;
	}

	// 2. Rotate towards the target at a constant angular speed
	float dot = glm::dot(rotation, other.rotation);

	// Clamp dot to prevent errors due to floating point precision
	dot = glm::clamp(dot, -1.0f, 1.0f);

	// Calculate the angle between the quaternions
	float angle = std::acos(dot) * 2.0f;

	float angleToMove = speed;

	if (angleToMove >= angle)
	{
		rotation = other.rotation; // Clamp to target
	}
	else
	{
		rotation = glm::slerp(rotation, other.rotation, angleToMove / angle);
		ret = false;
	}

	return ret;
}
