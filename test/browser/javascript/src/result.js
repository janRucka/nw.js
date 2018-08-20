function addTestResult(str) {
    var html = document.getElementById("testResults").innerHTML;
    document.getElementById("testResults").innerHTML = str + '<br />' + html;
}

function addFinalResult(str, bResult, elemId) {
    var html = document.getElementById("testResults").innerHTML;
    var result;
    if (bResult == true) {
        result = ' <font color="green">Ok! </font>';
    } else {
        result = ' <font color="red">Failed! </font>';
    }

    document.getElementById("testResults").innerHTML = '<b>' + result + str + '</b><br />' + html;

    if (elemId === undefined) {
        return;
    }


    var elem = document.createElement('div');
    elem.id = elemId;

    if (bResult == true) {
        elem.innerHTML = 'success';
    } else {
        elem.innerHTML = 'fail';
    }
    document.body.appendChild(elem);
}