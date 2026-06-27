#!/bin/bash
# Simple test script that outputs environment variables
# This is what a real PHP/Python/Perl script would do

echo "Content-Type: text/html"
echo ""
echo "<html><body>"
echo "<h1>CGI Test Output</h1>"
echo "<h2>Environment Variables:</h2>"
echo "<pre>"
echo "REQUEST_METHOD: $REQUEST_METHOD"
echo "QUERY_STRING: $QUERY_STRING"
echo "CONTENT_LENGTH: $CONTENT_LENGTH"
echo "SCRIPT_FILENAME: $SCRIPT_FILENAME"
echo "HTTP_HOST: $HTTP_HOST"
echo "</pre>"
echo "<h2>stdin data:</h2>"
echo "<pre>"
cat
echo "</pre>"
echo "</body></html>"