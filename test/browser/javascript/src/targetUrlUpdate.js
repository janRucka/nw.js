//tag browser-v0.30.4+szn.2
//Revision: 57a3d554a9e9647af8d455aef965c1c9203718b4
//Author: Michal Va�ek < michal.vasek@firma.seznam.cz>
//    Date: 25.4.2018 16:41:42
//Message:
//Event`targeturlupdate` with `targetUrl` parameter on hover url change.

//----
//    Modified: components / guest_view / browser / guest_view_base.cc
//Modified: components / guest_view / browser / guest_view_base.h
//Modified: extensions / browser / extension_event_histogram_value.h
//Modified: extensions / browser / guest_view / guest_view_events.cc
//Modified: extensions / browser / guest_view / web_view / web_view_constants.cc
//Modified: extensions / browser / guest_view / web_view / web_view_constants.h
//Modified: extensions / browser / guest_view / web_view / web_view_guest.cc
//Modified: extensions / browser / guest_view / web_view / web_view_guest.h
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_events.js

// test `targeturlupdate` event on hover url change.
function targetUrlUpdateButtonFunction() {
    console.log("targetUrlUpdateButtonFunction");
    try {
        var webview = document.getElementById("webviewId");

        var timeout = setTimeout(function () {
            addFinalResult("no 'targeturlupdate' received within 20 seconds", false, 'targetUrlUpdateResult');
        }, 20000);

        var fLoadStop = function () {
            webview.removeEventListener('loadstop', fLoadStop);

            var targeturlupdateCallback = function (event) {
                webview.removeEventListener('targeturlupdate', targeturlupdateCallback);
                clearTimeout(timeout);
                addFinalResult("targeturlupdate event received: " + event.targetUrl, true, 'targetUrlUpdateResult');
            }
            webview.addEventListener('targeturlupdate', targeturlupdateCallback);
            webview.executeScript({ code: "document.links[0].focus();" }, function () { });
        }

        webview.addEventListener('loadstop', fLoadStop);
        webview.src = "https://www.novinky.cz/";
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'targetUrlUpdateResult');
    }
}