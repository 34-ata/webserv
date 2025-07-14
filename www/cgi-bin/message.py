#!/usr/bin/env python3

import cgi
import html

print("Content-Type: text/html\n")

form = cgi.FieldStorage()
name = html.escape(form.getfirst("name", "Anonymous"))
msg  = html.escape(form.getfirst("msg", ""))

print(f"""
<!DOCTYPE html>
<html>
<head><title>Message Received</title></head>
<body>
  <h1>Thank You, {name}!</h1>
  <p>Your message:</p>
  <blockquote>{msg}</blockquote>
  <a href="/contact.html">Go back</a>
</body>
</html>
""")
