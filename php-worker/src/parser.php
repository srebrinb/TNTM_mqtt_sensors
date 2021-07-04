<?php 
$tmpdata=file_get_contents("dump.dat");
$data=json_decode($tmpdata);
print_r($data);
?>