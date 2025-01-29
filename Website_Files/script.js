function uponReset(currentStatus) {
	currentStatus.textContent =  "Current Robot Status: No errors";
	document.getElementsByClassName("container2")[0].style.setProperty('background', '#fff');
	currentStatus.style.setProperty('color', '#01b32d');
};

function uponAnError(myArray, currentStatus) {
	document.getElementsByClassName("container2")[0].style.setProperty('background', '#e7bebe');
	currentStatus.style.setProperty('color', '#a60000');
	var errorType = myArray[0];
	if (errorType == 1) {
		currentStatus.textContent =  "Error: Conveyor belt is stuck!";
		if (!playedError1Audio) {
			playError1();
			playedError1Audio = true;
		}
	} else if (errorType == 2) {
		currentStatus.textContent =  "Error: Luggage was expected but is missing!";
		if (!playedError2Audio) {
			playError2();
			playedError2Audio = true;
		}
	} else if (errorType == 3) {
		currentStatus.textContent =  "Error: RGB sensor blocked or light level too low!";
		if (!playedError3Audio) {
			playError3();
			playedError3Audio = true;
		}
	}
};

var playedManualAudio = false;
var playedError1Audio = false;
var playedError2Audio = false;
var playedError3Audio = false;

function handlePacket(myArray) {
	document.getElementById('blackCount').innerText =  myArray[0];
	document.getElementById('whiteCount').innerText = myArray[1];
	document.getElementById('miscCount').innerText = myArray[2];
	if (myArray[3] == 1) {
		document.getElementById('belt_status').innerText = "Belt status: Running";
	} else if (myArray[3] == 0) {
		document.getElementById('belt_status').innerText = "Belt status: Not running";
	}
	if (myArray[4] == 0) {
		document.getElementById('sorting_status').innerText = "Sorting status: Off";
		playedManualAudio = false;
	} else if (myArray[4] == 1) {
		document.getElementById('sorting_status').innerText = "Sorting status: Automatic";
		playedManualAudio = false;
	} else if (myArray[4] == 2) {
		document.getElementById('sorting_status').innerText = "Sorting status: Manual";
		if (!playedManualAudio) {
			playManualMode();
			playedManualAudio = true;
		}
	}
	
};

function playManualMode() {
	const audioManual = new Audio('Assets/Manual_override_mode_activated.mp3');
	audioManual.loop = false;
    audioManual.play();
};

function playError1() {
	const audioError1 = new Audio('Assets/Conveyor_belt_is_stuck.mp3');
	audioError1.loop = false;
    audioError1.play();
};

function playError2() {
	const audioError2 = new Audio('Assets/Disk_missing.mp3');
	audioError2.loop = false;
    audioError2.play();
};

function playError3() {
	const audioError3 = new Audio('Assets/RGB_blocked.mp3');
    audioError3.play();
	audioError3.loop = false;
};

var source = new EventSource("pipeline.php");
	source.addEventListener("ping", (event) => {
		//myArray holds the data packets sent from the robot
		var myArray = event.data.split(",");
		const currentStatus = document.getElementById("header");
		//Checking if the robot was reset (Arduinos)
		if (JSON.stringify(myArray) === JSON.stringify(["0","0","0","0","0"])) {
			playedError1Audio = false;
			playedError2Audio = false;
			playedError3Audio = false;
			uponReset(currentStatus);
			handlePacket(myArray);
		}
		//Checking if the packet is of length one which indicates an error or the usual length (5)
		//Python code limits possible values here to 1 or 5
		if (myArray.length == 1) {
			uponAnError(myArray, currentStatus);
		} else {
			handlePacket(myArray);
		}
});