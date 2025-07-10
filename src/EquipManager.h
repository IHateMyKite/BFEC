#pragma once
#include "Hooks.h"

namespace BFEC
{
    class KeyEventSink : public RE::BSTEventSink<RE::InputEvent*>
    {
    SINGLETONHEADER(KeyEventSink)
    public:
        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr,RE::BSTEventSource<RE::InputEvent*>*);
    };
  
    class EquipManager
    {
    SINGLETONHEADER(EquipManager)
    public:
        void Init();
        bool UpdateOutfit(RE::TESNPC* a_npc, RE::Actor* a_actor);
        bool AddObjectToContainer(RE::Actor* a_actor, RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraList, int32_t a_count, RE::TESObjectREFR* a_fromRefr, Hooks::AddObjectToContainerHook::MoveItemParam a_param);
        void SetFastButtonPressed(bool a_value);
        uint32_t GetTransferKey() const;
        bool IsReady() const { return _Ready;};
        void AddKeySink() const;
    private:
        RE::TESFaction* _OutfitEquippedFaction;
        RE::TESFaction* _Blacklist;
        RE::TESGlobal*  _TransferKey;
        bool _FastButtonPressed = false;
        bool _Ready = false;
    };
}