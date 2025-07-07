<?php
// Fungsi level getaran
function levelGet($v)
{
    if ($v <= 1.8)
        return 1;
    elseif ($v <= 2.8)
        return 2;
    elseif ($v <= 7.1)
        return 3;
    else
        return 4;
}

// Fungsi status motor
function getStatusGetaran($v)
{
    $lvl = levelGet($v);
    switch ($lvl) {
        case 1:
            return "BAIK";
        case 2:
            return "WASPADA";
        case 3:
            return "BAHAYA";
        default:
            return "KRITIS";
    }
}
?>
