#pragma once

namespace BFEC
{
  class ListManager
  {
  SINGLETONHEADER(ListManager)
  public:
    void Init();
    bool IsOnBlackList(RE::Actor* a_actor);
    bool IsOnWhiteList(RE::Actor* a_actor);
    bool IsOnRaceList(RE::TESRace* a_race);
  private:
    void LoadBlackList();
    void LoadWhiteList();
    void LoadRaceList();
    void LoadList(std::string a_name, std::string a_regex, std::function<void(uint32_t,std::string)> a_fun);
    bool _Ready = false;
    std::vector<RE::TESNPC*> _blackList;
    std::vector<RE::TESNPC*> _whiteList;
    std::vector<RE::TESRace*> _raceList;
  };
  
}