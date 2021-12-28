// const { json } = require("stream/consumers");

let url = 'wss://fine.asuscomm.com:8888/ws';
const socket = new WebSocket(url);
let factor = {x: 1, y: 1, rotation: 1};         // scaling the x, y, rotation ratios
let rate = 1000;                                 // 100ms = 10 x/sec
let jOBJ = {};
let jSTR = "";
let messageFlag = "";

// <page selection
let pages = ["TouchPad", "Direction", "Destination", "WayPoints", "PID", "Display"];
 p = document.getElementById("select");
// <initialization of page on load
let a = document.querySelector('input[name="page"]:checked').value;
    pages.forEach(element => {
        let d = document.getElementById(element);
        if (element === a) {
            d.style.display = 'inline-block';
        } else { d.style.display = 'none';}
    });
// initialization of page on load>
p.addEventListener('change', (e)=>{
    let a = document.querySelector('input[name="page"]:checked').value;
    pages.forEach(element => {
        let d = document.getElementById(element);
        if (element === a) {
            d.style.display = 'inline-block';
        } else { d.style.display = 'none';}
    });
});
// page selection>


// deal with socket (client & server)
socket.addEventListener('open', (e)=>{                                          // socket - open / setInterval
    console.log('websocket connected ...');
    setInterval(() => {
        if (messageFlag === "1"){
            jSTR = "";
            messageFlag = "";
        }
        if (messageFlag){
            jSTR = messageFlag;
            messageFlag = "1";
        }
        if (jSTR === "") {
            jSTR = `{"alive": "true"}`
        }
        // console.log(jSTR);
        socket.send(jSTR);
        document.getElementById('message1').innerText = jSTR;
        document.getElementById('message2').innerText = jSTR;

    }, rate);
})
socket.addEventListener('message', (e)=>{                                       // socket - on message / receive from ESP32
    console.log('received from HTTP server: ' + e.data);
})




//  <TouchPad
let scr = {x: 0, y: 0};                         // screen size
let touches = 0;                                // 1-touch, 2-touches or 3-touches
let current = {currentState: 0, rotate: 0, outputL: 0, outputR: 0, oneX: 0, oneY: 0, twoX: 0, twoY: 0, moveX: 0, moveY: 0};

scr.x = window.visualViewport.width;                // get screen size
scr.y = window.visualViewport.height;
let cv = document.getElementById('myCanvas');       // set canvas
cv.width = scr.x - 40;
cv.height = scr.y - 300;


let arr = ['x', 'y', 'd'];

function changeStateTo (s, t) {     // react to different states = one/2/1 - finger touch events
    switch (s) {
        case 0:
            console.log("change s to 0");
            current.currentState = 0;
            break;
    
        case 1:
            console.log("change s to 1");
            switch (current.currentState) {
                case 0:
                    current.currentState = 1;
                    current.oneX = t.touches[0].screenX;
                    current.oneY = t.touches[0].screenY;
                    break;
            
                case 1:
                    current.currentState = 1;
                    break;

                case 2:
                    current.currentState = 1;
                    current.oneX = t.touches[0].screenX;
                    break;

                case 3:
                    current.currentState = 1;
                    break;
            }
            break;

        case 2:
            console.log("change s to 2");
            switch (current.currentState) {
                case 0:
                    current.currentState = 2;
                    current.twoX = t.touches[0].screenX
                    current.twoY = t.touches[0].screenY
                    break;

                case 1:
                    current.currentState = 2;
                    current.twoX = t.touches[0].screenX
                    current.twoY = t.touches[0].screenY
                    break;

                case 2:
                    break;

                case 3:
                    current.currentState = 2;
                    current.twoX = t.touches[0].screenX
                    current.twoY = t.touches[0].screenY
                    break;
            }
            break;

        case 3:
            console.log("change s to 3");
            switch (current.currentState) {
                case 0:
                    current.currentState = 3;
                    current.oneX = t.touches[0].screenX;
                    current.oneY = t.touches[0].screenY;
                    break;
            
                case 1:
                    current.currentState = 3;
                    break;

                case 2:
                    current.currentState = 3;
                    current.oneX = t.touches[0].screenX;
                    break;

                case 3:
                    break;
            }
            break;
    }


}

