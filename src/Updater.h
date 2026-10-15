#pragma once

#define UPDATE_TIMER 1.0f

namespace BFEC
{
    class UpdaterManager
    {
    SINGLETONHEADER(UpdaterManager)
    public:
        void Init();
        void UpdateActor(RE::Actor* a_actor, float a_delta);
        static void UpdateCharacter(RE::Actor* a_actor, float a_delta);
        
    private:
        bool _Ready = false;
        std::unordered_map<RE::Actor*,float> _timers;
        inline static REL::Relocation<decltype(UpdateCharacter)> UpdateCharacter_old;
    };
}