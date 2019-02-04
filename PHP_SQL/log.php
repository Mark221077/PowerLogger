<?php

$servername = "localhost";
$username = "logger";
$password = "logger";
$dbname = "smart_home";



if(isset($_POST['ip']) && isset($_POST['station']) && isset($_POST['delta'])) {
    $ipaddr = $_POST['ip'];
    $station = intval($_POST['station']);
    $delta = floatval($_POST['delta']);


    $conn = new mysqli($servername, $username, $password, $dbname);

    if ($conn->connect_error) {
        http_response_code(500);
        exit();
    } 

    $sql = "SELECT Consumption FROM PowerLogger ORDER BY ID DESC LIMIT 1";

    $prev = 0;

    $result = $conn->query($sql);

    if($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $prev = floatval($row["Consumption"]);
    }

    $consumption = $prev + $delta;

    $sql = "INSERT INTO PowerLogger (IP, StationNo, Consumption, TimePosted) VALUES ('$ipaddr', '$station', '$consumption', NOW())";


    if ($conn->query($sql) === TRUE) {
        http_response_code(201);
    } else {
        http_response_code(500);
    }

    $conn->close();

}


?>