#include <filesystem>
#include "gameLayerHelpers.h"
#include <gameLayer/GamePaths.h>

namespace {

void ensureDirectoryExists(const std::filesystem::path &path)
{
	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directories(path);
	}
}

}

void ensureAllDataDirectories()
{
	GamePaths &paths = GamePaths::get();
	ensureDirectoryExists(paths.data());
	ensureDirectoryExists(paths.worlds());
	ensureDirectoryExists(paths.logs());
	ensureDirectoryExists(paths.players());
	ensureDirectoryExists(paths.backups());
	ensureDirectoryExists(paths.config());
	ensureDirectoryExists(paths.plugins());
	ensureDirectoryExists(paths.resources());
	ensureDirectoryExists(paths.playerSettings());
	ensureDirectoryExists(paths.userAccounts());
}
