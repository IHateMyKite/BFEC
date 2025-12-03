#include "List.h"

SINGLETONBODY(BFEC::ListManager)

namespace BFEC
{
  void ListManager::Init()
  {
    if (!_Ready)
    {
      LoadBlackList();
      LoadWhiteList();
      LoadRaceList();
      _Ready = true;
    }
  }
  
  void ListManager::LoadBlackList()
  {
    RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
    LoadList("BlackList",R"regex(.*\\(.*_[bB][lL][aA][cC][kK][lL][iI][sS][tT]\.txt))regex",
    [this,loc_datahandler](uint32_t a_formID,std::string a_mod)
    {
      RE::TESNPC* loc_npc = reinterpret_cast<RE::TESNPC*>(loc_datahandler->LookupForm(a_formID, a_mod));
      if (loc_npc != nullptr)
      {
        LOG("NPC {} added to blacklist",loc_npc->GetName())
        _blackList.push_back(loc_npc);
      }
    });
  }
  
  void ListManager::LoadWhiteList()
  {
    RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
    LoadList("WhiteList",R"regex(.*\\(.*_[wW][hH][iI][tT][eE][lL][iI][sS][tT]\.txt))regex",
    [this,loc_datahandler](uint32_t a_formID,std::string a_mod)
    {
      RE::TESNPC* loc_npc = reinterpret_cast<RE::TESNPC*>(loc_datahandler->LookupForm(a_formID, a_mod));
      if (loc_npc != nullptr)
      {
        LOG("NPC {} added to whitelist",loc_npc->GetName())
        _whiteList.push_back(loc_npc);
      }
    });
  }
  
  void ListManager::LoadRaceList()
  {
    RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
    LoadList("RaceList",R"regex(.*\\(.*_[rR][aA][cC][eE][lL][iI][sS][tT]\.txt))regex",
    [this,loc_datahandler](uint32_t a_formID,std::string a_mod)
    {
      RE::TESRace* loc_race = reinterpret_cast<RE::TESRace*>(loc_datahandler->LookupForm(a_formID, a_mod));
      if (loc_race != nullptr)
      {
        LOG("Race {} added to racelist",loc_race->GetName())
        _raceList.push_back(loc_race);
      }
    });
  }
  
  void ListManager::LoadList(std::string a_name, std::string a_regex, std::function<void(uint32_t,std::string)> a_fun)
  {
    RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
    if (!loc_datahandler) return;
    
    std::string loc_ListsPath = std::filesystem::current_path().string() + "\\Data\\skse\\plugins\\BFEC";
    std::regex loc_regex(a_regex);
    for (const auto & entry : std::filesystem::directory_iterator(loc_ListsPath))
    {
      std::string loc_path = entry.path().string();
      if (entry.is_regular_file() && std::regex_match(loc_path,loc_regex))
      {
        LOG("Checking {} file {}",a_name,loc_path)
        std::fstream loc_file(loc_path,std::ios::in);
        
        if (loc_file.is_open())
        {
          std::string loc_entity = "";
          while (std::getline(loc_file, loc_entity,'\n')) // Get next line from file
          {
            if (loc_entity == "") continue;
            
            std::regex loc_Lregex(R"regex(.*\((.+\.\w{3})\|(\w{1,8})\).*)regex");
            auto loc_mod        = std::regex_replace(loc_entity, loc_Lregex, "$1",std::regex_constants::format_no_copy); // Mod file name
            auto loc_formIDStr  = std::regex_replace(loc_entity, loc_Lregex, "$2",std::regex_constants::format_no_copy); // Form ID
            
            if ((loc_mod == "") || (loc_formIDStr == "")) continue;
            
            bool loc_error = false;
            uint32_t loc_FormID = 0x0U;
            
            try
            {
              loc_FormID = stoi(loc_formIDStr, nullptr, 16);
            }
            catch (const std::invalid_argument & e)
            {
              LOG("Error during parsing of {} for {} FormID: {}",loc_formIDStr,a_name,e.what())
              loc_error = true;
            }
            catch (const std::out_of_range & e)
            {
              LOG("Error during parsing of {} for {} FormID: {}",loc_formIDStr,a_name,e.what())
              loc_error = true;
            }
            
            loc_FormID &= 0x00FFFFFFUL;
            
            if (loc_error) continue;
            
            a_fun(loc_FormID,loc_mod);
          }
          loc_file.close();
        }
      }
    }
  }
  
  bool ListManager::IsOnBlackList(RE::Actor* a_actor)
  {
    if (!a_actor || !a_actor->GetActorBase()) return false;
    return std::find(_blackList.begin(), _blackList.end(), a_actor->GetActorBase()) != _blackList.end();
  }
  
  bool ListManager::IsOnWhiteList(RE::Actor* a_actor)
  {
    if (!a_actor || !a_actor->GetActorBase()) return false;
    return std::find(_whiteList.begin(), _whiteList.end(), a_actor->GetActorBase()) != _whiteList.end();
  }
  
  bool ListManager::IsOnRaceList(RE::TESRace* a_race)
  {
    if (!a_race) return false;
    return std::find(_raceList.begin(), _raceList.end(), a_race) != _raceList.end();
  }
}