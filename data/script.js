
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function toggle(element){
    var valveNumber = element.id.charAt(element.id.length-1);
    if (document.getElementById('state'+valveNumber).innerHTML == "Κλειστή"){
        valveState=1;
    } else {
        valveState=0;
    }
    websocket.send(valveNumber+"b"+valveState);
}

function getReadings(){     // Κλήση της συνάρτησης για την λήψη μετρήσεων από τους αισθητήρες
    websocket.send("getReadings");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {    // Η συνάρτηση τρέχει μετά την πραγματοποίηση μιας σύνδεσης
    console.log('Connection opened');
    getReadings();
    websocket.send("getValveValues");
}

function onClose(event) {   // Η συνάρτηση τρέχει μετά το κλείσιμο μιας σύνδεσης
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
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    console.log(event.data);
    if (!isNaN(myObj['temperature'])) {
        document.getElementById('temperature').innerHTML = myObj["temperature"]; }
    if (!isNaN(myObj['air-humidity'])) {
        document.getElementById('air-humidity').innerHTML = myObj['air-humidity']; }

    if (myObj['valve1']) {
        if (myObj['valve1']==1) {
            document.getElementById('state1').innerHTML = "Ανοιχτή";
        } else {
            document.getElementById('state1').innerHTML = "Κλειστή";
        }
    }
    if (myObj['valve2']) {
        if (myObj['valve2']==1) {
            document.getElementById('state2').innerHTML = "Ανοιχτή";
        } else {
            document.getElementById('state2').innerHTML = "Κλειστή";
        }
    }
    if (myObj['valve3']) {
        if (myObj['valve3']==1) {
            document.getElementById('state3').innerHTML = "Ανοιχτή";
        } else {
            document.getElementById('state3').innerHTML = "Κλειστή";
        }
    }
    if (myObj['valve4']) {
        if (myObj['valve4']==1) {
            document.getElementById('state4').innerHTML = "Ανοιχτή";
        } else {
            document.getElementById('state4').innerHTML = "Κλειστή";
        }
    }
    if (myObj['valve5']) {
        if (myObj['valve5']==1) {
            document.getElementById('state5').innerHTML = "Ανοιχτή";
        } else {
            document.getElementById('state5').innerHTML = "Κλειστή";
        }
    }
}