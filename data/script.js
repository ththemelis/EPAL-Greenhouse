
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
    //initButton();
}

// function initButton() {
//     document.getElementById('valve1').addEventListener('click', toggle);
//     document.getElementById('valve2').addEventListener('click', toggle);
//     document.getElementById('valve3').addEventListener('click', toggle);
//     document.getElementById('valve4').addEventListener('click', toggle);
//     document.getElementById('valve5').addEventListener('click', toggle);
// }

function toggle(element){
    var valveNumber = element.id.charAt(element.id.length-1);
    //var valveState;
    if (document.getElementById('state'+valveNumber).innerHTML == "Κλειστή"){
        document.getElementById('state'+valveNumber).innerHTML = "Ανοιχτή";
        valveState=1;
    }
    else {
        document.getElementById('state'+valveNumber).innerHTML = "Κλειστή";
        valveState=0;
    }
    //websocket.send("toggle");
    websocket.send(valveNumber+"b"+valveState);
    // websocket.send(valveNumber+"b");
    console.log(valveNumber+"b"+valveState);
}

function getReadings(){
    websocket.send("getReadings");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    getReadings();
    websocket.send("getValveValues");
    //initButton();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function updateSliderPWM(element) {
    var sliderNumber = element.id.charAt(element.id.length-1);
    var sliderValue = document.getElementById(element.id).value;
    document.getElementById("sliderValue"+sliderNumber).innerHTML = sliderValue;
    //console.log(sliderValue);
    websocket.send(sliderNumber+"s"+sliderValue.toString());
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    //console.log(event.data);
    var state;
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        // if (key == 'temperature' || key == "humidity" || key == "state1") {
            document.getElementById(key).innerHTML = myObj[key];
            console.log(myObj[key]);
        // }
    }
}