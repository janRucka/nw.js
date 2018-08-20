//tag browser-v0.30.4+szn.2
//Revision: 048dd652652a803f4389dcd319fac501f66d0dc8
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 23.4.2018 18:42:52
//Message:
//On title and favicon change event on webview:
//-titlechange
//    - faviconchange
//----
//    Modified: extensions / browser / extension_event_histogram_value.h
//Modified: extensions / browser / guest_view / guest_view_events.cc
//Modified: extensions / browser / guest_view / web_view / web_view_constants.cc
//Modified: extensions / browser / guest_view / web_view / web_view_constants.h
//Modified: extensions / browser / guest_view / web_view / web_view_guest.cc
//Modified: extensions / browser / guest_view / web_view / web_view_guest.h
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_events.js

function isEmpty(str) {
    return (!str || 0 === str.length);
}

function titleFaviconEventButtonFunction() {
    console.log("titleFaviconEventButtonFunction");

    var webview = document.getElementById("webviewId");

    var titleTimeout = setTimeout(function () {
        if (!document.getElementById("titlechangeResult")) {
            addFinalResult("no 'titlechange' received within 20 seconds", false, 'titlechangeResult');
        }
    }, 20000);

    var faviconTimeout = setTimeout(function () {
        if (!document.getElementById("faviconchangeResult")) {
            addFinalResult("no 'faviconchange' received within 20 seconds", false, 'faviconchangeResult');
        }
    }, 20000);


    webview.addEventListener("titlechange", function (arg) {
        if (!isEmpty(arg.title) && !document.getElementById("titlechangeResult")) {
            addFinalResult("'titlechange' received", true, 'titlechangeResult');
            clearTimeout(titleTimeout);
        }
        console.log(name, "titlechange", url)
    });

    webview.addEventListener("faviconchange", function (arg) {
        if (!isEmpty(arg.faviconUrl) && !document.getElementById("faviconchangeResult")) {
            addFinalResult("'faviconchange' received", true, 'faviconchangeResult');
            clearTimeout(faviconTimeout);
        }
        console.log(name, "faviconchange", url)
    });

    webview.src = "https://www.novinky.cz";
}