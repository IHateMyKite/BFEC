#pragma once

// Copied from https://github.com/SkyrimScripting/MessageBox

namespace BFEC
{
  class MessageboxManager
  {
  SINGLETONHEADER(MessageboxManager)
  public:

    class MessageBoxResultCallback : public RE::IMessageBoxCallback 
    {
    public:
      ~MessageBoxResultCallback() override {}
      MessageBoxResultCallback(std::function<void(uint32_t)> callback) : _callback(callback) {}
      void Run(RE::IMessageBoxCallback::Message message) override 
      {
        _callback(static_cast<uint32_t>(message));
      }
    private:
      std::function<void(uint32_t)> _callback;
    };

    void Init();
    std::string GetResultText(int32_t messageBoxId, bool deleteResultOnAccess = true);
    void ShowMessageBox(const std::string& bodyText,const std::vector<std::string>& buttonTextValues, std::function<void(uint32_t)> callback, bool useHtml);
  private:
    bool _Ready = false;
  };
}
