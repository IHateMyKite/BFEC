#include "Papyrus.h"
#include "Hooks.h"
#include "EquipManager.h"
#include "MessageBox.h"

using namespace PAR;

void InitializeLog([[maybe_unused]] spdlog::level::level_enum a_level = spdlog::level::info)
{
#ifndef NDEBUG
  auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
  auto path = logger::log_directory();
  if (!path) {
    util::report_and_fail("Failed to find standard logging directory"sv);
  }

  *path /= std::format("{}.log"sv, Plugin::NAME);
  auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

  const auto level = a_level;

  auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
  log->set_level(level);
  log->flush_on(spdlog::level::info);

  spdlog::set_default_logger(std::move(log));
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");
}


void Listener(SKSE::MessagingInterface::Message* message) noexcept
{
  if (message->type == SKSE::MessagingInterface::kDataLoaded) 
  {
    BFEC::EquipManager::GetSingleton()->Init();
    BFEC::MessageboxManager::GetSingleton()->Init();
  }
  else if (message->type == SKSE::MessagingInterface::kInputLoaded)
  {
    BFEC::EquipManager::GetSingleton()->AddKeySink();
  }
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
  InitializeLog();
  logger::info("Loaded plugin {} {}", Plugin::NAME, Plugin::VERSION.string());
  SKSE::Init(a_skse);

  BFEC::HookManager::GetSingleton()->Init();

  if (const auto messaging{ SKSE::GetMessagingInterface() }; !messaging->RegisterListener(Listener))
    return false;

  const auto papyrus = SKSE::GetPapyrusInterface();
  papyrus->Register(Papyrus::RegisterFunctions);

  return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
  SKSE::PluginVersionData v;
  v.PluginName(Plugin::NAME.data());
  v.PluginVersion(Plugin::VERSION);
  v.UsesAddressLibrary(true);
  v.HasNoStructUse();
  return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
  pluginInfo->name = SKSEPlugin_Version.pluginName;
  pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
  pluginInfo->version = SKSEPlugin_Version.pluginVersion;
  return true;
}