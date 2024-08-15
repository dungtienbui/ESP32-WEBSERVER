var gateway = `ws://${window.location.hostname}/ws`;
var websocket;


function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage; // <-- add this line
}
function onOpen(event) {
    console.log('Connection opened');
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    console.log(`Event websocket data ${event.data}`)

    var datajson = JSON.parse(event.data);

    const relay_id = datajson["id"];
    const relay_state = datajson["state"];

    const spanE = document.querySelectorAll(`#relay_state${relay_id}`);
    spanE[0].innerHTML = relay_state ? "ON" : "OFF";
    spanE[0].style.color = relay_state ? "blue" : "red";
    
}
window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
    initButton();
}

function initButton() {
    const buttons = document.querySelectorAll(".control_relay_block button");

    buttons.forEach(button => {
        button.addEventListener('click', () => toggle(button.id));
    });
}

function toggle(id) {
    console.log(`Button with id ${id} clicked`);
    websocket.send(`{"action":"toggle", "id":${id}}`);
}