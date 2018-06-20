#ifndef NW_API_APP_API_H_
#define NW_API_APP_API_H_

#include <vector>

#include "base/run_loop.h"
#include "content/public/browser/browsing_data_remover.h"
#include "chrome/browser/shell_integration.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

class NwAppQuitFunction : public AsyncExtensionFunction {
 public:
  NwAppQuitFunction();

 protected:
  ~NwAppQuitFunction() override;

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.quit", UNKNOWN)
 private:
  void Callback();
};

class NwAppCloseAllWindowsFunction : public AsyncExtensionFunction {
 public:
  NwAppCloseAllWindowsFunction() {}

 protected:
  ~NwAppCloseAllWindowsFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.closeAllWindows", UNKNOWN)
};

class NwAppGetArgvSyncFunction : public NWSyncExtensionFunction {
 public:
  NwAppGetArgvSyncFunction();
  bool RunNWSync(base::ListValue* response, std::string* error) override;

 protected:
  ~NwAppGetArgvSyncFunction() override;


  DECLARE_EXTENSION_FUNCTION("nw.App.getArgvSync", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwAppGetArgvSyncFunction);
};

class NwAppClearCacheFunction : public NWSyncExtensionFunction, public content::BrowsingDataRemover::Observer {
 public:
  NwAppClearCacheFunction();
  bool RunNWSync(base::ListValue* response, std::string* error) override;
  void OnBrowsingDataRemoverDone() override;

 protected:
  ~NwAppClearCacheFunction() override;

  base::RunLoop run_loop_;

  DECLARE_EXTENSION_FUNCTION("nw.App.clearCache", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwAppClearCacheFunction);
};

class NwAppClearAppCacheFunction : public NWSyncExtensionFunction {
 public:
  NwAppClearAppCacheFunction();
  bool RunNWSync(base::ListValue* response, std::string* error) override;

 protected:
  ~NwAppClearAppCacheFunction() override;

  DECLARE_EXTENSION_FUNCTION("nw.App.clearAppCache", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwAppClearAppCacheFunction);
};

class NwAppSetProxyConfigFunction : public NWSyncExtensionFunction {
 public:
  NwAppSetProxyConfigFunction();
  bool RunNWSync(base::ListValue* response, std::string* error) override;

 protected:
  ~NwAppSetProxyConfigFunction() override;


  DECLARE_EXTENSION_FUNCTION("nw.App.setProxyConfig", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwAppSetProxyConfigFunction);
};

class NwAppGetDataPathFunction : public NWSyncExtensionFunction {
 public:
  NwAppGetDataPathFunction(){}
  bool RunNWSync(base::ListValue* response, std::string* error) override;
    
 protected:
  ~NwAppGetDataPathFunction() override {}
    
  DECLARE_EXTENSION_FUNCTION("nw.App.getDataPath", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwAppGetDataPathFunction);
};

class NwAppCrashBrowserFunction : public AsyncExtensionFunction {
 public:
  NwAppCrashBrowserFunction() {}

 protected:
  ~NwAppCrashBrowserFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.crashBrowser", UNKNOWN)
};

class NwAppGetIEBookmarksFunction : public AsyncExtensionFunction {
public:
  NwAppGetIEBookmarksFunction() {}

protected:
  ~NwAppGetIEBookmarksFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.getIEBookmarks", UNKNOWN)
};

class NwAppGetIEHistoryFunction : public AsyncExtensionFunction {
public:
  NwAppGetIEHistoryFunction() {}

protected:
  ~NwAppGetIEHistoryFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.getIEHistory", UNKNOWN)
};

class NwAppIsDefaultBrowserFunction : public AsyncExtensionFunction {
public:
  NwAppIsDefaultBrowserFunction() {}
  void OnCallback(shell_integration::DefaultWebClientState state);

protected:
  ~NwAppIsDefaultBrowserFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.isDefaultBrowser", UNKNOWN)

private:
  bool IsDefaultBrowserInRegistry();
};

class NwAppSetDefaultBrowserFunction : public AsyncExtensionFunction {
public:
  NwAppSetDefaultBrowserFunction() {}
  void OnCallback(shell_integration::DefaultWebClientState state);

protected:
  ~NwAppSetDefaultBrowserFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.setDefaultBrowser", UNKNOWN)

private:
  bool SetDefaultBrowserViaRegistry();
};

class NwAppRegisterBrowserFunction : public AsyncExtensionFunction {
public:
  NwAppRegisterBrowserFunction() {}
  void OnCallback(shell_integration::DefaultWebClientState state);

protected:
  ~NwAppRegisterBrowserFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.registerBrowser", UNKNOWN)

private:
  bool SetRegistrationViaRegistry();
};

class NwAppGetFlagsSettingFunction : public AsyncExtensionFunction {
public:
  NwAppGetFlagsSettingFunction() {}

protected:
  ~NwAppGetFlagsSettingFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.getFlagsSetting", UNKNOWN)
};

class NwAppSetFlagsSettingFunction : public AsyncExtensionFunction {
public:
  NwAppSetFlagsSettingFunction() {}

protected:
  ~NwAppSetFlagsSettingFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.setFlagsSetting", UNKNOWN)
};

} // namespace extensions
#endif
