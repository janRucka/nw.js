#include "content/nw/src/api/nw_app_api.h"

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/memory/ptr_util.h"
#include "chrome/browser/about_flags.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/browsing_data/browsing_data_appcache_helper.h"
#include "chrome/browser/browsing_data/browsing_data_helper.h"
#include "chrome/browser/devtools/devtools_window.h"
#include "chrome/browser/extensions/devtools_util.h"
#include "chrome/browser/extensions/extension_service.h"
#include "components/flags_ui/flags_state.h"
#include "components/flags_ui/pref_service_flags_storage.h"
#include "content/nw/src/api/nw_app.h"
#include "content/nw/src/nw_base.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/app_window/app_window.h"
#include "extensions/browser/app_window/app_window_registry.h"
#include "extensions/browser/app_window/native_app_window.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/error_utils.h"
#include "net/proxy_resolution/proxy_config.h"
#include "net/proxy_resolution/proxy_config_service_fixed.h"
#include "net/proxy_resolution/proxy_resolution_service.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"

#if defined(OS_WIN)
#include "../nw_importer_bridge.h"
#include "base/win/windows_version.h"
#include "chrome/common/importer/imported_bookmark_entry.h"
#include "chrome/installer/util/registry_entry.h"
#include "chrome/installer/util/work_item.h"
#include "chrome/installer/util/work_item_list.h"
#include "chrome/utility/importer/ie_importer_win.h"
#endif

using namespace extensions::nwapi::nw__app;

namespace {
void SetProxyConfigCallback(
    base::WaitableEvent* done,
    const scoped_refptr<net::URLRequestContextGetter>& url_request_context_getter,
    const net::ProxyConfigWithAnnotation& proxy_config) {
  net::ProxyResolutionService* proxy_service =
      url_request_context_getter->GetURLRequestContext()->proxy_resolution_service();
  proxy_service->ResetConfigService(base::WrapUnique(new net::ProxyConfigServiceFixed(proxy_config)));
  done->Signal();
}

#if defined(OS_WIN)
base::ListValue *ListValue_FromStringArray(const std::vector<std::wstring> &arr) {
  base::ListValue *v = new base::ListValue();
  for (std::vector<std::wstring>::const_iterator iter = arr.begin(); iter != arr.end(); ++iter) {
    v->AppendString(*iter);
  }
  return v;
}

enum class IEImportType {
  Bookmarks,
  History
  };

base::ListValue* GetImportListFromIE(IEImportType importType) {
  importer::SourceProfile profile;
  profile.importer_name = L"Microsoft Internet Explorer";
  profile.importer_type = importer::TYPE_IE;
  profile.services_supported = 31;

  NwImporterBridge* bridge = new NwImporterBridge;
  IEImporter* importer = new IEImporter();
  importer->StartImport(profile,
    importType == IEImportType::Bookmarks ? importer::FAVORITES : importer::HISTORY,
    bridge);
  base::ListValue* values = new base::ListValue();

  if (importType == IEImportType::Bookmarks) {
    for (const ImportedBookmarkEntry& bookmark : bridge->GetBookmarks()) {
      base::DictionaryValue* dict = new base::DictionaryValue;
      dict->SetBoolean("in_toolbar", bookmark.in_toolbar);
      dict->SetBoolean("in_folder", bookmark.is_folder);
      dict->SetList("path", std::unique_ptr<base::ListValue>(ListValue_FromStringArray(bookmark.path)));
      dict->SetBoolean("in_toolbar", bookmark.in_toolbar);
      dict->SetString("title", bookmark.title);
      dict->SetString("url", bookmark.url.spec());
      values->Append(std::unique_ptr<base::Value>(static_cast<base::Value*>(dict)));
    }
  } else if (importType == IEImportType::History) {
    for (const ImporterURLRow& historyItem : bridge->GetHistory()) {
      base::DictionaryValue* dict = new base::DictionaryValue;
      dict->SetString("url", historyItem.url.spec());
      dict->SetString("title", historyItem.title);
      dict->SetInteger("visit_count", historyItem.visit_count);
      dict->SetInteger("typed_count", historyItem.typed_count);
      dict->SetDouble("last_visit", historyItem.last_visit.ToJsTime());
      dict->SetBoolean("hidden", historyItem.hidden);
      values->Append(std::unique_ptr<base::Value>(static_cast<base::Value*>(dict)));
    }
  }

  importer->Release();
  bridge->Release();
  return values;
}
#endif
} // namespace

