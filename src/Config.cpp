#include "Config.h"

SINGLETONBODY(BFEC::ConfigManager)


void BFEC::ConfigManager::Init()
{
  if (!_Ready)
  {
    RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
    if (loc_datahandler)
    {
      #include "Config.inc"
    }
    _Ready = true;
  }
}

std::shared_ptr<BFEC::ConfigElementBase> BFEC::ConfigManager::GetConfig(std::string a_name)
{
  return _elements[a_name];
}

bool    BFEC::ConfigManager::GetDataBool(std::string a_name,bool a_def)
{
  auto loc_ptr = BFEC::ConfigManager::GetSingleton()->GetConfig(a_name);
  if (loc_ptr != nullptr) return loc_ptr->GetDataBool();
  else return a_def;
}

uint8_t  BFEC::ConfigManager::GetDataUI8(std::string a_name,uint8_t a_def)
{
  auto loc_ptr = BFEC::ConfigManager::GetSingleton()->GetConfig(a_name);
  if (loc_ptr != nullptr) return loc_ptr->GetDataUI8();
  else return a_def;
}
uint32_t  BFEC::ConfigManager::GetDataUI32(std::string a_name,uint32_t a_def)
{
  auto loc_ptr = BFEC::ConfigManager::GetSingleton()->GetConfig(a_name);
  if (loc_ptr != nullptr) return loc_ptr->GetDataUI32();
  else return a_def;
}
float     BFEC::ConfigManager::GetDataFloat(std::string a_name,float a_def)
{
  auto loc_ptr = BFEC::ConfigManager::GetSingleton()->GetConfig(a_name);
  if (loc_ptr != nullptr) return loc_ptr->GetDataFloat();
  else return a_def;
}
std::string  BFEC::ConfigManager::GetDataString(std::string a_name,std::string a_def)
{
  auto loc_ptr = BFEC::ConfigManager::GetSingleton()->GetConfig(a_name);
  if (loc_ptr != nullptr) return loc_ptr->GetDataString();
  else return a_def;
}