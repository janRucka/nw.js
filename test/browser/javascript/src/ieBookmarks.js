//tag browser-v0.30.4+szn.2
//Revision: bc8ff7092a2a8cdab7590343942e16dfa5c1e6dc
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 25.4.2018 17:43:59
//Message:
//It is possible to get IE bookmarks (changes in 'content\nw' as well):
//- nw.App.getIEBookmarks
//----
//    Modified: base / files / file_enumerator_win.cc
//Modified: chrome / utility / importer / ie_importer_win.cc

//Revision: 2c367c44680a28662058de53c4f80a243c3184e2
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 25.4.2018 17:45:07
//Message:
//It is possible to get IE bookmarks (changes in chromium project 'src' as well):
//- nw.App.getIEBookmarks
//----
//    Modified: BUILD.gn
//Modified: nw.gypi
//Modified: src / api / nw_app.idl
//Modified: src / api / nw_app_api.cc
//Modified: src / api / nw_app_api.h
//Added: src / nw_importer_bridge.cc
//Added: src / nw_importer_bridge.h

//Revision: 9c548f552e6c5d603573c76b903d3a7c48f0077e
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 26.4.2018 14:11:33
//Message:
//Getting IE history via function: nw.App.getIEHistory(callback)

//----
//    Modified: src / api / nw_app.idl
//Modified: src / api / nw_app_api.cc
//Modified: src / api / nw_app_api.h
//Modified: src / nw_importer_bridge.cc
//Modified: src / nw_importer_bridge.h

// IE needs to have some bookmarks. Default bookmarks are ignored.
// Tests just installed version of IE
function ieBookmarksButtonFunction() {
    console.log("ieBookmarksButtonFunction");
    try {
        var bookmarksTimeout = setTimeout(function () {
            addFinalResult("getIEBookmarks callback not received within 8 seconds", false, 'ieBookmarksResult');
        }, 8000);

        var historyTimeout = setTimeout(function () {
            addFinalResult("getIEHistory callback not received within 8 seconds", false, 'ieHistoryResult');
        }, 8000);

        nw.App.getIEBookmarks(function (arg) {
            clearTimeout(bookmarksTimeout);
            if (arg[0].hasOwnProperty('title') && arg[0].hasOwnProperty('url')) {
                addFinalResult("Right callback 'getIEBookmarks'", true, 'ieBookmarksResult');
            } else {
                addFinalResult("Wrong type in callback", false, 'ieBookmarksResult');
            }
        });
        nw.App.getIEHistory(function (arg) {
            clearTimeout(historyTimeout);
            if (arg[0].hasOwnProperty('last_visit') && arg[0].hasOwnProperty('url')) {
                addFinalResult("Right callback 'getIEHistory'", true, 'ieHistoryResult');
            } else {
                addFinalResult("Wrong type in callback", false, 'ieHistoryResult');
            }
        });
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'ieBookmarksResult');
        addFinalResult("Unexpected exception", false, 'ieHistoryResult');
    }
}