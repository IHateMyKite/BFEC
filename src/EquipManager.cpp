#include "EquipManager.h"
#include "MessageBox.h"
#include "Hooks.h"
#include "Config.h"
#include "List.h"
#include "Util.h"

SINGLETONBODY(BFEC::EquipManager);
SINGLETONBODY(BFEC::KeyEventSink)

#define VALIDATE_START()  if (
#define VALIDATE_PTR(x)   x != nullptr &&
#define VALIDATE_END()    true) {_Ready = true;}
        
namespace BFEC
{
  void EquipManager::Init()
  {
    if (!_Ready)
    {
      RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
      if (loc_datahandler)
      {
        _OutfitEquippedFaction = reinterpret_cast<RE::TESFaction*>(loc_datahandler->LookupForm(0x000800, "BetterFollowerEquipControl.esp"));
        _Blacklist = reinterpret_cast<RE::TESFaction*>(loc_datahandler->LookupForm(0x000801, "BetterFollowerEquipControl.esp"));
        
        VALIDATE_START()
        VALIDATE_PTR(_OutfitEquippedFaction)
        VALIDATE_PTR(_Blacklist)
        VALIDATE_END()
      }
    }
    if (!_Ready) LOG("Failed to Init EquipManager")
  }
  
  bool EquipManager::UpdateOutfit(RE::TESNPC* a_npc, RE::Actor* a_actor)
  {
    if (!_Ready) return true;
    
    LOG("EquipManager::UpdateOutfit({},{}) called",a_npc ? a_npc->GetName() : "NONE", a_actor ? a_actor->GetName() : "NONE")
    
    if (a_actor == nullptr || a_npc == nullptr) return true;
    
    if (a_actor->IsDead() || a_actor->IsChild())
    {
      LOG("EquipManager::UpdateOutfit({},{}) - Actor is child or dead -> skipping",a_npc->GetName(),a_actor->GetName())
      return true;
    }
    
    const bool loc_whitelisted = ListManager::GetSingleton()->IsOnWhiteList(a_actor);
    
    if (ConfigManager::GetDataBool("race-check",true) && !loc_whitelisted)
    {
      if (!a_npc->GetRace()->GetPlayable() && !ListManager::GetSingleton()->IsOnRaceList(a_npc->GetRace())) 
      {
        LOG("EquipManager::UpdateOutfit({},{}) - Actor is of invalid race -> skipping",a_npc->GetName(),a_actor->GetName())
        return true;
      }
    }
    
    if (ListManager::GetSingleton()->IsOnBlackList(a_actor) || a_actor->IsInFaction(_Blacklist)) 
    {
      LOG("EquipManager::UpdateOutfit({},{}) - Actor is on BlackList -> skipping",a_npc->GetName(),a_actor->GetName())
      return true;
    }
    
    if (ConfigManager::GetDataBool("persist-follower",false) && a_actor->IsInFaction(_OutfitEquippedFaction))
    {
      // Actor already proccessed, skip
      return false;
    }
    
    const bool loc_isFollower =  !ConfigManager::GetDataBool("follower-only",true) || a_actor->GetActorRuntimeData().boolBits.any(RE::Actor::BOOL_BITS::kPlayerTeammate) || loc_whitelisted;
    
    if (loc_isFollower)
    {
      if (a_npc->defaultOutfit)
      {
        uint8_t loc_equippedItems = 0U;
        a_npc->defaultOutfit->ForEachItem(
        [&](RE::TESForm* a_item) -> RE::BSContainer::ForEachResult
        {
          if (a_item->IsArmor())
          {
            auto loc_armor = a_item->As<RE::TESObjectARMO>();
            LOG("Equipping {} on {}",a_item->GetName(),a_actor->GetName())
            //if (Util::GetItemCount(a_actor,loc_armor) == 0U)
            {
              a_actor->AddObjectToContainer(loc_armor,nullptr,1,nullptr);
            }
            RE::ActorEquipManager::GetSingleton()->EquipObject(a_actor,loc_armor);
            loc_equippedItems++;
          }
          else if (a_item->Is(RE::FormType::LeveledItem))
          {
            // Item in outfit is leveled list. Process
            auto loc_leveledlist = a_item->As<RE::TESLeveledList>();
            loc_equippedItems += EquipLeveledList(a_actor,loc_leveledlist);
          }
          return RE::BSContainer::ForEachResult::kContinue;
        });
        if (loc_equippedItems > 0) a_actor->AddToFaction(_OutfitEquippedFaction,1);
        else return true;
      }
      return false;
    }
    
    return true;
  }
  
