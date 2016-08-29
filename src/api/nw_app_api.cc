#include "content/nw/src/api/nw_app_api.h"

#include "base/command_line.h"
#include "chrome/browser/browsing_data/browsing_data_appcache_helper.h"
#include "chrome/browser/browsing_data/browsing_data_helper.h"
#include "chrome/browser/browsing_data/browsing_data_remover_factory.h"
#include "chrome/browser/devtools/devtools_window.h"
#include "chrome/browser/extensions/devtools_util.h"
#include "chrome/browser/extensions/extension_service.h"
#include "content/nw/src/nw_base.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/app_window/app_window.h"
#include "extensions/browser/app_window/app_window_registry.h"
#include "extensions/browser/app_window/native_app_window.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/error_utils.h"
#include "net/proxy/proxy_config.h"
#include "net/proxy/proxy_config_service_fixed.h"
#include "net/proxy/proxy_service.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"

#if defined(OS_WIN)
#include "base/path_service.h"
#include "base/win/windows_version.h"
#include "chrome/installer/util/registry_entry.h"
#include "chrome/installer/util/work_item.h"
#include "chrome/installer/util/work_item_list.h"
#endif

namespace {
void SetProxyConfigCallback(
    base::WaitableEvent* done,
    net::URLRequestContextGetter* url_request_context_getter,
    const net::ProxyConfig& proxy_config) {
  net::ProxyService* proxy_service =
      url_request_context_getter->GetURLRequestContext()->proxy_service();
  proxy_service->ResetConfigService(make_scoped_ptr(new net::ProxyConfigServiceFixed(proxy_config)));
  done->Signal();
}

class DefaultBrowserNwHandler : public shell_integration::DefaultWebClientObserver {
public:
  DefaultBrowserNwHandler(
    base::Callback<void(shell_integration::DefaultWebClientUIState)> is_default_browser
    = base::Callback<void(shell_integration::DefaultWebClientUIState)>()) :
    is_default_browser_(is_default_browser)
  {}

  ~DefaultBrowserNwHandler() override {}
  void SetDefaultWebClientUIState(shell_integration::DefaultWebClientUIState state) override {
    if (!is_default_browser_.is_null())
      is_default_browser_.Run(state);
  }

private:
  base::Callback<void(shell_integration::DefaultWebClientUIState)> is_default_browser_;
};
} // namespace

namespace extensions {
NwAppQuitFunction::NwAppQuitFunction() {

}

NwAppQuitFunction::~NwAppQuitFunction() {
}

bool NwAppQuitFunction::RunAsync() {
  ExtensionService* service =
    ExtensionSystem::Get(browser_context())->extension_service();
  base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&ExtensionService::TerminateExtension,
                   service->AsWeakPtr(),
                   extension_id()));
  return true;
}

bool NwAppCloseAllWindowsFunction::RunAsync() {
  AppWindowRegistry* registry = AppWindowRegistry::Get(browser_context());
  if (!registry)
    return false;

  AppWindowRegistry::AppWindowList windows =
    registry->GetAppWindowsForApp(extension()->id());

  for (AppWindow* window : windows) {
    if (window->NWCanClose())
      window->GetBaseWindow()->Close();
  }
  SendResponse(true);
  return true;
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
  BrowsingDataRemover* remover = BrowsingDataRemoverFactory::GetForBrowserContext(
      Profile::FromBrowserContext(context_));

  remover->AddObserver(this);
  remover->Remove(BrowsingDataRemover::Unbounded(),
                  BrowsingDataRemover::REMOVE_CACHE,
                  BrowsingDataHelper::ALL);
  // BrowsingDataRemover deletes itself.
  base::MessageLoop::ScopedNestableTaskAllower allow(
        base::MessageLoop::current());
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
  std::string proxy_config;
  EXTENSION_FUNCTION_VALIDATE(args_->GetString(0, &proxy_config));

  base::ThreadRestrictions::ScopedAllowWait allow_wait;

  net::ProxyConfig config;
  config.proxy_rules().ParseFromString(proxy_config);
  content::RenderProcessHost* render_process_host = GetSenderWebContents()->GetRenderProcessHost();
  net::URLRequestContextGetter* context_getter =
    render_process_host->GetBrowserContext()->
    GetRequestContextForRenderProcess(render_process_host->GetID());

  base::WaitableEvent done(false, false);
  content::BrowserThread::PostTask(
      content::BrowserThread::IO, FROM_HERE,
      base::Bind(&SetProxyConfigCallback, &done,
                 make_scoped_refptr(context_getter), config));
  done.Wait();
  return true;
}

bool NwAppGetDataPathFunction::RunNWSync(base::ListValue* response, std::string* error) {
  response->AppendString(browser_context()->GetPath().value());
  return true;
}

bool NwAppCrashBrowserFunction::RunAsync() {
  int* ptr = nullptr;
  *ptr = 1;
  return true;
}

bool NwAppIsDefaultBrowserFunction::RunAsync() {
  scoped_refptr<shell_integration::DefaultBrowserWorker> browserWorker(
    new shell_integration::DefaultBrowserWorker(
      new DefaultBrowserNwHandler(
        base::Bind(&NwAppIsDefaultBrowserFunction::OnCallback, this)), true));
  browserWorker->StartCheckIsDefault();
  return true;
}

