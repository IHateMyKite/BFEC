#include "EquipManager.h"
#include "MessageBox.h"
#include "Hooks.h"

SINGLETONBODY(BFEC::EquipManager);
SINGLETONBODY(BFEC::KeyEventSink)

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
        _TransferKey = reinterpret_cast<RE::TESGlobal*>(loc_datahandler->LookupForm(0x000803, "BetterFollowerEquipControl.esp"));
        if (_OutfitEquippedFaction && _Blacklist && _TransferKey) _Ready = true;
      }
    }
    if (!_Ready) LOG("Failed to Init EquipManager")
  }
  
  bool EquipManager::UpdateOutfit(RE::TESNPC* a_npc, RE::Actor* a_actor)
  {
    if (!_Ready) return true;
    
    if (a_actor == nullptr) return true;
    
    LOG("EquipManager::UpdateOutfit({}) called",a_actor->GetName())
    
    if (a_actor->IsInFaction(_OutfitEquippedFaction))
    {
      // Actor already proccessed, skip
      return false;
    }
    
    const bool loc_isFollower = a_actor->GetActorRuntimeData().boolBits.any(RE::Actor::BOOL_BITS::kPlayerTeammate);
    
    if (loc_isFollower && a_npc && a_actor && !a_actor->IsInFaction(_Blacklist))
    {
      if (a_npc->defaultOutfit)
      {
        a_npc->defaultOutfit->ForEachItem(
        [&](RE::TESForm* a_item) -> RE::BSContainer::ForEachResult
        {
          if (a_item->IsArmor())
          {
            LOG("Equipping {}",a_item->GetName())
            a_actor->AddObjectToContainer(a_item->As<RE::TESObjectARMO>(),nullptr,1,nullptr);
            RE::ActorEquipManager::GetSingleton()->EquipObject(a_actor,a_item->As<RE::TESObjectARMO>());
          }
          return RE::BSContainer::ForEachResult::kContinue;
        });
      }
      a_actor->AddToFaction(_OutfitEquippedFaction,1);
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
    else return false;
    
    RE::Actor* loc_target = a_actor;
    RE::Actor* loc_source = (a_fromRefr && a_fromRefr->Is(RE::FormType::ActorCharacter)) ? reinterpret_cast<RE::Actor*>(a_fromRefr) : nullptr;
    
    if (!loc_target || !loc_source) return false;

    const bool loc_playerPresent = (a_actor->IsPlayerRef() && !a_fromRefr->IsPlayerRef()) || (!a_actor->IsPlayerRef() && a_fromRefr->IsPlayerRef());
    
    if (!loc_playerPresent) return false;
    
    const bool loc_followerPresent = loc_target->GetActorRuntimeData().boolBits.any(RE::Actor::BOOL_BITS::kPlayerTeammate) || loc_source->GetActorRuntimeData().boolBits.any(RE::Actor::BOOL_BITS::kPlayerTeammate);
    
    if (!loc_followerPresent) return false;
    
    if ((a_object->IsArmor() && ((uint32_t)reinterpret_cast<RE::TESObjectARMO*>(a_object)->GetSlotMask() > 0)) || a_object->IsWeapon() || a_object->Is(RE::FormType::Light))
    {
      // == Check all possible combinations
      // Move object from Player to Follower
      if (loc_source->IsPlayerRef())
      {
        if (_FastButtonPressed)
        {
          if (Hooks::AddObjectToContainerHook::func(a_param.menu,a_param.object,a_param.count,a_param.mode))
          {
            Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_param.menu);
          }
        }
        else
        {
          static std::vector<std::string> loc_buttons = {"EQUIP","MOVE","BACK"};
          // Check if player wants to just move the item, or move it and also equip it
          std::erase_if(loc_buttons, [](const std::string& text) { return text.empty(); });
          MessageboxManager::GetSingleton()->ShowMessageBox(std::format("What do you want to do with {}?",a_object->GetName()), loc_buttons, [loc_target,a_object,a_param,this](uint32_t result) 
          {
            LOG("Button selected {}",result)
            switch (result)
            {
              case 0U:
              // EQUIP
              Hooks::ContainerMenuPostDisplay::AddCallout([loc_target,a_object,a_param]
              {
                if (Hooks::AddObjectToContainerHook::func(a_param.menu,a_param.object,a_param.count,a_param.mode))
                {
                  Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_param.menu);
                }
                SKSE::GetTaskInterface()->AddTask([loc_target,a_object,a_param]
                {
                  RE::ActorEquipManager::GetSingleton()->EquipObject(loc_target,a_object);
                  loc_target->Update3DModel();
                  Hooks::ContainerMenuPostDisplay::AddCallout([a_param]
                  {
                    Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_param.menu);
                  });
                });
              });
              break;
              case 2U:
              // BACK
              break;
              default:
              Hooks::ContainerMenuPostDisplay::AddCallout([a_param]
              {
                if (Hooks::AddObjectToContainerHook::func(a_param.menu,a_param.object,a_param.count,a_param.mode))
                {
                  Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_param.menu);
                }
              });
              break;
            }

          },false);
        }
      }
      // Move object from follower to Player
      else
      {
        if (_FastButtonPressed)
        {
          if (Hooks::AddObjectToContainerHook::func(a_param.menu,a_param.object,a_param.count,1))
          {
            Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_param.menu);
          }
        }
        else
        {
          LOG("Opening menu for Follower->Player")
          // Check if Player wants to just move item, or only equip it
          static std::vector<std::string> loc_buttons = {"EQUIP","UNEQUIP","MOVE","BACK"};
          // Check if player wants to just move the item, or move it and also equip it
          std::erase_if(loc_buttons, [](const std::string& text) { return text.empty(); });
          MessageboxManager::GetSingleton()->ShowMessageBox(std::format("What do you want to do with {}?",a_object->GetName()), loc_buttons, [loc_source,a_object,a_param,this](uint32_t result) 
          {
            LOG("Button selected {}",result)
            switch (result)
            {
              case 0U:
              // EQUIP
              SKSE::GetTaskInterface()->AddTask([loc_source,a_object,a_param]
              {
                RE::ActorEquipManager::GetSingleton()->EquipObject(loc_source,a_object);
                loc_source->Update3DModel();
                Hooks::ContainerMenuPostDisplay::AddCallout([a_param]
                {
                  Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_param.menu);
                });
              });
              break;
              case 1U:
              // UNEQUIP
              SKSE::GetTaskInterface()->AddTask([loc_source,a_object,a_param]
              {
                RE::ActorEquipManager::GetSingleton()->UnequipObject(loc_source,a_object);
                loc_source->Update3DModel();
                Hooks::ContainerMenuPostDisplay::AddCallout([a_param]
                {
                  Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_param.menu);
                });
              });
              break;
              case 3U:
              // BACK
              break;
              default:
              Hooks::ContainerMenuPostDisplay::AddCallout([a_param]
              {
                if (Hooks::AddObjectToContainerHook::func(a_param.menu,a_param.object,a_param.count,1))
                {
                  Hooks::AddObjectToContainerHook::UpdateContainerMenu(a_param.menu);
                }
              });
              break;
            }

          },false);
        }
      }
      return true;
    }
    return false;
  }
  
  void EquipManager::SetFastButtonPressed(bool a_value)
  {
    _FastButtonPressed = a_value;
  }
  
  uint32_t EquipManager::GetTransferKey() const
  {
    return round(_TransferKey->value);
  }
  
  void EquipManager::AddKeySink() const
  {
    RE::BSInputDeviceManager::GetSingleton()->AddEventSink(KeyEventSink::GetSingleton());
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