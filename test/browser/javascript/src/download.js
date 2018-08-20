//tag browser-v0.30.4+szn.2
//Revision: 94aedca3c0cdcfeefa8f0106f7b2490c7eaa1a04
//Author: Michal Vašek < michal.vasek@firma.seznam.cz>
//    Date: 23.4.2018 15:35:44
//Message:
//PartitionId added to options in chrome.downloads.download.For identifying where downloads were created.

//----
//    Modified: chrome / browser / extensions / api / downloads / downloads_api.cc
//Modified: chrome / common / extensions / api / downloads.idl
//Modified: components / download / public / common / download_create_info.h
//Modified: components / download / public / common / download_item.h
//Modified: content / browser / download / download_item_impl.cc
//Modified: content / browser / download / download_item_impl.h
//Modified: content / browser / download / download_resource_handler.cc
//Modified: content / browser / web_contents / web_contents_impl.cc
//Modified: content / browser / web_contents / web_contents_impl.h
//Modified: content / public / browser / web_contents.h

function onDownload(downloadItem) {
    console.log(downloadItem);
    chrome.downloads.cancel(downloadItem.id);

    if (typeof downloadItem.partitionId === "undefined") {
        addFinalResult("no 'partitionId' in downloadItem", false, 'downloadResult');
    }

    if (downloadItem.partitionId.includes("persist?res")) {
        addFinalResult("Right 'partitionId' in downloadItem", true, 'downloadResult');
    } else {
        addFinalResult("Wrong 'partitionId' in downloadItem", false, 'downloadResult');
    }
}

function downloadIdButtonFunction() {
    console.log("downloadIdButtonFunction");

    chrome.downloads.onCreated.addListener(onDownload);
    var webview = document.getElementById("webviewId");

    webview.addEventListener("permissionrequest", function (e) {
        console.info(name, "permissionrequest", arguments)
        if (e.permission === 'download') {
            console.info(name, "permissionrequest-download", arguments)
            e.request.allow();
        } else {
            e.request.deny();
        }
    });

    webview.src = "https://www.seznam.cz/prohlizec/stahnout/Seznam.cz.exe";
}