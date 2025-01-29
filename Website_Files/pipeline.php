<?php
header("X-Accel-Buffering: no");
header("Content-Type: text/event-stream");
header("Cache-Control: no-cache");

$pipePath = 'Patricks_pipe';
$pipe = fopen($pipePath, 'r');

if ($pipe) {
	//Whenever there is some data in the pipe it sends the data to the client
    while ($data = fgets($pipe)) {
		//Send the event and the data
		echo "event: ping\n";
		echo "data: $data";
		echo "\n\n";
		  
		//Sends the content of the output buffer if not empty
		if (ob_get_contents()) {
			ob_end_flush();
		}
		flush();
		
		//Break the loop if the client closed the page
		if (connection_aborted()) {
			break;
			fclose($pipe);
		}
    }
} else {
    echo "data: error with the pipe";
}
?>
