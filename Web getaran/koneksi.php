<?php
$host = "localhost";
$user = "root";
$pass = "";
$db   = "grafiksensor";

$conn = new mysqli($host, $user, $pass, $db);
if ($conn->connect_error) {
    die("Koneksi gagal: " . $conn->connect_error);
}

$sql = "SELECT * FROM tgetaran ORDER BY id DESC";
$result = $conn->query($sql);
?>