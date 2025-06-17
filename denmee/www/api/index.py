#!/usr/bin/env python3

import sys
import os

print("Content-Type: text/html")
print()

print("<html><body>")
print("<h1>CGI Python Script</h1>")
print("<p>Request Method: {}</p>".format(os.environ.get("REQUEST_METHOD", "UNKNOWN")))

if os.environ.get("REQUEST_METHOD") == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    post_data = sys.stdin.read(content_length) if content_length > 0 else ""
    print("<p>Posted Data: {}</p>".format(post_data))

print("</body></html>")
