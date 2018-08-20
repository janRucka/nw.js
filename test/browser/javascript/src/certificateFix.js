//tag browser-v0.30.4+szn.2

//Revision: f57d54a08f6d54f14e86c5c2049b57248063cad5
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 23.4.2018 18:19:17
//Message:
//Certificate fix:
//-Webview now has:
//--Attribute 'useautomaticcerthandling' added on webview.If allowed following needs to be used (not possible to handle it for users)
//    --onCertificateError event (when page with invalid certificate is opened)
//--allowCertificate function (to allow page with invalid certificate)
//-Don't load subframes with invalid certificate.
//--'subframecertificateerror' event on webview added (when page containing subframe with invalid certificate is opened).
//----
//    Modified: chrome / common / extensions / api / webview_tag.json
//Modified: content / browser / ssl / ssl_error_handler.cc
//Modified: content / browser / ssl / ssl_error_handler.h
//Modified: content / browser / ssl / ssl_manager.cc
//Modified: content / browser / ssl / ssl_manager.h
//Modified: content / browser / web_contents / web_contents_impl.cc
//Modified: content / browser / web_contents / web_contents_impl.h
//Modified: content / public / browser / web_contents.h
//Modified: content / public / browser / web_contents_delegate.h
//Modified: extensions / browser / api / guest_view / web_view / web_view_internal_api.cc
//Modified: extensions / browser / api / guest_view / web_view / web_view_internal_api.h
//Modified: extensions / browser / extension_event_histogram_value.h
//Modified: extensions / browser / extension_function_histogram_value.h
//Modified: extensions / browser / guest_view / guest_view_events.cc
//Modified: extensions / browser / guest_view / web_view / web_view_constants.cc
//Modified: extensions / browser / guest_view / web_view / web_view_constants.h
//Modified: extensions / browser / guest_view / web_view / web_view_guest.cc
//Modified: extensions / browser / guest_view / web_view / web_view_guest.h
//Modified: extensions / common / api / web_view_internal.json
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_api_methods.js
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_attributes.js
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_constants.js
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_events.js

function withAttribute() {
    var webview2 = document.createElement("webview")
    webview2.setAttribute('useautomaticcerthandling', 'true');
    webview2.id = "certificateFix";
    webview2.src = "https://www.google.com";
    document.body.appendChild(webview2);

    var trueStop = function (e) {
        webview2.removeEventListener('loadstop', trueStop);
        if (webview2.src != "https://expired.badssl.com/") {
            addTestResult("webview2.src in 'trueStop' = " + webview2.src);
            addFinalResult("Didn't get to 'https://expired.badssl.com/'.", false, 'certificateFixResult');
        } else {
            addFinalResult("All right.", true, 'certificateFixResult');
        }
        document.body.removeChild(webview2);
        return;
    }

    var falseStop = function (e) {
        webview2.removeEventListener('loadstop', falseStop);
        if (webview2.src != "https://www.google.com/") {
            addTestResult("webview2.src in 'falseStop' = " + webview2.src);
            addFinalResult("Didn't get back to 'https://www.google.com'.", false, 'certificateFixResult');
            document.body.removeChild(webview2);
            return;
        }

        webview2.addEventListener("certificateerror", errorContinue);
        webview2.src = "https://expired.badssl.com/";
    }

    var errorBack = function (e) {
        webview2.removeEventListener('certificateerror', errorBack);
        addTestResult("'certificateerror' received in 'errorBack'");

        var errorStop = function (e) {
            webview2.removeEventListener('loadstop', errorStop);
            webview2.addEventListener('loadstop', falseStop);
            webview2.src = "https://www.google.com";
        }

        webview2.addEventListener('loadstop', errorStop);
        webview2.allowCertificate(false);
    }

    var errorContinue = function (e) {
        webview2.removeEventListener('certificateerror', errorContinue);
        addTestResult("'certificateerror' received in 'errorContinue'");

        webview2.addEventListener('loadstop', trueStop);
        webview2.allowCertificate(true);
    }

    webview2.addEventListener("certificateerror", errorBack);
    webview2.src = "https://expired.badssl.com/";
}

function certificateFixButtonFunction() {
    console.log("certificateFixButtonFunction");

    var webview = document.getElementById("webviewId");

    webview.addEventListener("certificateerror", function (callback) {
        addFinalResult("'certificateerror' received when it shouldn't be.", false, 'certificateFixResult');
    });

    var stop = function (e) {
        webview.removeEventListener('loadstop', stop);
        addTestResult("Test with 'useautomaticcerthandling' attribute.");
        withAttribute();
    }

    webview.addEventListener("loadstop", stop);
    webview.src = "https://expired.badssl.com/";
}