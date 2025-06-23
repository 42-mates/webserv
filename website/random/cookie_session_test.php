<?php
session_start();
if (!isset($_SESSION['count'])) $_SESSION['count'] = 0;
$_SESSION['count']++;
echo "Content-Type: text/html\r\n";
echo "Set-Cookie: testcookie=value; Path=/\r\n\r\n";
echo "<h1>Count: " . $_SESSION['count'] . "</h1>";
?>