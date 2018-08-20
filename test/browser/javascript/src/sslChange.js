//tag browser-v0.30.4+szn.2
//Revision: 757ebf6a768e09d72cab4937eebdbf682809cd83
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 23.4.2018 11:24:53
//Message:
//'sslchange' event on webview is invoked when SSL state is changed.

//----
//    Modified: extensions / browser / extension_event_histogram_value.h
//Modified: extensions / browser / guest_view / guest_view_events.cc
//Modified: extensions / browser / guest_view / web_view / web_view_constants.cc
//Modified: extensions / browser / guest_view / web_view / web_view_constants.h
//Modified: extensions / browser / guest_view / web_view / web_view_guest.cc
//Modified: extensions / browser / guest_view / web_view / web_view_guest.h
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_events.js

// 'sslchange' event on webview is invoked when SSL state is changed.
function SSLChangeButtonFunction() {
    console.log("SSLChangeButton");
    var webview = document.getElementById("webviewId");

    var timeout = setTimeout(function () {
        addFinalResult("no 'sslchanged' received within 5 seconds", false, 'sslChangeResult');
    }, 5000);

    var sslChangeFunction = function () {
        console.log(name, "SSLChange", arguments);
        clearTimeout(timeout);
        webview.removeEventListener('sslchange', sslChangeFunction);
        addFinalResult("'sslchanged' received", true, 'sslChangeResult');
    };

    webview.addEventListener("sslchange", sslChangeFunction);

    addTestResult("Navigate webview to https://wikipedia.org");
    webview.src = "https://wikipedia.org";
}