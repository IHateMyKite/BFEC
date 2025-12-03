#pragma once

namespace BFEC
{
  
  class ConfigElementBase
  {
  public:
    virtual bool      GetDataBool()   = 0;
    virtual uint8_t   GetDataUI8()    = 0;
    virtual uint32_t  GetDataUI32()   = 0;
    virtual float     GetDataFloat()  = 0;
    virtual std::string  GetDataString()  = 0;
  protected:
    void* _data = nullptr;
  };
  
  class ConfigElementGlobalData : public ConfigElementBase
  {
  public:
    ConfigElementGlobalData(RE::TESGlobal* a_gv) : ConfigElementBase()
    {
      _data = a_gv;
    }
    bool    GetDataBool()
    {
      if (_data != nullptr) return round(reinterpret_cast<RE::TESGlobal*>(_data)->value);
      return false;
    }
    uint8_t GetDataUI8()
    {
      if (_data != nullptr) return round(reinterpret_cast<RE::TESGlobal*>(_data)->value);
      else return 0U;
    }
    uint32_t GetDataUI32()
    {
      if (_data != nullptr) return round(reinterpret_cast<RE::TESGlobal*>(_data)->value);
      else return 0UL;
    }
    float   GetDataFloat()
    {
      if (_data != nullptr) return reinterpret_cast<RE::TESGlobal*>(_data)->value;
      else return 0.0F;
    }
    virtual std::string  GetDataString() {return "";}
  };
  
  class ConfigManager
  {
  SINGLETONHEADER(ConfigManager)
  public:
    void Init();
    std::shared_ptr<ConfigElementBase> GetConfig(std::string a_name);
    static bool         GetDataBool(std::string a_name,bool a_def);
    static uint8_t      GetDataUI8(std::string a_name,uint8_t a_def);
    static uint32_t     GetDataUI32(std::string a_name,uint32_t a_def);
    static float        GetDataFloat(std::string a_name,float a_def);
    static std::string  GetDataString(std::string a_name,std::string a_def);
    
  private:
    bool _Ready = false;
    std::unordered_map<std::string,std::shared_ptr<ConfigElementBase>> _elements;
  };
}