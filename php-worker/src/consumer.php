<?php
require __DIR__ . '/../vendor/autoload.php';
$server   = '3.67.237.255';//'192.168.1.10';
$port     = 1883;
$clientId = 'php-sensor-subscriber';
//file_put_contents("dump.dat","[");
$mqtt = new \PhpMqtt\Client\MqttClient($server, $port, $clientId);
$mqtt->connect();
$i=0;
$messages="";
$mqtt->subscribe('srebrinb/sensor/outTemC', function ($topic, $message) {
    global $i,$messages;
    
    //$message=json_decode($message);
    //$message->tim=time();
    $today = date("Y-m-d H:i:s"); 
   $message="\"".$today."\",".$message."\n";
   file_put_contents("dump.csv",$message,FILE_APPEND);
   $i++;
   if ($i%100==0){
    echo sprintf("%s\n",$message);
   }
    
}, 0);
$mqtt->loop(true);
$mqtt->disconnect();
//file_put_contents("dump.dat","]",FILE_APPEND);
?>
