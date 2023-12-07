
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function toggle(element){
    var valveNumber = element.id.charAt(element.id.length-1);
    websocket.send(valveNumber+"v");
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
    websocket.send("getReadings");   // Κλήση της συνάρτησης για την λήψη μετρήσεων από τους αισθητήρες
    websocket.send("getValveValues");
    websocket.send("getSliderValues");
}

function onClose(event) {   // Η συνάρτηση τρέχει μετά το κλείσιμο μιας σύνδεσης
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function updateSlider(element) {
    var sliderNumber = element.id.charAt(element.id.length-1);
    var sliderValue = document.getElementById(element.id).value;
    document.getElementById("sliderValue"+sliderNumber).innerHTML = sliderValue;
    websocket.send(sliderNumber+"s"+sliderValue);
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    console.log(event.data);
    if (!isNaN(myObj['airTemperature'])) {  // Αν υπάρχει τιμή στο airTemperature, τότε προβολή της στην ιστοσελίδα
        document.getElementById('airTemperature').innerHTML = myObj["airTemperature"]; }
    if (!isNaN(myObj['airHumidity'])) {
        document.getElementById('airHumidity').innerHTML = myObj['airHumidity']; }

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
    if (myObj['airTempLimit_floor']) {
        document.getElementById('slider1').value = myObj['airTempLimit_floor'];
        document.getElementById('sliderValue1').innerHTML = myObj['airTempLimit_floor'];
    }
    if (myObj['airTempLimit_ceil']) {
        document.getElementById('slider2').value = myObj['airTempLimit_ceil'];
        document.getElementById('sliderValue2').innerHTML = myObj['airTempLimit_ceil'];
    }
}