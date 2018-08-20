function goButtonFunction() {
    console.log("goButtonFunction");
    var webview = document.getElementById("webviewId");
    webview.src = document.getElementById('website').value;
}

function canGoBackButtonFunction() {
    var webview = document.getElementById("webviewId");
    console.log("canGoBackButtonFunction: " + webview.canGoBack());
    webview.back();
}

function backButtonFunction() {
    console.log("backButtonFunction");
    var webview = document.getElementById("webviewId");
    webview.back();
}