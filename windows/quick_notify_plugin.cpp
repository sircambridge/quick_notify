#include "include/quick_notify/quick_notify_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

#include <winrt/Windows.UI.Notifications.h>
#include <winrt/Windows.Data.Xml.Dom.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

using namespace winrt;
using namespace Windows::UI::Notifications;
using namespace Windows::Data::Xml::Dom;

namespace {

class QuickNotifyPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  QuickNotifyPlugin();

  virtual ~QuickNotifyPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  ToastNotifier toastNotifier_{ ToastNotificationManager::CreateToastNotifier(L"RecBot") };
};

// static
void QuickNotifyPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "quick_notify",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<QuickNotifyPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

QuickNotifyPlugin::QuickNotifyPlugin() {}

QuickNotifyPlugin::~QuickNotifyPlugin() {}

void QuickNotifyPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("getPlatformVersion") == 0) {
    std::ostringstream version_stream;
    version_stream << "Windows ";
    if (IsWindows10OrGreater()) {
      version_stream << "10+";
    } else if (IsWindows8OrGreater()) {
      version_stream << "8";
    } else if (IsWindows7OrGreater()) {
      version_stream << "7";
    }
    result->Success(flutter::EncodableValue(version_stream.str()));
  } else if (method_call.method_name().compare("notify") == 0) {
    auto args = std::get<flutter::EncodableMap>(*method_call.arguments());
    auto content = std::get<std::string>(args[flutter::EncodableValue("content")]);

    auto toastContent = ToastNotificationManager::GetTemplateContent(ToastTemplateType::ToastText01);
    XmlNodeList xmlNodeList = toastContent.GetElementsByTagName(L"text");
    xmlNodeList.Item(0).AppendChild(toastContent.CreateTextNode(winrt::to_hstring(content)));
    ToastNotification toastNotification{ toastContent };
    toastNotifier_.Show(toastNotification);
    result->Success(nullptr);
  } else {
    result->NotImplemented();
  }
}

}  // namespace

void QuickNotifyPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  QuickNotifyPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