namespace extensions {
NwAppQuitFunction::NwAppQuitFunction() {

}

NwAppQuitFunction::~NwAppQuitFunction() {
}

ExtensionFunction::ResponseAction
NwAppQuitFunction::Run() {
  ExtensionService* service =
    ExtensionSystem::Get(browser_context())->extension_service();
  base::MessageLoop::current()->task_runner()->PostTask(
        FROM_HERE,
        base::Bind(&ExtensionService::TerminateExtension,
                   service->AsWeakPtr(),
                   extension_id()));
  return RespondNow(NoArguments());
}

void NwAppCloseAllWindowsFunction::DoJob(AppWindowRegistry* registry, std::string id) {
  AppWindowRegistry::AppWindowList windows =
    registry->GetAppWindowsForApp(id);

  for (AppWindow* window : windows) {
    if (window->NWCanClose())
      window->GetBaseWindow()->Close();
  }
}

ExtensionFunction::ResponseAction
NwAppCloseAllWindowsFunction::Run() {
  AppWindowRegistry* registry = AppWindowRegistry::Get(browser_context());
  if (!registry)
    return RespondNow(Error(""));
  base::MessageLoop::current()->task_runner()->PostTask(
        FROM_HERE,
        base::Bind(&NwAppCloseAllWindowsFunction::DoJob, registry, extension()->id()));

  return RespondNow(NoArguments());
}

NwAppGetArgvSyncFunction::NwAppGetArgvSyncFunction() {
}

NwAppGetArgvSyncFunction::~NwAppGetArgvSyncFunction() {
}

bool NwAppGetArgvSyncFunction::RunNWSync(base::ListValue* response, std::string* error) {

  nw::Package* package = nw::package();
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  base::CommandLine::StringVector args = command_line->GetArgs();
  base::CommandLine::StringVector argv = command_line->original_argv();

  // Ignore first non-switch arg if it's not a standalone package.
  bool ignore_arg = !package->self_extract();
  for (unsigned i = 1; i < argv.size(); ++i) {
    if (ignore_arg && args.size() && argv[i] == args[0]) {
      ignore_arg = false;
      continue;
    }

    response->AppendString(argv[i]);
  }
  return true;
}

NwAppClearAppCacheFunction::NwAppClearAppCacheFunction() {
}

NwAppClearAppCacheFunction::~NwAppClearAppCacheFunction() {
}

bool NwAppClearAppCacheFunction::RunNWSync(base::ListValue* response, std::string* error) {
  std::string manifest;
  EXTENSION_FUNCTION_VALIDATE(args_->GetString(0, &manifest));

  GURL manifest_url(manifest);
  scoped_refptr<CannedBrowsingDataAppCacheHelper> helper(
        new CannedBrowsingDataAppCacheHelper(Profile::FromBrowserContext(context_)));

  helper->DeleteAppCacheGroup(manifest_url);
  return true;
}

NwAppClearCacheFunction::NwAppClearCacheFunction() {
}

NwAppClearCacheFunction::~NwAppClearCacheFunction() {
}

bool NwAppClearCacheFunction::RunNWSync(base::ListValue* response, std::string* error) {
  content::BrowsingDataRemover* remover = content::BrowserContext::GetBrowsingDataRemover(
      Profile::FromBrowserContext(context_));

  remover->AddObserver(this);
  remover->RemoveAndReply(base::Time(), base::Time::Max(),
                          content::BrowsingDataRemover::DATA_TYPE_CACHE,
                          content::BrowsingDataRemover::ORIGIN_TYPE_UNPROTECTED_WEB,
                          this);
  // BrowsingDataRemover deletes itself.
  base::MessageLoop::ScopedNestableTaskAllower allow;

  run_loop_.Run();
  remover->RemoveObserver(this);
  return true;
}

void NwAppClearCacheFunction::OnBrowsingDataRemoverDone() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  run_loop_.Quit();
}

NwAppSetProxyConfigFunction::NwAppSetProxyConfigFunction() {
}

NwAppSetProxyConfigFunction::~NwAppSetProxyConfigFunction() {
}

