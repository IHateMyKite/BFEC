#pragma once

namespace BFEC::Util
{
  using SKSE::stl::enumeration;
  using SKSE::stl::report_and_fail;
  using std::to_underlying;

  const char* ws = " \t\n\r\f\v";

  inline std::vector<std::string> Split(const std::string& a_str, std::string_view a_delimiter)
  {
    auto range = a_str | std::ranges::views::split(a_delimiter) | std::ranges::views::transform([](auto&& r) { return std::string_view(r); });
    return { range.begin(), range.end() };
  }

  inline std::string Join(const std::vector<std::string>& a_vec, std::string_view a_delimiter)
  {
    return std::accumulate(a_vec.begin(), a_vec.end(), std::string{},
      [a_delimiter](const auto& str1, const auto& str2) {
        return str1.empty() ? str2 : str1 + a_delimiter.data() + str2;
      });
  }

  inline std::string str_toupper(std::string s)
  {
    std::transform(
      s.begin(),
      s.end(),
      s.begin(),
      [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
      });
    return s;
  }

  inline std::string& rtrim(std::string& s, const char* t = ws)
  {
    s.erase(s.find_last_not_of(t) + 1);
    return s;
  }

  inline std::string& ltrim(std::string& s, const char* t = ws)
  {
    s.erase(0, s.find_first_not_of(t));
    return s;
  }

  inline std::string& trim(std::string& s, const char* t = ws)
  {
    return ltrim(rtrim(s, t), t);
  }

  template <typename T = RE::TESForm>
  inline T* GetFormFromString(const std::string& s)
  {
    if (auto form = RE::TESForm::LookupByEditorID(s)) {
      return form->As<T>();
    }

    const auto splits = Split(s, "|"sv);

    if (splits.size() != 2) {
      return nullptr;
    }

    char* p;
    const auto formId = std::strtol(splits[0].c_str(), &p, 16);

    if (*p != 0) {
      return nullptr;
    }

    return RE::TESDataHandler::GetSingleton()->LookupForm<T>(formId, splits[1]);
  }
  
  //copy of RE::InventoryChanges::IItemChangeVisitor, with full definition so it can be inherited from
  class IItemChangeVisitor
  {
  public:
    virtual ~IItemChangeVisitor(){}  // 00

    // add
    virtual RE::BSContainer::ForEachResult Visit(RE::InventoryEntryData* a_entryData) {return RE::BSContainer::ForEachResult::kContinue;}; // 01
    virtual bool ShouldVisit([[maybe_unused]] RE::InventoryEntryData* a_entryData, [[maybe_unused]] RE::TESBoundObject* a_object) { return true; }  // 02
    virtual RE::BSContainer::ForEachResult Unk_03(RE::InventoryEntryData* a_entryData, [[maybe_unused]] void* a_arg2, bool* a_arg3) // 03
    {
      *a_arg3 = true;
      return Visit(a_entryData);
    }

    RE::InventoryChanges::IItemChangeVisitor& AsNativeVisitor(){return *(RE::InventoryChanges::IItemChangeVisitor*)this;}
  };
  static_assert(sizeof(IItemChangeVisitor) == 0x8);
  
  class WornVisitor : public IItemChangeVisitor
  {
  public:
    WornVisitor(std::function<RE::BSContainer::ForEachResult(RE::InventoryEntryData*)> a_fun) : _fun(a_fun) {};

    virtual RE::BSContainer::ForEachResult Visit(RE::InventoryEntryData* a_entryData) override
    {
      return _fun(a_entryData);
    }
  private:
    std::function<RE::BSContainer::ForEachResult(RE::InventoryEntryData*)> _fun;
  };
  
