#pragma once
#include "Hooks.h"

namespace BFEC
{
    class EquipManager
    {
    SINGLETONHEADER(EquipManager)
    public:
        void Init();
        bool UpdateOutfit(RE::TESNPC* a_npc, RE::Actor* a_actor);
        bool AddObjectToContainer(RE::Actor* a_actor, RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraList, int32_t a_count, RE::TESObjectREFR* a_fromRefr, Hooks::AddObjectToContainerHook::MoveItemParam a_param);
    private:
        RE::TESFaction* _OutfitEquippedFaction;
        RE::TESFaction* _Blacklist;
        bool _Ready = false;
    };
}