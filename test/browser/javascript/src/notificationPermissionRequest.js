var notificationPermissionRequestFunction = (function(){
    "use strict";

    const EventEmitter = require('events');

    class Telephone extends EventEmitter {

        constructor () {
            super();

            this._ports = new Set();
            this._listeners = new WeakMap();
            this._boundOnConnect = this._onConnect.bind(this);

            chrome.runtime.onConnect.addListener(this._boundOnConnect);
        }

        destructor () {
            chrome.runtime.onConnect.removeListener(this._boundOnConnect);
            this.removeAllListeners();

            const ports = Array.from(this._ports);
            ports.forEach((port) => {
                this._onDisconnect(port);
                port.disconnect();
            });

            this._ports.clear();

            delete this._ports;
            delete this._listeners;
            delete this._boundOnConnect;
        }

        _onConnect (port) {
            console.log("[Telephone] Rrrriiiiinnnnnggg!", port);

            let reject = false;
            this.emit("ring", port.name, function(){ reject = true; });

            if (reject) {
                console.warn("[Telephone] Last call rejected");
                return;
            }

            console.log("[Telephone] Last call accepted");

            const listeners = {
                onMessage: (message) => this._onMessage(port, message),
                onDisconnect: () => this._onDisconnect(port)
            };

            port.onDisconnect.addListener(listeners.onDisconnect);
            port.onMessage.addListener(listeners.onMessage);

            this._listeners.set(port, listeners);
            this._ports.add(port);
        }

        _onDisconnect (port) {
            if (!(this._ports && this._ports.has(port) && this._listeners.has(port))) return console.warn("Got disconect for unknown port!!!");

            this._ports.delete(port);
            const { onDisconnect, onMessage } = this._listeners.get(port) || {};

            port.onDisconnect.removeListener(onDisconnect);
            port.onMessage.removeListener(onMessage);

            console.log("[Telephone] Hang up", port);
        }

        _onMessage (port, message) {
            console.log("[Telephone] RECV:", message);
            this.emit("message", message, port);
        }

        postMessage (message) {
            let count = 0;
            for (let port of this._ports) {
                port.postMessage(message);
                count++;
            }

            if (count > 0) {
                console.log("[Telephone] SEND:", message);
            } else {
                console.warn("[Telephone] NO CALLS IN PROGRESS!", message);
            }
        }
    }

    function insideWebview () {
        // Tohle se injectne do webview jako string, takže žádný reference ven !!!!
        console.log("[notificationPermissionRequestFunction]", "Code inside webview started...");

        const RUNTIME_ID = $$RUNTIME_ID$$; // $$RUNTIME_ID$$ se nahradí za ID naší extensiony
        const port = chrome.runtime.connect(RUNTIME_ID, { name: location.href });
        port.onMessage.addListener((message) => {
            try {
                switch (message) {
                    case "ask-permission": {
                        console.log("[notificationPermissionRequestFunction]", "Asking permission...");
                        Notification.requestPermission(function(permission) {
                            port.postMessage(["permission", permission]);
                        });
                        break;
                    }

                    case "show-notification": {
                        console.log("[notificationPermissionRequestFunction]", "Showing notification...");
                        const n = new Notification("Notification Test", {
                            body: "Hello, I am a notification for testing purposes"
                        });

                        n.addEventListener("show", function () {
                            port.postMessage(["notification-shown"]);
                        });

                        n.addEventListener("error", function (evt) {
                            port.postMessage(["error", "notification-errored", (evt.error ? evt.error.message + "" : "Unknown notification error, permission=" + Notification.permission)]);
                        });
                        break;
                    }

                    default: {
                        port.postMessage(["error", "unknown-message", message]);
                    }
                }
            } catch (err) {
                port.postMessage(["error", "uncaught-error", err.message]);
            }
        });
        port.postMessage(["hello"]);
    }

    function load (webview, url) {
        return new Promise((resolve, reject) => {
            let removeListeners, notStarted;

            const finish = (err = null) => {
                removeListeners();
                clearTimeout(notStarted);

                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            };

            notStarted = setTimeout(() => finish(new Error("Webview haven't started loading!!!")), 1000);

            const onLoadStart = function (evt) {
                if (!evt.isTopLevel) return;
                console.log("[notificationPermissionRequestFunction]", "LoadStart", evt.url);
                clearTimeout(notStarted);
            };

            const onLoadAbort = function (evt) {
                if (!evt.isTopLevel) return console.warn("[notificationPermissionRequestFunction]", "LoadAbort", evt.url);
                finish(new Error("Webview aborted!"));
            };

            const onLoadStop = function (evt) {
                webview.stop();
                console.log("[notificationPermissionRequestFunction]", "LoadStop", webview.src);
                finish();
            };

            removeListeners = function () {
                webview.removeEventListener("loadstart", onLoadStart);
                webview.removeEventListener("loadabort", onLoadAbort);
                webview.removeEventListener("loadstop", onLoadStop);
            };

            webview.addEventListener("loadstart", onLoadStart);
            webview.addEventListener("loadabort", onLoadAbort);
            webview.addEventListener("loadstop", onLoadStop);

            webview.src = url;
        });
    }

    function testWithUrl (url, showDevTools = false) {
        const webview = document.querySelector("webview");

        return load(webview, url).then(
            () => new Promise((resolve, reject) => {
                if (!showDevTools) return resolve();
                try {
                    webview.showDevTools(true);
                    setTimeout(resolve, 500);
                } catch (err) { reject(err); }
            })
        ).then(
            () => new Promise((resolve, reject) => {
                try {
                    let notConnect, globalTimeout, listenerRemovers = [];

                    const telephone = new Telephone();

                    const finish = (err = null) => {
                        clearTimeout(notConnect);
                        clearTimeout(globalTimeout);
                        telephone.destructor();
                        listenerRemovers.forEach((remover) => remover());
                        listenerRemovers = [];

                        if (err) {
                            reject(err);
                        } else {
                            resolve();
                        }
                    };

                    
                    telephone.on("ring", (callee, rejectCall) => {
                        if (callee !== url) return rejectCall();
                        clearTimeout(notConnect);
                    });

                    let state, allowPermissionRequest;

                    const onPermissionrequest = (evt) => {
                        const permission = evt.permission;
                        if (permission !== 'notification') {
                            evt.request.deny();
                            finish(new Error("Received invalid permission request", permission));
                            return;
                        }

                        if (allowPermissionRequest) {
                            console.log("[notificationPermissionRequestFunction]", "grant permission", permission);
                            evt.request.allow();
                        } else {
                            console.log("[notificationPermissionRequestFunction]", "deny permission", permission);
                            evt.request.deny();
                        }
                        state++;
                    };

                    webview.addEventListener("permissionrequest", onPermissionrequest);
                    listenerRemovers.push(() => webview.removeEventListener("permissionrequest", onPermissionrequest));

                    telephone.on("message", (message) => {
                        if (!(Array.isArray(message) && message.length > 0)) return console.warn("Invalid message received");

                        switch (message[0]) {
                            case "error":{
                                const err = new Error(message[2] || message[1]);
                                err.code = message[1];

                                finish(err);
                                break;
                            }

                            case "hello":
                                state = 0;
                                allowPermissionRequest = false;
                                telephone.postMessage("ask-permission");
                                break;
                            
                            case "permission":
                                if (state === 0) {
                                    return finish(new Error("Permissionrequest 1 wasn't received, state=" + state));
                                } else if (state === 1) {
                                    if (message[1] !== "denied") return finish(new Error("Permission should be denied!!!, state=" + state));
                                    state++;

                                    allowPermissionRequest = true;
                                    telephone.postMessage("ask-permission");
                                } else if (state === 2) {
                                    return finish(new Error("Permissionrequest 2 wasn't received, state=" + state));
                                } else if (state === 3) {
                                    if (message[1] !== "granted") return finish(new Error("Permission should be granted!!!, state=" + state));
                                    state++;

                                    allowPermissionRequest = true;
                                    telephone.postMessage("show-notification");
                                } else {
                                    finish(new Error("Invalid state in 'permission', state=" + state));
                                }
                                break;

                            case "notification-shown":
                                if (state === 4) {
                                    finish();
                                } else {
                                    finish(new Error("Invalid state in 'notification-shown', state=" + state));
                                }
                                break;
                        }
                    });

                    console.log("[notificationPermissionRequestFunction]", "Injecting JS into webview");
                    webview.executeScript({ code: `(${insideWebview.toString().replace(/\$\$RUNTIME_ID\$\$/g, JSON.stringify(chrome.runtime.id))})();` });

                    notConnect = setTimeout(() => finish(new Error("Unable to connect through telephone....")), 1000);
                    globalTimeout = setTimeout(() => finish(new Error("TIMEOUT!!!")), 10000);
                } catch (err) { reject(err); }
            })
        );
    }
    
    
    function test () {
        const SHOW_DEV_TOOLS = false;

        testWithUrl("https://search.seznam.cz/", SHOW_DEV_TOOLS).then(
            () => {
                addTestResult("[notificationPermissionRequest] SUCCESS on https://search.seznam.cz/");

                return testWithUrl("http://zskravare.cz/", SHOW_DEV_TOOLS).then(
                    () => { throw new Error("This should fail...."); },
                    (err) => {
                        if (["notification-errored"].includes(err.code)) {
                            addTestResult("[notificationPermissionRequest] SUCCESS on http://zskravare.cz/... ("+err.message+")");
                        } else {
                            throw err;
                        }
                    }
                );
            }
        ).then(
            () => addFinalResult("Notifications WORK", true, 'notificationPermissionRequestResult'),
            (err) => addFinalResult("Notifications are BROKEN!!! " + err.message, false, 'notificationPermissionRequestResult')
        );
    }

    return test;
})();
