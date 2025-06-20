#include "MessageBox.h"

SINGLETONBODY(BFEC::MessageboxManager)

void BFEC::MessageboxManager::ShowMessageBox(const std::string& bodyText, const std::vector<std::string>& buttonTextValues, std::function<void(uint32_t)> callback, bool useHtml)
{
  auto* factoryManager = RE::MessageDataFactoryManager::GetSingleton();
  auto* uiStringHolder = RE::InterfaceStrings::GetSingleton();
  auto* factory = factoryManager->GetCreator<RE::MessageBoxData>(uiStringHolder->messageBoxData);  // "MessageBoxData" <--- can we just use this string?
  auto* messagebox = factory->Create();
  RE::BSTSmartPointer<RE::IMessageBoxCallback> messageCallback = RE::make_smart<MessageBoxResultCallback>(callback);
  messagebox->callback = messageCallback;
  messagebox->bodyText = bodyText;
  for (auto text : buttonTextValues) messagebox->buttonText.push_back(text.c_str());

  messagebox->unk4D = useHtml; // yes, this actually enables html

  messagebox->QueueMessage();
}

void BFEC::MessageboxManager::Init()
{
  if (!_Ready)
  {
    _Ready = true;
  }
}
