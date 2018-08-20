//tag browser-v0.30.4+szn.2
//Revision: 6cfd0b52359a4d83c7d18572d67b54325aa040c1
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 25.4.2018 16:47:00
//Message:
//Webview navigation history getter.

//Webview now has:
//-getPagesHistory function, returning array of URLs, titles and favicons of pages in history.Titles and favicons are not known for current page because array is created before page is fully loaded.
//-getCurrentHistoryIndex function, returning current history index
//----
//    Modified: extensions / browser / guest_view / web_view / web_view_guest.cc
//Modified: extensions / renderer / resources / guest_view / web_view / web_view.js
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_api_methods.js
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_events.js

//Revision: 25342b61ff1a2b7a1875839fa83c5908040b96d0
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 23.4.2018 19:04:06
//Message:
//Restore navigation history on webview.It is possible to insert history to newly created webview via 'restoreHistory' function.

//----
//    Modified: chrome / common / extensions / api / webview_tag.json
//Modified: content / browser / frame_host / navigation_controller_impl.cc
//Modified: content / browser / frame_host / navigation_controller_impl.h
//Modified: content / public / browser / navigation_controller.h
//Modified: extensions / browser / api / guest_view / web_view / web_view_internal_api.cc
//Modified: extensions / browser / api / guest_view / web_view / web_view_internal_api.h
//Modified: extensions / browser / extension_function_histogram_value.h
//Modified: extensions / common / api / web_view_internal.json
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_api_methods.js

// test navigation history getter and navigation history restoration
function historyButtonFunction() {
    console.log("historyRestoreButtonFunction");
    try {

        var webview = document.createElement("webview")
        webview.id = "historyRestore";

        document.body.appendChild(webview);

        var history = [{ url: "https://www.seznam.cz/", title: "Seznam - najdu tam, co neznam" }, { url: "https://www.novinky.cz/", title: "Novinky.cz   nejctenejsi zpravy na ceskem internetu" }, { url: "https://www.seznam.cz/", title: "Seznam - najdu tam, co neznam" }]

        var fLoadStop = function () {
            webview.removeEventListener('loadstop', fLoadStop);
            webview.restoreHistory(1, history)

            webview.addEventListener('loadstop', function () {
                if (webview.getCurrentHistoryIndex() === 1) {
                    var webviewHistory = webview.getPagesHistory();
                    console.log(history);
                    console.log(webviewHistory);
                    for (var i = 0; i < webviewHistory.length; i++) {
                        if (webviewHistory[i].url !== history[i].url) {
                            throw "History is not the same!";
                        }
                    }
                    addFinalResult("history correctly restored", true, 'historyResult');
                    document.body.removeChild(webview);
                    return;
                }
                throw "History index is wrong!";
            });
        }

        webview.addEventListener('loadstop', fLoadStop);
        webview.src = "about:blank";
    } catch (e)
    {
        addFinalResult("history error", false, 'historyResult');
        document.body.removeChild(webview);
    }
}