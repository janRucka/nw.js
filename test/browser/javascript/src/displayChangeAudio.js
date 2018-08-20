//tag browser-v0.30.4+szn.2
//Revision: 780985fb20f1f6e6f5fc04f601e0706444d58a9a
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 25.4.2018 17:02:51
//Message:
//Don't change audio mute state if webview.style.display="none" to webview.style.display="flex".

//----
//    Modified: components / guest_view / browser / guest_view_base.cc

// test that we don't change audio mute state if webview.style.display="none" to webview.style.display="flex".
function displayChangeAudioButtonFunction() {
    console.log("displayChangeAudioButtonFunction");
    try {
        var webview = document.getElementById("webviewId");
        webview.setAudioMuted(true);

        webview.style.display = 'none';
        setTimeout(function () {
            webview.style.display = 'flex';

            setTimeout(function () {
                webview.isAudioMuted(function (arg) {
                    console.log(arg);
                    if (arg === true) {
                        addFinalResult("Audio is muted after display 'none' -> 'flex'", true, 'displayChangeAudioResult');
                    } else {
                        addFinalResult("Audio is not muted after display 'none' -> 'flex'", false, 'displayChangeAudioResult');
                    }
                });
            }, 1000);
        }, 1000);
    } catch (e) {
        addFinalResult("Unexpected exception", false, 'displayChangeAudioResult');
    }
}