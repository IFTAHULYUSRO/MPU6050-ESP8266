// auto.js
function fetchTableData() {
    $.ajax({
        url: 'get_table.php', // file PHP untuk ambil data tabel dari database
        type: 'GET',
        success: function(data) {
            $('#my-table').html(data); // update isi tabel dengan id my-table
        }
    });
}

setInterval(fetchTableData, 1000); // setiap 1 detik