bool NwAppIsDefaultBrowserFunction::IsDefaultBrowserInRegistry() {
#if defined(OS_WIN)
  if (base::win::GetVersion() > base::win::VERSION_WIN7)
    return false;

  base::FilePath chrome_exe;
  if (!PathService::Get(base::FILE_EXE, &chrome_exe))
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
  shell_integration::DefaultWebClientUIState state) {
  results_ = scoped_ptr<base::ListValue>(new base::ListValue());
  switch (state)
  {
  case shell_integration::STATE_PROCESSING:
    return;
  case shell_integration::STATE_NOT_DEFAULT:
    if (IsDefaultBrowserInRegistry())
      results_->AppendString("default");
    else
      results_->AppendString("not default");
    break;
  case shell_integration::STATE_IS_DEFAULT:
    results_->AppendString("default");
    break;
  case shell_integration::STATE_UNKNOWN:
  default:
    if (IsDefaultBrowserInRegistry())
      results_->AppendString("default");
    else
      results_->AppendString("unknown");
  }
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  SendResponse(true);
}

bool NwAppSetDefaultBrowserFunction::RunAsync() {
  scoped_refptr<shell_integration::DefaultBrowserWorker> browserWorker(
    new shell_integration::DefaultBrowserWorker(
      new DefaultBrowserNwHandler(
        base::Bind(&NwAppSetDefaultBrowserFunction::OnCallback, this)), true));
  browserWorker->StartSetAsDefault();
  return true;
}

#if defined(OS_WIN)
bool AddToHKCURegistry(const ScopedVector<RegistryEntry>& registryItems)
{
  HKEY key = HKEY_CURRENT_USER;
  scoped_ptr<WorkItemList> items(WorkItem::CreateWorkItemList());
  for (const RegistryEntry* const entry : registryItems)
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
  if (!PathService::Get(base::FILE_EXE, &chrome_exe))
    return false;

  ScopedVector<RegistryEntry> registryItems;
  registryItems.push_back(new RegistryEntry(L"Software\\Classes\\SeznamHTML", L"Seznam HTML Document"));
  registryItems.push_back(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\DefaultIcon", chrome_exe.value() + L",1"));
  registryItems.push_back(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\shell", L"open"));
  registryItems.push_back(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\shell\\open\\command", L"\"" + chrome_exe.value() + L"\"" + std::wstring(L"-surl=\"%1\"")));

  registryItems.push_back(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities", L"ApplicationDescription", L"Prohlížeè Seznam.cz"));
  registryItems.push_back(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\UrlAssociations", L"http", L"SeznamHTML"));
  registryItems.push_back(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\UrlAssociations", L"https", L"SeznamHTML"));
  registryItems.push_back(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\Startmenu", L"StartmenuInternet", chrome_exe.value()));

  registryItems.push_back(new RegistryEntry(L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice", L"Progid", L"SeznamHTML"));
  registryItems.push_back(new RegistryEntry(L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice", L"Progid", L"SeznamHTML"));
  registryItems.push_back(new RegistryEntry(L"Software\\RegisteredApplications", L"Seznam.cz", L"Software\\Seznam.cz\\WebBrowser\\Capabilities"));

  // url association
  if (!AddToHKCURegistry(registryItems))
    return false;

  registryItems.erase(registryItems.begin(), registryItems.end());
  registryItems.push_back(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"html", L"SeznamHTML"));
  registryItems.push_back(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"htm", L"SeznamHTML"));
  registryItems.push_back(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"xhtml", L"SeznamHTML"));
  registryItems.push_back(new RegistryEntry(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.html\\UserChoice", L"Progid", L"SeznamHTML"));
  registryItems.push_back(new RegistryEntry(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.htm\\UserChoice", L"Progid", L"SeznamHTML"));
  registryItems.push_back(new RegistryEntry(L"Software\\Classes\\.htm\\OpenWithProgids", L"SeznamHTML", L""));
  registryItems.push_back(new RegistryEntry(L"Software\\Classes\\.html\\OpenWithProgids", L"SeznamHTML", L""));
  registryItems.push_back(new RegistryEntry(L"Software\\Classes\\.shtml\\OpenWithProgids", L"SeznamHTML", L""));

  // file (*.htm, *.html, *.xhtml) association, it will probably fail, but we try anyway
  AddToHKCURegistry(registryItems);
  return true;
#endif
  return false;
}

void NwAppSetDefaultBrowserFunction::OnCallback(
  shell_integration::DefaultWebClientUIState state) {
  results_ = scoped_ptr<base::ListValue>(new base::ListValue());
  switch (state)
  {
  case shell_integration::STATE_PROCESSING:
    return;
  case shell_integration::STATE_NOT_DEFAULT:
    if (SetDefaultBrowserViaRegistry())
      results_->AppendString("default");
    else
      results_->AppendString("not default");
    break;
  case shell_integration::STATE_IS_DEFAULT:
    results_->AppendString("default");
    break;
  case shell_integration::STATE_UNKNOWN:
  default:
    if (SetDefaultBrowserViaRegistry())
      results_->AppendString("default");
    else
      results_->AppendString("unknown");
  }
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  SendResponse(true);
}

} // namespace extensions
