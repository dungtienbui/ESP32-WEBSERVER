if (!!window.EventSource) {
    var source = new EventSource('/monitor_env');

    source.addEventListener('open', function (e) {
        console.log("Events Connected");
    }, false);
    source.addEventListener('error', function (e) {
        if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
        }
    }, false);

    source.addEventListener('message', function (e) {
        console.log("message", e.data);
    }, false);

    source.addEventListener('temperature', function (e) {
        console.log("temperature", e.data);
        var jsonData = JSON.parse(e.data);
        const ele = document.getElementById("temperature");
        ele.getElementsByClassName("weather_value")[0].innerHTML = jsonData["data"] + " °C";
        // ele.getElementsByClassName("timestamp")[0].innerHTML = jsonData["timestamp"];
    }, false);

    source.addEventListener('humidity', function (e) {
        console.log("humidity", e.data);
        var jsonData = JSON.parse(e.data);
        const ele = document.getElementById("humidity");
        ele.getElementsByClassName("weather_value")[0].innerHTML = jsonData["data"] + " %";
        // ele.getElementsByClassName("timestamp")[0].innerHTML = jsonData["timestamp"];
    }, false);

    source.addEventListener('moisture', function (e) {
        console.log("humidity", e.data);
        var jsonData = JSON.parse(e.data);
        const ele = document.getElementById("moisture");
        ele.getElementsByClassName("weather_value")[0].innerHTML = jsonData["data"] + " %";
        // ele.getElementsByClassName("timestamp")[0].innerHTML = jsonData["timestamp"];
    }, false);

    source.addEventListener('light', function (e) {
        console.log("humidity", e.data);
        var jsonData = JSON.parse(e.data);
        const ele = document.getElementById("light");
        ele.getElementsByClassName("weather_value")[0].innerHTML = jsonData["data"] + " lux";
        // ele.getElementsByClassName("timestamp")[0].innerHTML = jsonData["timestamp"];
    }, false);

}