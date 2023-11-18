#!/bin/bash

URL="https://localhost:8080/"
# URL of the web page you want to fetch
while true
do

# Using curl to retrieve the web page content and save it to a file
	curl -o output.html "$URL"

# Check if the curl command was successful
	if [ $? -eq 0 ]; then
		echo "Web page downloaded successfully."
	else
		echo "Failed to download the web page."
	fi
done