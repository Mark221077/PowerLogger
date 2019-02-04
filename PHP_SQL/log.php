<?php

$servername = "localhost";
$username = "logger";
$password = "logger";
$dbname = "smart_home";



if(isset($_POST['ip']) && isset($_POST['station']) && isset($_POST['data'])) {
    $ipaddr = $_POST['ip'];
    $station = $_POST['station'];
    $data = $_POST['data'];

    $conn = new mysqli($servername, $username, $password, $dbname);

    if ($conn->connect_error) {
        http_response_code(500);
        exit();
    } 


    $sql = "INSERT INTO PowerLogger (IP, StationNo, Consumption, TimePosted) VALUES ('$ipaddr', '$station', '$data', NOW())";


    if ($conn->query($sql) === TRUE) {
        http_response_code(201);
    } else {
        http_response_code(500);
    }

    $conn->close();

}


?>