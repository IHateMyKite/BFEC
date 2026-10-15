#pragma once

constexpr std::string_view PapyrusClass = "BFEC";

namespace
{

}

namespace BFEC::Papyrus
{
    inline bool RegisterFunctions(RE::BSScript::IVirtualMachine* vm)
    {
        #define REGISTERPAPYRUSFUNC(name) vm->RegisterFunction(#name, PapyrusClass, name);

        return true;
    }
}