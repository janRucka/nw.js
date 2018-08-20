//tag browser-v0.30.4+szn.2
//Revision: e28e03efd926cd6942492a1411337fec6b7cdd31
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 25.4.2018 17:52:41
//Message:
//Default browser.It is possible to check whether nwjs is default browser or set as default browser via these function (changes in 'content\nw' as well):
//- nw.App.isDefaultBrowser
//    - nw.App.setDefaultBrowser
//----
//    Modified: chrome / installer / util / shell_util.cc

//Revision: 7b691f37c8c0eb52df382eafb89b3a4067e036c2
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 25.4.2018 17:53:09
//Message:
//Default browser.It is possible to check whether nwjs is default browser or set as default browser via these function (changes in chromium project 'src' as well):
//- nw.App.isDefaultBrowser
//    - nw.App.setDefaultBrowser
//----
//    Modified: src / api / nw_app.idl
//Modified: src / api / nw_app_api.cc
//Modified: src / api / nw_app_api.h

// just parcial test of 'isDefaultBrowser' function
function defaultBrowserButtonFunction() {
    console.log("defaultBrowserButtonFunction");
    try {
        var timeout = setTimeout(function () {
            addFinalResult("defaultBrowser callback not received within 8 seconds", false, 'defaultBrowserResult');
        }, 8000);

        nw.App.isDefaultBrowser(function (arg) {
            clearTimeout(timeout);
            addFinalResult("Right callback 'isDefaultBrowser'", true, 'defaultBrowserResult');
        });
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'defaultBrowserResult');
    }
}