bool NwAppSetProxyConfigFunction::RunNWSync(base::ListValue* response, std::string* error) {
  net::ProxyConfigWithAnnotation config;
  std::unique_ptr<nwapi::nw__app::SetProxyConfig::Params> params(
      nwapi::nw__app::SetProxyConfig::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  std::string pac_url = params->pac_url.get() ? *params->pac_url : "";
  if (!pac_url.empty()) {
    if (pac_url == "<direct>")
      config = net::ProxyConfigWithAnnotation::CreateDirect();
    else if (pac_url == "<auto>")
      config = net::ProxyConfigWithAnnotation(net::ProxyConfig::CreateAutoDetect(), TRAFFIC_ANNOTATION_FOR_TESTS);
    else
      config = net::ProxyConfigWithAnnotation(net::ProxyConfig::CreateFromCustomPacURL(GURL(pac_url)), TRAFFIC_ANNOTATION_FOR_TESTS);
  } else {
    std::string proxy_config;
    net::ProxyConfig pc;
    EXTENSION_FUNCTION_VALIDATE(args_->GetString(0, &proxy_config));
    pc.proxy_rules().ParseFromString(proxy_config);
    config = net::ProxyConfigWithAnnotation(pc, TRAFFIC_ANNOTATION_FOR_TESTS);
  }

  base::ThreadRestrictions::ScopedAllowWait allow_wait;

  content::RenderProcessHost* render_process_host = GetSenderWebContents()->GetMainFrame()->GetProcess();
  net::URLRequestContextGetter* context_getter =
    render_process_host->GetStoragePartition()->GetURLRequestContext();

  base::WaitableEvent done(base::WaitableEvent::ResetPolicy::AUTOMATIC,
                           base::WaitableEvent::InitialState::NOT_SIGNALED);
  content::BrowserThread::PostTask(
      content::BrowserThread::IO, FROM_HERE,
      base::Bind(&SetProxyConfigCallback, &done,
                 base::WrapRefCounted(context_getter), config));
  done.Wait();
  return true;
}

bool NwAppGetDataPathFunction::RunNWSync(base::ListValue* response, std::string* error) {
  response->AppendString(browser_context()->GetPath().value());
  return true;
}

ExtensionFunction::ResponseAction
NwAppCrashBrowserFunction::Run() {
  int* ptr = nullptr;
  *ptr = 1;
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
NwAppGetIEBookmarksFunction::Run() {
#if defined(OS_WIN)
  importer::SourceProfile profile;
  profile.importer_name = L"Microsoft Internet Explorer";
  profile.importer_type = importer::TYPE_IE;
  profile.services_supported = 31;

  NwImporterBridge* bridge = new NwImporterBridge;
  IEImporter* importer = new IEImporter();
  importer->StartImport(profile, importer::FAVORITES, bridge);
  base::ListValue* values = new base::ListValue();

  for (const ImportedBookmarkEntry& bookmark : bridge->GetBookmarks()) {
    base::DictionaryValue* dict = new base::DictionaryValue;
    dict->SetBoolean("in_toolbar", bookmark.in_toolbar);
    dict->SetBoolean("in_folder", bookmark.is_folder);
    dict->SetList("path", std::unique_ptr<base::ListValue>(ListValue_FromStringArray(bookmark.path)));
    dict->SetBoolean("in_toolbar", bookmark.in_toolbar);
    dict->SetString("title", bookmark.title);
    dict->SetString("url", bookmark.url.spec());
    values->Append(std::unique_ptr<base::DictionaryValue>(dict));
  }

  importer->Release();
  bridge->Release();
  return RespondNow(OneArgument(std::unique_ptr<base::ListValue>(values)));
#else
  return RespondNow(Error(""));
#endif
}

ExtensionFunction::ResponseAction
NwAppGetIEHistoryFunction::Run() {
#if defined(OS_WIN)
  base::ListValue* values = GetImportListFromIE(IEImportType::History);
  return RespondNow(OneArgument(std::unique_ptr<base::ListValue>(values)));
#else
  return RespondNow(Error(""));
#endif
}

ExtensionFunction::ResponseAction
NwAppIsDefaultBrowserFunction::Run() {
  scoped_refptr<shell_integration::DefaultBrowserWorker> browserWorker(
    new shell_integration::DefaultBrowserWorker(
      base::Bind(static_cast<void (NwAppIsDefaultBrowserFunction::*)
      (shell_integration::DefaultWebClientState)>(&NwAppIsDefaultBrowserFunction::OnCallback),
        base::RetainedRef(this))));

  browserWorker->set_interactive_permitted(true);
  browserWorker->StartCheckIsDefault();

  return RespondLater();
}

bool NwAppIsDefaultBrowserFunction::IsDefaultBrowserInRegistry() {
#if defined(OS_WIN)
  if (base::win::GetVersion() > base::win::VERSION_WIN7)
    return false;

  base::FilePath chrome_exe;
  if (!base::PathService::Get(base::FILE_EXE, &chrome_exe))
    return false;

  if (RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\UrlAssociations", L"http", L"SeznamHTML").ExistsInRegistry(RegistryEntry::LOOK_IN_HKCU)
    && RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\UrlAssociations", L"https", L"SeznamHTML").ExistsInRegistry(RegistryEntry::LOOK_IN_HKCU)
    && RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\Startmenu", L"StartmenuInternet", chrome_exe.value()).ExistsInRegistry(RegistryEntry::LOOK_IN_HKCU)
    && RegistryEntry(L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice", L"Progid", L"SeznamHTML").ExistsInRegistry(RegistryEntry::LOOK_IN_HKCU)
    && RegistryEntry(L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice", L"Progid", L"SeznamHTML").ExistsInRegistry(RegistryEntry::LOOK_IN_HKCU))
    return true;
  else
    return false;
#endif
  return false;
}

void NwAppIsDefaultBrowserFunction::OnCallback(
  shell_integration::DefaultWebClientState state) {
  switch (state)
  {
  case shell_integration::DefaultWebClientState::NOT_DEFAULT:
    if (IsDefaultBrowserInRegistry())
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    else
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("not default"))));
    break;
  case shell_integration::DefaultWebClientState::IS_DEFAULT:
    Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    break;
  case shell_integration::DefaultWebClientState::NUM_DEFAULT_STATES:
  case shell_integration::DefaultWebClientState::UNKNOWN_DEFAULT:
  default:
    if (IsDefaultBrowserInRegistry())
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    else
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("unknown"))));
  }
}

