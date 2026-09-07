#include <gameplay/quests.h>
#include <iostream>

QuestManager gQuestManager;

void QuestManager::init(){
    quests.clear();
    quests.push_back({1, "Era dos Artesãos", "Encontre o livro tutorial", MAIN, false, 0, 1});
    quests.push_back({2, "O Vazio", "Ative o portal do Nether", MAIN, false, 0, 1});
    quests.push_back({3, "4 Facções", "Fale com o aldeão comerciante", MAIN, false, 0, 1});
    quests.push_back({4, "Protagonista acorda", "Acorde nas ruínas", MAIN, true, 1, 1});
    quests.push_back({5, "Livro Tutorial", "Leia as 10 páginas", MAIN, false, 0, 10});
    quests.push_back({6, "NPC Aldeão", "Negocie com o aldeão", MAIN, false, 0, 1});
    for(int i=7;i<=21;i++) quests.push_back({i, "Quest "+std::to_string(i), "Complete a quest "+std::to_string(i), i<=15?MAIN:SIDE, false, 0, 1});
    quests.push_back({22, "Final Boss", "Derrote o Titã de Obsidiana", MAIN, false, 0, 1});
    for(int i=23;i<=30;i++) quests.push_back({i, "Lore "+std::to_string(i-22), "Encontre pergaminho "+std::to_string(i-22), MAIN, false, 0, 1});
}
void QuestManager::update(){ for(auto &q: quests) if(q.condition && q.condition()) q.completed=true; }
void QuestManager::complete(int id){ if(auto *q=get(id)) q->completed=true; }
Quest* QuestManager::get(int id){ for(auto &q: quests) if(q.id==id) return &q; return nullptr; }
std::vector<Quest*> QuestManager::getActive(){ std::vector<Quest*> r; for(auto &q: quests) if(!q.completed) r.push_back(&q); return r; }
