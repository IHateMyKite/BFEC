#include "Updater.h"
#include "EquipManager.h"

SINGLETONBODY(BFEC::UpdaterManager)

void BFEC::UpdaterManager::Init()
{
    _timers.clear();
    if (!_Ready)
    {
        LOG("UpdaterManager::Init called")
        _Ready = true;
        
        REL::Relocation<std::uintptr_t> vtbl_character{RE::Character::VTABLE[0]};
        //UpdateCharacter_old = vtbl_character.write_vfunc(REL::Module::GetRuntime() != REL::Module::Runtime::VR ? 0x0AD : 0x0AF, UpdateCharacter);
    }
}

void BFEC::UpdaterManager::UpdateActor(RE::Actor* a_actor, float a_delta)
{
    if (a_delta <= 0.0f) a_delta = 1.0f/60.0f;
    
    
    _timers[a_actor] += a_delta;
    
    //LOG("UpdateActor({},{}) - Timer = {}",a_actor->GetName(),a_delta,_timers[a_actor])
    
    if (_timers[a_actor] >= UPDATE_TIMER)
    {
        EquipManager::GetSingleton()->UpdateOutfit(a_actor->GetActorBase(),a_actor);
        _timers[a_actor] = 0.0f;
    }
}

void BFEC::UpdaterManager::UpdateCharacter(RE::Actor* a_actor, float a_delta)
{
    //LOG("UpdateCharacter called")
    UpdaterManager::GetSingleton()->UpdateActor(a_actor,a_delta);
    UpdateCharacter_old(a_actor,a_delta);
}