ExtensionFunction::ResponseAction
NwAppSetDefaultBrowserFunction::Run() {
  scoped_refptr<shell_integration::DefaultBrowserWorker> browserWorker(
    new shell_integration::DefaultBrowserWorker(
      base::Bind(static_cast<void (NwAppSetDefaultBrowserFunction::*)
      (shell_integration::DefaultWebClientState)>(&NwAppSetDefaultBrowserFunction::OnCallback),
        base::RetainedRef(this))));

  browserWorker->set_interactive_permitted(true);
  browserWorker->StartSetAsDefault();

  return RespondLater();
}

#if defined(OS_WIN)
static bool AddToHKCURegistry(const std::vector<std::unique_ptr<RegistryEntry>>& registryItems)
{
  HKEY key = HKEY_CURRENT_USER;
  std::unique_ptr<WorkItemList> items(WorkItem::CreateWorkItemList());
  for (const std::unique_ptr<RegistryEntry>& entry : registryItems)
    entry->AddToWorkItemList(key, items.get());

  if (!items->Do()) {
    items->Rollback();
    return false;
  }
  return true;
}
#endif

bool NwAppSetDefaultBrowserFunction::SetDefaultBrowserViaRegistry() {
#if defined(OS_WIN)
  if (base::win::GetVersion() > base::win::VERSION_WIN7)
    return false;

  base::FilePath chrome_exe;
  if (!base::PathService::Get(base::FILE_EXE, &chrome_exe))
    return false;

  std::vector<std::unique_ptr<RegistryEntry>> registryItems;
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML", L"Seznam HTML Document")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\DefaultIcon", chrome_exe.value() + L",1")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\shell", L"open")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\shell\\open\\command", L"\"" + chrome_exe.value() + L"\"" + std::wstring(L"-surl=\"%1\""))));

  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities", L"ApplicationDescription", L"Prohlizec Seznam.cz")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\UrlAssociations", L"http", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\UrlAssociations", L"https", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\Startmenu", L"StartmenuInternet", chrome_exe.value())));

  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice", L"Progid", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice", L"Progid", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\RegisteredApplications", L"Seznam.cz", L"Software\\Seznam.cz\\WebBrowser\\Capabilities")));

  // url association
  if (!AddToHKCURegistry(registryItems))
    return false;

  registryItems.erase(registryItems.begin(), registryItems.end());
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"html", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"htm", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"xhtml", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.html\\UserChoice", L"Progid", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.htm\\UserChoice", L"Progid", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\.htm\\OpenWithProgids", L"SeznamHTML", L"")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\.html\\OpenWithProgids", L"SeznamHTML", L"")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\.shtml\\OpenWithProgids", L"SeznamHTML", L"")));

  // file (*.htm, *.html, *.xhtml) association, it will probably fail, but we try anyway
  AddToHKCURegistry(registryItems);
  return true;
#endif
  return false;
}