  bool EquipManager::AddObjectToContainer(RE::Actor* a_actor, RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraList, int32_t a_count, RE::TESObjectREFR* a_fromRefr, Hooks::AddObjectToContainerHook::MoveItemParam a_param)
  {
    LOG("AddObjectToContainer({},{},{}) called",a_actor->GetName(),a_object->GetName(),a_fromRefr ? a_fromRefr->GetName() : "NONE")
    if (!_Ready) return false;
    
    if (RE::UI::GetSingleton())
    {
      RE::GPtr<RE::ContainerMenu> loc_invMenu = RE::UI::GetSingleton()->GetMenu<RE::ContainerMenu>();
      if (!loc_invMenu.get()) 
      {
        LOG("Inventory not open, skipping")
        return false;
      }
    }
    else 
      return false;
    
    RE::Actor* loc_target = a_actor;
    RE::Actor* loc_source = (a_fromRefr && a_fromRefr->Is(RE::FormType::ActorCharacter)) ? reinterpret_cast<RE::Actor*>(a_fromRefr) : nullptr;
    
    if (!loc_target || !loc_source) return false;

    const bool loc_playerPresent = (a_actor->IsPlayerRef() && !a_fromRefr->IsPlayerRef()) || (!a_actor->IsPlayerRef() && a_fromRefr->IsPlayerRef());
    
    if (!loc_playerPresent) return false;
    
    RE::Actor* loc_npc = nullptr;
    if (loc_target->IsPlayerRef()) loc_npc = loc_source;
    else loc_npc = loc_target;
    
    if (loc_npc->IsDead() || loc_npc->IsChild()) return false;
    
    if (true && loc_npc->GetActorBase()) // TODO: Add switch
    {
      auto loc_race = loc_npc->GetActorBase()->GetRace();
      if (!loc_race->GetPlayable() && !ListManager::GetSingleton()->IsOnRaceList(loc_race)) return false;
    }
    
    if (ListManager::GetSingleton()->IsOnBlackList(loc_npc) || loc_npc->IsInFaction(_Blacklist))
    {
      LOG("AddObjectToContainer({},{},{}) - Actor is on BlackList -> skipping",a_actor->GetName(),a_object->GetName(),a_fromRefr ? a_fromRefr->GetName() : "NONE")
      return false;
    }
    
    const bool loc_followerPresent = !ConfigManager::GetDataBool("follower-only",true) || loc_npc->GetActorRuntimeData().boolBits.any(RE::Actor::BOOL_BITS::kPlayerTeammate) || ListManager::GetSingleton()->IsOnWhiteList(a_actor);
    
    if (!loc_followerPresent) return false;
    
    if ((a_object->IsArmor() && (reinterpret_cast<RE::TESObjectARMO*>(a_object)->GetSlotMask().underlying() > 0)) || a_object->IsWeapon() || a_object->Is(RE::FormType::Light))
    {
      // == Check all possible combinations
      // Move object from Player to Follower
      if (loc_source->IsPlayerRef())
      {
        if (_FastButtonPressed && IsTransferButtonEnabled())
        {
          if (Hooks::AddObjectToContainerHook::func(a_param.menu,a_param.object,a_param.count,a_param.mode))
          {
            Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_param.menu);
          }
        }
        else
        {
          ShowMessageboxPlayer({loc_target,a_object,a_param});
        }
      }
      // Move object from follower to Player
      else
      {
        if (_FastButtonPressed && IsTransferButtonEnabled())
        {
          if (Hooks::AddObjectToContainerHook::func(a_param.menu,a_param.object,a_param.count,1))
          {
            Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_param.menu);
          }
        }
        else
        {
          ShowMessageboxNPC({loc_source,a_object,a_param});
        }
      }
      return true;
    }
    return false;
  }
  
  void EquipManager::ShowMessageboxPlayer(ShowMessageBoxArgs a_args)
  {
    static std::vector<std::string> loc_buttons = {"EQUIP","MOVE","BACK"};
    // Check if player wants to just move the item, or move it and also equip it
    std::erase_if(loc_buttons, [](const std::string& text) { return text.empty(); });
    MessageboxManager::GetSingleton()->ShowMessageBox(std::format("What do you want to do with {}?",a_args.object->GetName()), loc_buttons, [a_args,this](uint32_t result) 
    {
      LOG("Button selected {}",result)
      switch (result)
      {
        case 0U:
        // EQUIP
        Hooks::ContainerMenuPostDisplay::AddCallout([a_args]
        {
          if (Hooks::AddObjectToContainerHook::func(a_args.hookparam.menu,a_args.hookparam.object,a_args.hookparam.count,a_args.hookparam.mode))
          {
            Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_args.hookparam.menu);
          }
          SKSE::GetTaskInterface()->AddTask([a_args]
          {
            RE::ActorEquipManager::GetSingleton()->EquipObject(a_args.actor,a_args.object);
            a_args.actor->Update3DModel();
            Hooks::ContainerMenuPostDisplay::AddCallout([a_args]
            {
              Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_args.hookparam.menu);
            });
          });
        });
        break;
        case 2U:
        // BACK
        break;
        default:
        Hooks::ContainerMenuPostDisplay::AddCallout([a_args]
        {
          if (Hooks::AddObjectToContainerHook::func(a_args.hookparam.menu,a_args.hookparam.object,a_args.hookparam.count,a_args.hookparam.mode))
          {
            Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_args.hookparam.menu);
          }
        });
        break;
      }
    },false);
  }
  
  void EquipManager::ShowMessageboxNPC(ShowMessageBoxArgs a_args)
  {
    LOG("Opening menu for Follower->Player")
    // Check if Player wants to just move item, or only equip it
    static std::vector<std::string> loc_buttons = {"EQUIP","UNEQUIP","MOVE","BACK"};
    // Check if player wants to just move the item, or move it and also equip it
    std::erase_if(loc_buttons, [](const std::string& text) { return text.empty(); });
    
    const std::string loc_msg = GetMessageBoxStrNPC(a_args.actor,a_args.object,false);
    
    MessageboxManager::GetSingleton()->ShowMessageBox(loc_msg, loc_buttons, [a_args,this](uint32_t result) 
    {
      LOG("Button selected {}",result)
      switch (result)
      {
        case 0U:
        // EQUIP
        SKSE::GetTaskInterface()->AddTask([a_args]
        {
          RE::ActorEquipManager::GetSingleton()->EquipObject(a_args.actor,a_args.object);
          a_args.actor->Update3DModel();
          Hooks::ContainerMenuPostDisplay::AddCallout([a_args]
          {
            Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_args.hookparam.menu);
          });
        });
        break;
        case 1U:
        // UNEQUIP
        SKSE::GetTaskInterface()->AddTask([a_args]
        {
          RE::ActorEquipManager::GetSingleton()->UnequipObject(a_args.actor,a_args.object);
          a_args.actor->Update3DModel();
          Hooks::ContainerMenuPostDisplay::AddCallout([a_args]
          {
            Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_args.hookparam.menu);
          });
        });
        break;
        case 3U:
        // BACK
        break;
        default:
        Hooks::ContainerMenuPostDisplay::AddCallout([a_args]
        {
          if (Hooks::AddObjectToContainerHook::func(a_args.hookparam.menu,a_args.hookparam.object,a_args.hookparam.count,1))
          {
            Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_args.hookparam.menu);
          }
        });
        break;
      }

    },false);
  }
  
  void EquipManager::SetFastButtonPressed(bool a_value)
  {
    _FastButtonPressed = a_value;
  }
  
  uint32_t EquipManager::GetTransferKey() const
  {
    return ConfigManager::GetDataUI32("transfer-key",0);
  }
  
  bool EquipManager::IsTransferButtonEnabled() const
  {
    return ConfigManager::GetDataBool("transfer-key-toggle",false);
  }
  
  // Danger: Recursion
  uint8_t EquipManager::EquipLeveledList(RE::Actor* a_actor, RE::TESLeveledList* a_list)
  {
    uint8_t loc_res = 0U;
    
    if (!a_list) return loc_res;
    
    RE::BSScrapArray<RE::CALCED_OBJECT> loc_array;
    //loc_array.reserve(10);
    a_list->CalculateCurrentFormList(a_actor->GetLevel(), 1, loc_array, 0, true);
    
    for (auto && it : loc_array)
    {
      if (it.form != nullptr)
      {
        if (it.form->IsArmor())
        {
          auto loc_armor = it.form->As<RE::TESObjectARMO>();
          LOG("Equipping {} on {}",it.form->GetName(),a_actor->GetName())
          //if (Util::GetItemCount(a_actor,loc_armor) == 0U)
          {
            a_actor->AddObjectToContainer(loc_armor,nullptr,1,nullptr);
          }
          RE::ActorEquipManager::GetSingleton()->EquipObject(a_actor,loc_armor);
          loc_res++;
        }
        else if (it.form->Is(RE::FormType::LeveledItem))
        {
          auto loc_leveledlist = it.form->As<RE::TESLeveledList>();
          loc_res += EquipLeveledList(a_actor, loc_leveledlist);
        }
      }
    }
    return loc_res;
  }
  
  void EquipManager::AddKeySink() const
  {
    RE::BSInputDeviceManager::GetSingleton()->AddEventSink(KeyEventSink::GetSingleton());
  }
  
  std::string EquipManager::GetMessageBoxStrNPC(RE::Actor* a_target,RE::TESBoundObject* a_item, bool a_html)
  {
    //if (a_html)
    //{
    //  return "";
    //}
    //else
    {
      std::string loc_res = "";
      switch (a_item->GetFormType())
      {
        case RE::FormType::Armor:
        {
          auto loc_armor = reinterpret_cast<RE::TESObjectARMO*>(a_item);
          loc_res = std::format("What do you want to do with {}?\nTarget is {}",loc_armor->GetName(),a_target->GetName());
          if (!Util::IsArmorWorn(a_target,loc_armor))
          {
            auto loc_conflict = Util::GetConflictingArmor(a_target,loc_armor);
            if (loc_conflict.size() > 0)
            {
              loc_res += "\nEquipping this armor will unequip following armor(s):";
              uint8_t loc_cnt = 1U;
              for (auto && it : loc_conflict)
              {
                loc_res += std::format("\n{}) {}",loc_cnt,it->GetName() ? it->GetName() : "-MISSING NAME-");
                loc_cnt++;
              }
            }
          }
        }
        break;
        case RE::FormType::Weapon:
        case RE::FormType::Light:
        {
          auto loc_weapon = reinterpret_cast<RE::TESObjectWEAP*>(a_item);
          loc_res = std::format("What do you want to do with {}?\nTarget is {}",loc_weapon->GetName(),a_target->GetName());
        }
        break;
        default:
        
        break;
      }
      return loc_res;
    }
  }
  
  RE::BSEventNotifyControl KeyEventSink::ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*)
  {
    if (!eventPtr) return RE::BSEventNotifyControl::kContinue;
    
    auto* event = *eventPtr;
    if (!event) return RE::BSEventNotifyControl::kContinue;

    if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton)
    {
      // Check if BFEC is installed correctly
      if (!EquipManager::GetSingleton()->IsReady()) return RE::BSEventNotifyControl::kContinue;
      
      if (!EquipManager::GetSingleton()->IsTransferButtonEnabled()) return RE::BSEventNotifyControl::kContinue;
      
      const auto*       loc_buttonEvent = event->AsButtonEvent();
      const RE::INPUT_DEVICE loc_Device = loc_buttonEvent->GetDevice();
      const uint32_t    loc_dxScanCode  = loc_buttonEvent->GetIDCode();
      const uint32_t    loc_transferCode = EquipManager::GetSingleton()->GetTransferKey();
      //LOG("Button pressed = 0x{:08X}, Fast transfer button = 0x{:08X}",loc_dxScanCode,loc_transferCode)
      if (loc_dxScanCode == loc_transferCode)
      {
        EquipManager::GetSingleton()->SetFastButtonPressed(loc_buttonEvent->IsPressed());
      }
    }
    return RE::BSEventNotifyControl::kContinue;
  }
}