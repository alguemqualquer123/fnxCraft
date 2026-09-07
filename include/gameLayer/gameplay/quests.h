#pragma once
#include <string>
#include <vector>
#include <functional>

enum QuestType { MAIN, SIDE };

struct Quest {
    int id;
    std::string title;
    std::string desc;
    QuestType type;
    bool completed = false;
    int progress = 0;
    int target = 1;
    std::function<bool()> condition;
};

struct QuestManager {
    std::vector<Quest> quests;
    void init();
    void update();
    void complete(int id);
    Quest* get(int id);
    std::vector<Quest*> getActive();
};

extern QuestManager gQuestManager;
