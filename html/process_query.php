<?php

if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    // Retrieve the query parameter from the GET request
    $query = isset($_GET['query']) ? $_GET['query'] : '';

    // Process the query (you can perform any desired logic here)
    $result = "You entered: $query";

    // Display the result
    echo "<p>$result</p>";
} else {
    // Method not allowed
    header('HTTP/1.1 405 Method Not Allowed');
    echo json_encode(['error' => 'Method Not Allowed']);
}
