
#define ADD_ELEMENT_GLOBAL(a,b) {_elements[b] = std::shared_ptr<ConfigElementGlobalData>(new ConfigElementGlobalData(reinterpret_cast<RE::TESGlobal*>(loc_datahandler->LookupForm(a, "BetterFollowerEquipControl.esp"))));}

ADD_ELEMENT_GLOBAL(0x000803,"transfer-key")
ADD_ELEMENT_GLOBAL(0x000804,"transfer-key-toggle")
ADD_ELEMENT_GLOBAL(0x000805,"persist-follower")
ADD_ELEMENT_GLOBAL(0x000806,"follower-only")
ADD_ELEMENT_GLOBAL(0x000807,"race-check")

#undef ADD_ELEMENT_GLOBAL