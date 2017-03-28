#include "chrome/common/importer/importer_bridge.h"
#include "chrome/common/importer/imported_bookmark_entry.h"

class NwImporterBridge : public ImporterBridge {
public:

  NwImporterBridge();
  ~NwImporterBridge() override;

  void AddBookmarks(const std::vector<ImportedBookmarkEntry>& bookmarks,
    const base::string16& first_folder_name) override;

  void AddHomePage(const GURL& home_page) override;

#if defined(OS_WIN)
  void AddIE7PasswordInfo(
    const importer::ImporterIE7PasswordInfo& password_info) override;
#endif

  void SetFavicons(const favicon_base::FaviconUsageDataList& favicons) override;

  void SetHistoryItems(const std::vector<ImporterURLRow>& rows,
    importer::VisitSource visit_source) override;

  void SetKeywords(
    const std::vector<importer::SearchEngineInfo>& search_engines,
    bool unique_on_host_and_path) override;

  void SetFirefoxSearchEnginesXMLData(
    const std::vector<std::string>& search_engine_data) override;

  void SetPasswordForm(const autofill::PasswordForm& form) override;

  void SetAutofillFormData(
    const std::vector<ImporterAutofillFormDataEntry>& entries) override;

  void NotifyStarted() override;
  void NotifyItemStarted(importer::ImportItem item) override;
  void NotifyItemEnded(importer::ImportItem item) override;
  void NotifyEnded() override;

  base::string16 GetLocalizedString(int message_id) override;

  const std::vector<ImportedBookmarkEntry>& GetBookmarks() const { return bookmarks_; } 

private:
  std::vector<ImportedBookmarkEntry> bookmarks_;
  //favicon_base::FaviconUsageDataList favicons_;
};