<?php

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Retrieve form data
    $name = isset($_POST['name']) ? $_POST['name'] : '';
    $email = isset($_POST['email']) ? $_POST['email'] : '';
    $message = isset($_POST['message']) ? $_POST['message'] : '';

    // Process the form data (you can perform any desired logic here)

    // Display a confirmation message
    echo "<h1>Thank you, $name ($email)!</h1>";
    echo "<p>We appreciate your message:</p>";
    echo "<blockquote>$message</blockquote>";

    // Optionally, you can send an email, store the data in a database, etc.

} else {
    // Method not allowed
    header('HTTP/1.1 405 Method Not Allowed');
    echo json_encode(['error' => 'Method Not Allowed']);
}

?>
