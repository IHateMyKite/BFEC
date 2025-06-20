#include "Hooks.h"
#include "EquipManager.h"
#include <detours/detours.h>

SINGLETONBODY(BFEC::HookManager);

namespace BFEC
{
  namespace Hooks
  {
    inline void UpdateNPCOutfitHook::Install()
    {
      // 1403BD380 = 418622
      const uintptr_t loc_Address = REL::VariantID(24234, 418622,0x0 /* TODO */).address();
      func = (decltype(func))loc_Address;
      
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach(&(PVOID&)func, (PBYTE)&thunk);
      if (DetourTransactionCommit() != NO_ERROR)
      {
      ERROR("Failed to install UpdateNPCOutfitHook");
      }
      else
      {
      LOG("UpdateNPCOutfitHook installed")
      }
    }
    
    inline void AddObjectToContainerHook::Install()
    {
      // 140678F90 = 37525
      //const uintptr_t loc_Address = REL::VariantID(36525, 37525,0x0 /* TODO */).address();
      
      // 0x1408FC110 = 51141
      const uintptr_t loc_Address = REL::VariantID(50212,51141,0x0 /* TODO */).address();
      
      func = (decltype(func))loc_Address;
      
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach(&(PVOID&)func, (PBYTE)&thunk);
      if (DetourTransactionCommit() != NO_ERROR)
      {
      ERROR("Failed to install AddObjectToContainerHook");
      }
      else
      {
      LOG("AddObjectToContainerHook installed")
      }
    }
    
    void UpdateNPCOutfitHook::thunk(RE::TESNPC* a_npc, RE::Actor* a_actor, __int64 a_unk1, bool a_CheckDead, int a_unk2, char a_unk3)
    {
      if (EquipManager::GetSingleton()->UpdateOutfit(a_npc,a_actor))
      {
        func(a_npc,a_actor,a_unk1,a_CheckDead,a_unk2,a_unk3);
      }
    }
    
    bool AddObjectToContainerHook::thunk(RE::ContainerMenu* a_this, RE::TESBoundObject** a_object, uint16_t a_count, uint8_t a_mode)
    {
      LOG("AddObjectToContainerHook called")
      
      auto loc_targetRefHandle = a_this->GetTargetRefHandle();
      auto loc_target = RE::Actor::LookupByHandle(loc_targetRefHandle);
      
      LOG("Mode     = {}",(uintptr_t)a_this->GetContainerMode())
      LOG("Target   = {}",loc_target ? loc_target->GetName() : "NONE")
      LOG("Object   = {}",(*a_object)->GetName())
      LOG("Count    = {}",a_count)
      LOG("From/To  = {}",a_mode)
      
      if ((a_this->GetContainerMode() == RE::ContainerMenu::ContainerMode::kNPCMode) && a_count && loc_target)
      {
        bool loc_res;
        
        auto loc_extradatalist = a_this->GetRuntimeData().itemList->GetSelectedItem()->data.objDesc->extraLists;
        
        //LOG("Extra data count = {}",loc_extradata->size())
        
        RE::ExtraDataList* loc_extradata = nullptr;
        
        if (loc_extradatalist)
        {
          for (auto&& it : *loc_extradatalist)
          {
            LOG("Extra list found")
            loc_extradata = it; // use the first one in tree
            break;
          }
        }
        
        // NPC mode
        if (a_mode == 0x0U) // Player -> NPC
        {
          loc_res = EquipManager::GetSingleton()->AddObjectToContainer(loc_target.get(),*a_object,loc_extradata,a_count,RE::PlayerCharacter::GetSingleton(),{a_this,a_object,a_count,a_mode});
          
        }
        else if (a_mode == 0x01U) // NPC -> Player
        {
          loc_res = EquipManager::GetSingleton()->AddObjectToContainer(RE::PlayerCharacter::GetSingleton(),*a_object,loc_extradata,a_count,loc_target.get(),{a_this,a_object,a_count,a_mode});
        }
        
        if (loc_res)
        {
          return false;
        }
        else
        {
          return func(a_this,a_object,a_count,a_mode);
        }
      }
      else
      {
        return func(a_this,a_object,a_count,a_mode);
      }
    }
    
    uint64_t AddObjectToContainerHook::UpdateContainerMenu(RE::ContainerMenu* a_this)
    {
      REL::Relocation<uint64_t(RE::ItemList*,RE::Actor*)> UpdateMenu{ RELOCATION_ID(50099, 51031)};
      return UpdateMenu(a_this->GetRuntimeData().itemList,RE::PlayerCharacter::GetSingleton());
    }
  }
  
  
  
  void HookManager::Init()
  {
    if (!_Ready)
    {
      _Ready = true;
      
      Hooks::UpdateNPCOutfitHook::Install();
      Hooks::AddObjectToContainerHook::Install();
      
    }
  }
}