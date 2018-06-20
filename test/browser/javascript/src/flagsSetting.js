
// test flags setting
function flagsSettingButtonFunction() {
    console.log("flagsSettingButtonFunction");
    try {
        setTimeout(function () {
            if (!document.getElementById("flagsSettingResult")) {
                addFinalResult("'flagsSettingResult' not ready within 8 seconds", false, 'flagsSettingResult');
            }
        }, 8000);

        var validList = ["disable-accelerated-video-decode", "disable-accelerated-2d-canvas"];

        nw.App.setFlagsSetting(validList, function (args) {
            if (!areArrayEqual(args, validList)) {
                arraysNotMatch();
                return;
            }
            nw.App.getFlagsSetting(function (args) {
                if (!areArrayEqual(args, validList)) {
                    arraysNotMatch();
                    return;
                }
                testList();
            });
        });
    } catch (e) {
        addFinalResult("Unexpected exception in flagsSettingButtonFunction", false, 'flagsSettingResult');
    }
}

function testList(sanitizedList, unvalidList, testListFunc) {
    var validList = ["disable-accelerated-video-decode", "disable-accelerated-2d-canvas"];
    var unValidList = ["disable-accelerated-video-decode", "non-existing-unvalid-name-bdshfsdh", "disable-accelerated-2d-canvas", "non-existing-unvalid-name-bxcvbxcv"];

    nw.App.setFlagsSetting(unValidList, function (args) {
        if (!areArrayEqual(args, validList)) {
            arraysNotMatch();
            return;
        }
        nw.App.getFlagsSetting(function (args) {
            if (!areArrayEqual(args, validList)) {
                arraysNotMatch();
                return;
            }
            addFinalResult("flagsSettingButtonFunction is working", true, 'flagsSettingResult');
        });
    });
}

function areArrayEqual(arrX, arrY) {
    if (arrX.length != arrY.length) {
        return false;
    }

    arrX.sort();
    arrY.sort();
    for (var i = 0; i < arrX.length; i++) {
        if (arrX[i] != arrY[i]) {
            return false;
        }
    }

    return true;
}

function arraysNotMatch() {
    addFinalResult("Valid list not saved right", false, 'flagsSettingResult');
}