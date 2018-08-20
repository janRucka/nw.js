//tag browser-v0.30.4+szn.2
//Revision: ed05e89a0f96935e152b21a1833a15bc8a5cd06b
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 23.4.2018 18:26:53
//Message:
//chrome.history.addUrl - optional title attribute added

//----
//    Modified: chrome / browser / extensions / api / history / history_api.cc
//Modified: chrome / common / extensions / api / history.json

function historyTitleButtonFunction() {
    console.log("historyTitle");

    var page = "https://www.novinky.cz";
    var pageTitle = "someTitle";
    chrome.history.deleteAll(function () {
        try {
            chrome.history.addUrl({ url: page, title: pageTitle }, function () {
                chrome.history.search({ text: pageTitle }, function (items) {
                    try {
                        if (items[0].title === pageTitle) {
                            addFinalResult("'chrome.history.addUrl' has 'title' option", true, 'historyTitleResult');
                            return;
                        }
                    } catch (e) {
                        addFinalResult("historyTitle error: Cannot find title", false, 'historyTitleResult');
                    }
                });
            });
        } catch (e) {
            addFinalResult("historyTitle error: Cannot insert title in 'chrome.history.addUrl'", false, 'historyTitleResult');
        }
    });
}