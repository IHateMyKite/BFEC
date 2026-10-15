#pragma once
#include "Hooks.h"

namespace BFEC
{
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

    // Visitor for worn items
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
    
    class KeyEventSink : public RE::BSTEventSink<RE::InputEvent*>
    {
    SINGLETONHEADER(KeyEventSink)
    public:
        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr,RE::BSTEventSource<RE::InputEvent*>*);
    };
    
    struct ShowMessageBoxArgs
    {
      RE::Actor* actor;
      RE::TESBoundObject* object;
      Hooks::AddObjectToContainerHook::MoveItemParam hookparam;
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
        bool IsTransferButtonEnabled() const;
        void AddKeySink() const;
        std::string GetMessageBoxStrPlayer(RE::Actor* a_target,RE::TESBoundObject* a_item, bool a_html);
        std::string GetMessageBoxStrNPC(RE::Actor* a_target,RE::TESBoundObject* a_item, bool a_html);
        void ShowMessageboxPlayer(ShowMessageBoxArgs a_args);
        void ShowMessageboxNPC(ShowMessageBoxArgs a_args);
        int ApplyOutfitItems(RE::Actor* a_actor);
    private:
        uint8_t EquipLeveledList(RE::Actor* a_actor, RE::TESLeveledList* a_list);
        
    private:
        RE::TESFaction* _OutfitEquippedFaction;
        RE::TESFaction* _Blacklist;
        bool _FastButtonPressed = false;
        bool _Ready = false;
    };
}