void NwAppSetDefaultBrowserFunction::OnCallback(
  shell_integration::DefaultWebClientState state) {
  switch (state)
  {
  case shell_integration::DefaultWebClientState::NOT_DEFAULT:
    if (SetDefaultBrowserViaRegistry())
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    else
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("not default"))));
    break;
  case shell_integration::DefaultWebClientState::IS_DEFAULT:
    Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    break;
  case shell_integration::DefaultWebClientState::UNKNOWN_DEFAULT:
  case shell_integration::DefaultWebClientState::NUM_DEFAULT_STATES:
  default:
    if (SetDefaultBrowserViaRegistry())
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    else
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("unknown"))));
  }
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}


ExtensionFunction::ResponseAction
NwAppRegisterBrowserFunction::Run() {
  scoped_refptr<shell_integration::DefaultBrowserWorker> browserWorker(
    new shell_integration::DefaultBrowserWorker(
      base::Bind(static_cast<void (NwAppRegisterBrowserFunction::*)
      (shell_integration::DefaultWebClientState)>(&NwAppRegisterBrowserFunction::OnCallback),
        base::RetainedRef(this))));

  browserWorker->StartRegistration();

  return RespondLater();
}

void NwAppRegisterBrowserFunction::OnCallback(
  shell_integration::DefaultWebClientState state) {
  switch (state)
  {
  case shell_integration::DefaultWebClientState::NOT_DEFAULT:
    if (SetRegistrationViaRegistry())
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value(true))));
    else
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value(false))));
    break;
  case shell_integration::DefaultWebClientState::IS_DEFAULT:
    Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value(true))));
    break;
  case shell_integration::DefaultWebClientState::UNKNOWN_DEFAULT:
  case shell_integration::DefaultWebClientState::NUM_DEFAULT_STATES:
  default:
    Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value(false))));
  }
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

bool NwAppRegisterBrowserFunction::SetRegistrationViaRegistry() {
#if defined(OS_WIN)
  if (base::win::GetVersion() > base::win::VERSION_WIN7)
    return false;

  base::FilePath chrome_exe;
  if (!base::PathService::Get(base::FILE_EXE, &chrome_exe))
    return false;

  std::vector<std::unique_ptr<RegistryEntry>> registryItems;
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML", L"Seznam HTML Document")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\DefaultIcon", chrome_exe.value() + L",1")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\shell", L"open")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\shell\\open\\command", L"\"" + chrome_exe.value() + L"\"" + std::wstring(L"-surl=\"%1\""))));

  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"html", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"htm", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"pdf", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\.htm\\OpenWithProgids", L"SeznamHTML", L"")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\.html\\OpenWithProgids", L"SeznamHTML", L"")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\.pdf\\OpenWithProgids", L"SeznamHTML", L"")));

  return AddToHKCURegistry(registryItems);

#endif
  return false;
}

base::ListValue* GetFlagsSettings() {
  std::unique_ptr<flags_ui::FlagsStorage> flags_storage(new flags_ui::PrefServiceFlagsStorage(g_browser_process->local_state()));
  std::set<std::string> flags = flags_storage->GetFlags();

  base::ListValue* values = new base::ListValue();
  for (const std::string& flag : flags)
    values->GetList().emplace_back(flag);

  return values;
}

ExtensionFunction::ResponseAction
NwAppGetFlagsSettingFunction::Run() {

  base::ListValue* values = GetFlagsSettings();
  return RespondNow(OneArgument(std::unique_ptr<base::Value>(static_cast<base::Value*>(values))));
}

void SanitizeList(flags_ui::FlagsStorage* flags_storage) {
  std::unique_ptr<base::ListValue> supported_features(new base::ListValue);
  std::unique_ptr<base::ListValue> unsupported_features(new base::ListValue);
  about_flags::GetFlagFeatureEntries(flags_storage,
    flags_ui::kOwnerAccessToFlags,
    supported_features.get(),
    unsupported_features.get());
}

ExtensionFunction::ResponseAction
NwAppSetFlagsSettingFunction::Run() {

  net::ProxyConfig config;
  std::unique_ptr<nwapi::nw__app::SetFlagsSetting::Params> params(
    nwapi::nw__app::SetFlagsSetting::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  std::set<std::string> flags;
  for (const std::string& param : params->data)
    flags.insert(param);

  std::unique_ptr<flags_ui::FlagsStorage> flags_storage(new flags_ui::PrefServiceFlagsStorage(g_browser_process->local_state()));
  flags_storage->SetFlags(flags);
  SanitizeList(flags_storage.get());

  base::ListValue* values = GetFlagsSettings();
  return RespondNow(OneArgument(std::unique_ptr<base::Value>(static_cast<base::Value*>(values))));
}

} // namespace extensions
