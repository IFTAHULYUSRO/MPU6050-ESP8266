<?php
include 'koneksi.php';
include 'getaran.php';?>
<?php
// Set timezone to Asia/Jakarta
date_default_timezone_set('Asia/Jakarta');

// Query data dari database
$sql = "SELECT * FROM tgetaran ORDER BY waktu DESC";
$result = $conn->query($sql);
?>

<!-- Reload otomatis setiap 10 detik -->
<meta http-equiv="refresh" content="10">
<!DOCTYPE html>
<html>
<head>
    <title>Monitoring Getaran</title>
    <?php
    date_default_timezone_set('Asia/Jakarta');
    ?>
    <style>
        body { margin: 0; padding: 0; }
        h2 { text-align: center; }
        .table-container {
            width: 800px;
            margin: 30px auto 0 auto;
        }
        table { 
            border-collapse: collapse; 
            width: 100%; 
            margin-top: 20px; 
        }
        th, td { 
            border: 1px solid #aaa; 
            padding: 8px; 
            text-align: center; 
        }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <h2 style="margin-top: 20px;">Nilai Getaran Pada Motor</h2>
    <div class="table-container">
        <table>
            <tr>
                <th>No</th>
                <th>Waktu</th>
                <th>Tanggal</th>
                <th>Getaran</th>
                <th>Status Motor</th>
            </tr>
            <?php
            if ($result->num_rows > 0) {
                $no = 1;
                $limit = 10;
                $count = 0;
                while($row = $result->fetch_assoc()) {
                    if ($count >= $limit) break;
                    $datetime = strtotime($row['waktu']);
                    $waktu = date('H:i:s', $datetime);
                    $tanggal = date('d-m-Y', $datetime);
                    echo "<tr>
                        <td>".$no++."</td>
                        <td>".$waktu."</td>
                        <td>".$tanggal."</td>
                        <td>".$row['getaran']."</td>
                        <td>" . getStatusGetaran($row['getaran']) . "</td>
                    </tr>";
                    $count++;
                }
            } else {
                echo "<tr><td colspan='5'>Belum ada data</td></tr>";
            }
            ?>
        </table>
    </div>
    <script src="auto.js"></script>
</body>
</html>
