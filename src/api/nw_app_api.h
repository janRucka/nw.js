#ifndef NW_API_APP_API_H_
#define NW_API_APP_API_H_

#include <vector>

#include "base/run_loop.h"
#include "chrome/browser/browsing_data/browsing_data_remover.h"
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

class NwAppClearCacheFunction : public NWSyncExtensionFunction, public BrowsingDataRemover::Observer {
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

class NwAppIsDefaultBrowserFunction : public AsyncExtensionFunction {
public:
  NwAppIsDefaultBrowserFunction() {}
  void OnCallback(ShellIntegration::DefaultWebClientUIState state);

protected:
  ~NwAppIsDefaultBrowserFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.isDefaultBrowser", UNKNOWN)
};

class NwAppSetDefaultBrowserFunction : public AsyncExtensionFunction {
public:
  NwAppSetDefaultBrowserFunction() {}
  void OnCallback(bool success);

protected:
  ~NwAppSetDefaultBrowserFunction() override {}

  // ExtensionFunction:
  bool RunAsync() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.setDefaultBrowser", UNKNOWN)
};

} // namespace extensions
#endif