  RE::TESObjectARMO* GetWornArmor(RE::Actor* a_actor, uint32_t a_mask)
  {
    if (a_actor == nullptr) return nullptr;

    RE::TESObjectARMO* loc_res = nullptr;
    auto loc_visitor = WornVisitor([&loc_res,a_mask](RE::InventoryEntryData* a_entry)
    {
      #undef GetObject
      auto loc_object = a_entry->GetObject();
      RE::TESObjectARMO* loc_armor = nullptr;
      if (loc_object != nullptr && loc_object->IsArmor()) loc_armor = static_cast<RE::TESObjectARMO*>(loc_object);

      if (loc_armor != nullptr && (loc_armor->GetSlotMask().underlying() & a_mask))
      {
        loc_res = loc_armor;
        return RE::BSContainer::ForEachResult::kStop;
      }
      else return RE::BSContainer::ForEachResult::kContinue;
    });
    a_actor->GetInventoryChanges()->VisitWornItems(loc_visitor.AsNativeVisitor());
    return loc_res;
  }
  
  std::vector<RE::TESObjectARMO*> GetAllWornArmor(RE::Actor* a_actor, uint32_t a_mask)
  {
    std::vector<RE::TESObjectARMO*> loc_res;
    
    if (a_actor == nullptr) return loc_res;

    auto loc_visitor = WornVisitor([&loc_res,a_mask](RE::InventoryEntryData* a_entry)
    {
      #undef GetObject
      auto loc_object = a_entry->GetObject();
      RE::TESObjectARMO* loc_armor = nullptr;
      if (loc_object != nullptr && loc_object->IsArmor()) loc_armor = static_cast<RE::TESObjectARMO*>(loc_object);

      if (loc_armor != nullptr && (loc_armor->GetSlotMask().underlying() & a_mask))
      {
        if (std::find(loc_res.begin(),loc_res.end(),loc_armor) == loc_res.end()) loc_res.push_back(loc_armor);
      }
      return RE::BSContainer::ForEachResult::kContinue;
    });
    a_actor->GetInventoryChanges()->VisitWornItems(loc_visitor.AsNativeVisitor());
    return loc_res;
  }
  
  bool IsArmorWorn(RE::Actor* a_actor, RE::TESObjectARMO* a_armor)
  {
    if (a_actor == nullptr) return false;
    if (a_armor == nullptr) return false;
    
    bool loc_res = false;
    auto loc_visitor = WornVisitor([&loc_res,a_armor](RE::InventoryEntryData* a_entry)
    {
      #undef GetObject
      auto loc_object = a_entry->GetObject();
      RE::TESObjectARMO* loc_armor = nullptr;
      if (loc_object != nullptr && loc_object->IsArmor()) loc_armor = static_cast<RE::TESObjectARMO*>(loc_object);

      if (loc_armor != nullptr && (loc_armor == a_armor))
      {
        loc_res = true;
        return RE::BSContainer::ForEachResult::kStop;
      }
      else return RE::BSContainer::ForEachResult::kContinue;
    });
    a_actor->GetInventoryChanges()->VisitWornItems(loc_visitor.AsNativeVisitor());
    return loc_res;
  }
  
  std::vector<RE::TESObjectARMO*> GetConflictingArmor(RE::Actor* a_actor, RE::TESObjectARMO* a_armor)
  {
    std::vector<RE::TESObjectARMO*> loc_res;
    
    if (a_actor == nullptr) return loc_res;
    if (a_armor == nullptr) return loc_res;
    
    const uint32_t loc_mask = a_armor->GetSlotMask().underlying();
    
    if (loc_mask > 0)
    {
      loc_res = GetAllWornArmor(a_actor,loc_mask);
    }
    
    return loc_res;
  }
  
  using Item = std::pair<RE::TESBoundObject* const, std::pair<int32_t, std::unique_ptr<RE::InventoryEntryData>>>;
  
  uint16_t GetItemCount(RE::Actor* a_actor, RE::TESBoundObject* a_item)
  {
      //LOG("GetItemCount called")
      
      uint16_t loc_res =  0U;
      
      auto loc_ic = a_actor->GetInventoryChanges();
      
      if (loc_ic)
      {
        loc_res = loc_ic->GetItemCount(a_item);
      }
      
      return loc_res;
  }
}