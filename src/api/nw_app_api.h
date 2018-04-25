#ifndef NW_API_APP_API_H_
#define NW_API_APP_API_H_

#include <vector>

#include "base/run_loop.h"
#include "content/public/browser/browsing_data_remover.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

class NwAppQuitFunction : public UIThreadExtensionFunction {
 public:
  NwAppQuitFunction();

 protected:
  ~NwAppQuitFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.quit", UNKNOWN)
 private:
  void Callback();
};

class NwAppCloseAllWindowsFunction : public UIThreadExtensionFunction {
 public:
  NwAppCloseAllWindowsFunction() {}

 protected:
  ~NwAppCloseAllWindowsFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
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

class NwAppCrashBrowserFunction : public UIThreadExtensionFunction {
 public:
  NwAppCrashBrowserFunction() {}

 protected:
  ~NwAppCrashBrowserFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.crashBrowser", UNKNOWN)
};

class NwAppGetIEBookmarksFunction : public UIThreadExtensionFunction {
public:
  NwAppGetIEBookmarksFunction() {}

protected:
  ~NwAppGetIEBookmarksFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.getIEBookmarks", UNKNOWN)
};

} // namespace extensions
#endif
