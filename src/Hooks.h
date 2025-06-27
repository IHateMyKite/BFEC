#pragma once

namespace BFEC
{
    namespace Hooks
    {
        class UpdateNPCOutfitHook
        {
        public:
            static inline void Install(void);
            static void thunk(RE::TESNPC* a_npc, RE::Actor* a_actor, __int64 a_unk1, bool a_CheckDead, int a_unk2, char a_unk3);
            static inline REL::Relocation<decltype(thunk)> func;
        };
        
        class AddObjectToContainerHook
        {
        public:
            static inline void Install(void);
            static bool thunk(RE::ContainerMenu* a_this, RE::TESBoundObject** a_object, uint16_t a_count, uint8_t a_unk4);
            //__int64 a1, __int64 *a_actor, unsigned int a3, unsigned __int8 a4
            static inline REL::Relocation<decltype(thunk)> func;
            static uint64_t UpdateContainerMenu(RE::ContainerMenu* a_this);
            
            
            struct MoveItemParam
            {
              RE::ContainerMenu*    menu;
              RE::TESBoundObject**  object;
              uint16_t              count;
              uint8_t               mode;
            };
        };
        
        class ContainerMenuPostDisplay
        {
        public:
            using CalloutFn = void(void);
            static inline void Install(void);
            static void thunk(RE::ContainerMenu* a_this);
            static inline REL::Relocation<decltype(thunk)> func;
            static void AddCallout(std::function<CalloutFn> a_callout);
        private:
            static std::mutex _mutex;
            static std::function<CalloutFn> _callout;
        };
    }
    
    
    class HookManager
    {
    SINGLETONHEADER(HookManager)
    public:
        void Init();
    private:
        bool _Ready = false;
    };
}