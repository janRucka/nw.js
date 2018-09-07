document.addEventListener("DOMContentLoaded", function () {
    console.log("DOMContentLoaded");
    document.getElementById("goButton").addEventListener("click", goButtonFunction);
    document.getElementById("canGoBackButton").addEventListener("click", canGoBackButtonFunction);
    document.getElementById("backButton").addEventListener("click", backButtonFunction);

    document.getElementById("processButton").addEventListener("click", processButtonFunction);
    document.getElementById("SSLChangeButton").addEventListener("click", SSLChangeButtonFunction);
    document.getElementById("downloadIdButton").addEventListener("click", downloadIdButtonFunction);
    document.getElementById("certificateFixButton").addEventListener("click", certificateFixButtonFunction);
    document.getElementById("historyTitleButton").addEventListener("click", historyTitleButtonFunction);
    document.getElementById("titleFaviconEventButton").addEventListener("click", titleFaviconEventButtonFunction);
    document.getElementById("historyButton").addEventListener("click", historyButtonFunction);
    document.getElementById("targetUrlUpdateButton").addEventListener("click", targetUrlUpdateButtonFunction);
    document.getElementById("displayChangeAudioButton").addEventListener("click", displayChangeAudioButtonFunction);
    document.getElementById("defaultBrowserButton").addEventListener("click", defaultBrowserButtonFunction);
    document.getElementById("ieBookmarksButton").addEventListener("click", ieBookmarksButtonFunction);
    document.getElementById("flagsSettingButton").addEventListener("click", flagsSettingButtonFunction);
    document.getElementById("notificationPermissionRequestButton").addEventListener("click", notificationPermissionRequestFunction);

    var webview = document.getElementById("webviewId");
    webview.addEventListener("loadstop", function () {
        document.getElementById("canGoBackButton").disabled = !webview.canGoBack();
        console.log(name, "loadstop", webview.src, arguments);
    });

    webview.addEventListener("audiostatechanged", function (e) {
        console.log(name, "audiostatechanged", arguments);
        if (e.audible === true) {
            document.getElementById("audiostatechangeText").innerHTML = "audible";
        } else {
            document.getElementById("audiostatechangeText").innerHTML = "not audible";
        }
    });

    webview.style.width = '100%';
    webview.style.height = '100%';
});