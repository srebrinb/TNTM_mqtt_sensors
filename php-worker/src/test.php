<?php
require __DIR__ . '/../vendor/autoload.php';
$server   = '192.168.1.10';
$port     = 1883;
$clientId = 'php-test-subscriber';
file_put_contents("dump.dat","[");
$mqtt = new \PhpMqtt\Client\MqttClient($server, $port, $clientId);
$mqtt->connect();
$i=0;
$messages="";
$mqtt->subscribe('outTemC', function ($topic, $message) {
    global $i,$messages;
    echo sprintf("Received message on topic [%s]: %s\n", $topic, $message);
    $message=json_decode($message);
    $message->tim=time();
   $messages.=json_encode($message).",";
   $i++;
   if ($i%10==0){
    file_put_contents("dump.dat",$messages."\n",FILE_APPEND);
    $messages="";
   }
    
}, 0);
$mqtt->loop(true);
$mqtt->disconnect();
file_put_contents("dump.dat","]",FILE_APPEND);
?>
