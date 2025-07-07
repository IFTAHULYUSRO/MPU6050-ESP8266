<?php
include 'koneksi.php';

$conn = new mysqli($host, $user, $pass, $db);
if ($conn->connect_error) {
    die("Koneksi gagal: " . $conn->connect_error);
}
date_default_timezone_set('Asia/Jakarta');

// Ubah bagian ini agar bisa menerima GET atau POST
$getaran = null;

if (isset($_POST['getaran'])) {
    $getaran = $_POST['getaran'];
} elseif (isset($_GET['getaran'])) {
    $getaran = $_GET['getaran'];
}

if ($getaran !== null) {
    $stmt = $conn->prepare("INSERT INTO tgetaran (getaran) VALUES (?)");
    $stmt->bind_param("s", $getaran);

    if ($stmt->execute()) {
        echo "Data berhasil disimpan!";
    } else {
        echo "Error: " . $stmt->error;
    }
    $stmt->close();
} else {
    echo "Data tidak lengkap!";
}

$conn->close();
?>
