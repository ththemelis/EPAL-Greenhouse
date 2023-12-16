
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

function toggleOp(){
    websocket.send(1+"o");
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
    websocket.send("getLimitValues");
    websocket.send("getOperationValue");
}

function onClose(event) {   // Η συνάρτηση τρέχει μετά το κλείσιμο μιας σύνδεσης
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function updateLimit(element) {
    var limitNumber = element.id.charAt(element.id.length-1);
    var limitVal=document.getElementById(element.id).value;
    // if (limitNumber == 1 && limitVal<27) {
    //     document.getElementById(element.id).value=27;
    //     limitVal=27;
    // }
    // console.log(limitNumber+"l"+limitVal);
    websocket.send(limitNumber+"l"+limitVal);
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    console.log(event.data);

    if (myObj['operation']) {
        if (myObj['operation']==1) {
            document.getElementById("operation").checked = true;
            document.getElementById('operationLabel').innerHTML = "Αυτόματη λειτουργία";
            for (let i=1; i <= 5; i++){
                document.getElementById("valve" + i).disabled = true;
            }
        } else {
            document.getElementById("operation").checked = false;
            document.getElementById('operationLabel').innerHTML = "Χειροκίνητη λειτουργία";
            for (let i=1; i <= 5; i++){
                document.getElementById("valve" + i).disabled = false;
            }
        }
    }

    if (!isNaN(myObj['airTemperature'])) {  // Αν υπάρχει τιμή στο airTemperature, τότε προβολή της στην ιστοσελίδα
        document.getElementById('airTemperature').innerHTML = myObj["airTemperature"]; }
    if (!isNaN(myObj['airHumidity'])) {
        document.getElementById('airHumidity').innerHTML = myObj['airHumidity']; }

    if (myObj['valve1']) {
        if (myObj['valve1']==1) {
            document.getElementById('valve1').innerHTML = "On";
            document.getElementById("valve1").setAttribute('class', 'btn btn-success');
            document.getElementById("state1").setAttribute('class', 'bi bi-1-circle-fill')
        } else {
            document.getElementById('valve1').innerHTML = "Off";
            document.getElementById("valve1").setAttribute('class', 'btn btn-danger');
            document.getElementById("state1").setAttribute('class', 'bi bi-1-circle')
        }
    }
    if (myObj['valve2']) {
        if (myObj['valve2']==1) {
            document.getElementById('valve2').innerHTML = "On";
            document.getElementById("valve2").setAttribute('class', 'btn btn-success');
            document.getElementById("state2").setAttribute('class', 'bi bi-2-circle-fill')
        } else {
            document.getElementById('valve2').innerHTML = "Off";
            document.getElementById("valve2").setAttribute('class', 'btn btn-danger');
            document.getElementById("state2").setAttribute('class', 'bi bi-2-circle')
        }
    }
    if (myObj['valve3']) {
        if (myObj['valve3']==1) {
            document.getElementById('valve3').innerHTML = "On";
            document.getElementById("valve3").setAttribute('class', 'btn btn-success');
            document.getElementById("state3").setAttribute('class', 'bi bi-3-circle-fill')
        } else {
            document.getElementById('valve3').innerHTML = "Off";
            document.getElementById("valve3").setAttribute('class', 'btn btn-danger');
            document.getElementById("state3").setAttribute('class', 'bi bi-3-circle')        }
    }
    if (myObj['valve4']) {
        if (myObj['valve4']==1) {
            document.getElementById('valve4').innerHTML = "On";
            document.getElementById("valve4").setAttribute('class', 'btn btn-success');
            document.getElementById("state4").setAttribute('class', 'bi bi-4-circle-fill')
        } else {
            document.getElementById('valve4').innerHTML = "Off";
            document.getElementById("valve4").setAttribute('class', 'btn btn-danger');
            document.getElementById("state4").setAttribute('class', 'bi bi-4-circle')        }
    }
    if (myObj['valve5']) {
        if (myObj['valve5']==1) {
            document.getElementById('valve5').innerHTML = "On";
            document.getElementById("valve5").setAttribute('class', 'btn btn-success');
            document.getElementById("state5").setAttribute('class', 'bi bi-5-circle-fill')
        } else {
            document.getElementById('valve5').innerHTML = "Off";
            document.getElementById("valve5").setAttribute('class', 'btn btn-danger');
            document.getElementById("state5").setAttribute('class', 'bi bi-5-circle')        }
    }
    if (myObj['airTempLimit_floor']) {
        document.getElementById('limit1').value = myObj['airTempLimit_floor'];
    }
    if (myObj['airTempLimit_ceil']) {
        document.getElementById('limit2').value = myObj['airTempLimit_ceil'];
    }
    if (myObj['airHumLimit_floor']) {
        document.getElementById('limit3').value = myObj['airHumLimit_floor'];
    }
    if (myObj['airHumLimit_ceil']) {
        document.getElementById('limit4').value = myObj['airHumLimit_ceil'];
    }
    if (myObj['gndHumLimit_floor']) {
        document.getElementById('limit5').value = myObj['gndHumLimit_floor'];
    }
    if (myObj['gndHumLimit_ceil']) {
        document.getElementById('limit6').value = myObj['gndHumLimit_ceil'];
    }
}

