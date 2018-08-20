chrome.app.runtime.onLaunched.addListener(function () {
    console.log("chrome.app.runtime.onLaunched.addListener");

    // Center window on screen.
    var screenWidth = screen.availWidth;
    var screenHeight = screen.availHeight;
    var width = 1024;
    var height = 800;

    chrome.app.window.create('./src/index.html', {
        id: "Tests",
        outerBounds: {
            width: width,
            height: height,
            left: Math.round((screenWidth - width) / 2),
            top: Math.round((screenHeight - height) / 2)
        }
    });
})