<?php
// Koneksi ke database
$host = "localhost";
$user = "root";
$pass = "";
$db   = "grafiksensor"; //nama database

$conn = new mysqli($host, $user, $pass, $db);

// Cek koneksi
if ($conn->connect_error) {
    die("Koneksi gagal: " . $conn->connect_error);
}

// Query untuk mengambil data
$sql = "SELECT * FROM tgetaran"; // Ganti dengan nama tabel Anda
$result = $conn->query($sql);

// Tampilkan data dalam bentuk tabel
if ($result->num_rows > 0) {
    echo "<table border='1'>";
    echo "<tr><th>ID</th><th>Getaran</th><th>Status Motor</th></tr>"; // Ganti header sesuai kolom tabel
    while($row = $result->fetch_assoc()) {
        echo "<tr>";
        echo "<td>".$row['id']."</td>"; 
        echo "<td>".$row['waktu']."</td>"; // Ganti 'kolom1' sesuai nama kolom
        echo "<td>".$row['getaran']."</td>"; // Ganti 'kolom2' sesuai nama kolom
        echo "</tr>";
    }
    echo "</table>";
} else {
    echo "Tidak ada data.";
}

$conn->close();
?>