function outputLR (t) {             // set JSON(output) according to states and fingers positions
    switch (current.currentState) {
        case 0:
            current.outputL = 0;
            current.outputR = 0;
            console.log(current.outputL + "-----" + current.outputR);
        
            jOBJ.L = 0;
            jOBJ.R = 0;
            

            document.getElementById(arr[0]).innerText = jOBJ.L;
            document.getElementById(arr[1]).innerText = jOBJ.R;
            document.getElementById(arr[2]).innerText = 0;
            
            break;
        
        case 1:
            current.moveX = t.touches[0].screenX;
            current.moveY = t.touches[0].screenY;
            current.outputL = (current.moveX - current.oneX)*2*factor.x/scr.x;
            current.outputR = (current.oneY - current.moveY)*2*factor.y/scr.y;
            console.log(current.outputL + "-----" + current.outputR);
            
            document.getElementById(arr[0]).innerText = jOBJ.L;
            document.getElementById(arr[1]).innerText = jOBJ.R;
            document.getElementById(arr[2]).innerText = 0;
            // d3.innerText = `no rotation ${current.currentState}`;
            jOBJ.L = current.outputL;
            jOBJ.R = current.outputR;
            break;
        
        case 2:
            current.moveX = t.touches[0].screenX 
            current.moveY = t.touches[0].screenY 
            current.rotate = (t.touches[0].screenX - current.twoX)*2 / scr.x;
            console.log(current.rotate);

            let i = current.rotate*2*100/scr.x
            current.outputR = -i;
            current.outputL = i;
            jOBJ.L = current.outputL;
            jOBJ.R = current.outputR;
            document.getElementById(arr[0]).innerText = 0;
            document.getElementById(arr[1]).innerText = 0;
            document.getElementById(arr[2]).innerText = current.rotate;
            break;

        case 3:
            current.moveX = t.touches[0].screenX;
            current.moveY = t.touches[0].screenY;
            current.outputL = (current.moveX - current.oneX)*2*factor.x/scr.x;
            current.outputR = (current.oneY - current.moveY)*2*factor.y/scr.y;

            jOBJ.L = current.outputL;
            jOBJ.R = current.outputR;
            document.getElementById(arr[0]).innerText = jOBJ.L;
            document.getElementById(arr[1]).innerText = jOBJ.R;
            document.getElementById(arr[2]).innerText = 0;

            break;
    }
    jSTR = JSON.stringify(jOBJ);                    // turn outputLR or rotation into STRING
}


// <TouchPad events
cv.addEventListener('touchstart', (t)=>{                                        // touch - start    
    t.preventDefault();
    console.log("touch started");
    touches = t.touches.length;
    if (touches > 3) { touches = 3};
    console.log (touches)
    changeStateTo(touches, t);
    outputLR(t);
}, {passive : false })

cv.addEventListener('touchmove', (t)=>{                                         // touch - move
    t.preventDefault();
    console.log("touch moved");
    outputLR(t);
}, {passive : false })

cv.addEventListener('touchend', (t)=>{                                          // touch - end
    t.preventDefault();
    console.log("touch ended");
    touches = t.touches.length;
    if (touches > 3) { touches = 3};
    changeStateTo(touches, t);
    outputLR(t);
}, {passive : false })
// TouchPad Events>

// <PID Events
let btn = document.getElementById('button')
let ar = ['ms', 'kP', 'kI', 'kD', 'direction', 'speed'];

ar.forEach(a => {                                               // addEventListeners
    let v = document.getElementById(a);
    v.addEventListener('input',(e)=>{
        let i = document.getElementById(`value-${e.target.name}`);
        i.innerText = e.target.value;
    });    
});
btn.addEventListener('click', (e)=>{                            // add Click event
    let obj = {};
    obj.cmd = "PID";
    ar.forEach(a => {
        let i = document.getElementById(a);
        obj[a] = i.value;
    let st = JSON.stringify(obj);
    messageFlag = st;
    })
    // console.log(messageFlag);
});
// PID Events>




// window.addEventListener("deviceorientation", h, true);
if (window.DeviceOrientationEvent) {
    document.getElementById('or').innerText = 'GOT DEVICE';    
} else { document.getElementById('or').innerText = 'no device';}
if (window.ondeviceorientation === null) {
    document.getElementById('or2').innerText = "ondeviceorientation = null"; 
} 

function requestOrientationPermission(){
    try {
        DeviceOrientationEvent.requestPermission()
        .then(response => {
            if (response == 'granted') {
                window.addEventListener('deviceorientation', (e) => {
                    console.log(e);
                    console.log(e.absolute);
                    document.getElementById('or').innerText = e.absolute;
                    document.getElementById('or2').innerText = e.alpha;

                })
            }
        })
        .catch(console.error)
    }
    catch (error) {
        console.error("Orientation Sensor does not work unless on iPhone");
    }
}


function h (position){
    const lat = position.coords.latitude;
    const lon = position.coords.longitude;
    alert(`${lat} and ${lon}`);
    alert(`${m.href} and ${m}`);
    
    m.href = `https://www.openstreetmap.org/#map=18/${lat}/${lon}`;
    o.src = `https://www.openstreetmap.org/#map=18/${lat}/${lon}`;
    alert (`${o.src}`);

}
function e (error){
    alert(`ERROR (${error.code}); ${error.message}`);
}
const g = navigator.geolocation.getCurrentPosition(h,e);
const m = document.getElementById('map');
const o = document.getElementById('map1');
const n = document.getElementById('map2');
alert(`${m} and ${n}`);
m.href = '';
