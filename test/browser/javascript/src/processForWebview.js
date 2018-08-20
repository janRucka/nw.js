//tag browser-v0.30.4+szn.2
//Revision: 209eb156c052a3f004fc82ff5e9e4b8484040d90
//Author: Michal Va�ek < michal.vasek@firma.seznam.cz>
//    Date: 4.5.2018 10:42:56
//Message:
//Allow create webview in new process.
//-Attribute 'usenewprocess' added on webview.If used it ensure that webview is started in new process.If not used chrome will decide whether or not new process will be used.
//----
//    Modified: extensions / browser / guest_view / web_view / web_view_constants.cc
//Modified: extensions / browser / guest_view / web_view / web_view_constants.h
//Modified: extensions / browser / guest_view / web_view / web_view_guest.cc
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_attributes.js
//Modified: extensions / renderer / resources / guest_view / web_view / web_view_constants.js

function getNumberOfProcesses(processName, callback) {
    var exec = require('child_process').exec;
    exec('C:/Windows/System32/tasklist.exe /FI "IMAGENAME eq ' + processName + '"', function (err, stdout, stderr) {
        var regex = new RegExp(processName, 'g');
        callback((stdout.match(regex) || []).length);
    });
}

//Allow create webview in new process.
//-Attribute 'usenewprocess' added on webview.If used it ensure that webview is started in new process.If not used chrome will decide whether or not new process will be used.
function processButtonFunction() {
    console.log("processButton");
    getNumberOfProcesses("nw.exe", function (numberOfProcesess1) {
        addTestResult(numberOfProcesess1 + " number of proccesses");

        var webview = document.createElement("webview")
        webview.setAttribute("partition", "persist:res");
        webview.id = "process1";
        webview.src = "https://www.google.com";
        document.body.appendChild(webview);
        getNumberOfProcesses("nw.exe", function (numberOfProcesess2) {
            addTestResult(numberOfProcesess2 + " number of proccesses after new webview without 'usenewprocess'");
            if (numberOfProcesess1 != numberOfProcesess2) {
                document.body.removeChild(webview);
                addFinalResult("New process for webview with the same partitionId!?", false, 'processResult');
                return;
            }
            var webview2 = document.createElement("webview")
            webview2.setAttribute("partition", "persist:res");
            webview2.setAttribute('usenewprocess', 'true');
            webview2.id = "process2";
            webview2.src = "https://www.google.com";
            document.body.appendChild(webview2);
            getNumberOfProcesses("nw.exe", function (numberOfProcesess3) {
                addTestResult(numberOfProcesess3 + " number of proccesses after new webview with 'usenewprocess'");
                document.body.removeChild(webview);
                document.body.removeChild(webview2);
                if (numberOfProcesess2 + 1 == numberOfProcesess3) {
                    addFinalResult("New process for webview with 'usenewprocess' and same partitionId.", true, 'processResult');
                } else {
                    addFinalResult("No new process for webview with 'usenewprocess' and same partitionId!", false, 'processResult');
                }
            });
        });
    });
}