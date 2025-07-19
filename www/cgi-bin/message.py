#!/usr/bin/env python3

import cgi
import html
import sys
import os

print("Content-Type: text/html")
print()

# Get form data
form = cgi.FieldStorage()

# Extract all form fields
name = html.escape(form.getfirst("name", "Anonymous"))
email = html.escape(form.getfirst("email", ""))
subject = html.escape(form.getfirst("subject", ""))
message = html.escape(form.getfirst("message", ""))

# Also handle the simple message field for backward compatibility
msg = html.escape(form.getfirst("msg", ""))

print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>Message Received - WebServ</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .success {{ background: #d4edda; border: 1px solid #c3e6cb; padding: 15px; border-radius: 5px; }}
        .info {{ background: #f8f9fa; border: 1px solid #dee2e6; padding: 10px; margin: 10px 0; }}
        .back-btn {{ background: #007bff; color: white; padding: 10px 15px; text-decoration: none; border-radius: 5px; }}
    </style>
</head>
<body>
    <div class="success">
        <h1>✅ Message Received Successfully!</h1>
        <p><strong>Thank you, {name}!</strong></p>
    </div>
    
    <div class="info">
        <h2>📧 Message Details:</h2>
        <p><strong>Name:</strong> {name}</p>
        {f'<p><strong>Email:</strong> {email}</p>' if email else ''}
        {f'<p><strong>Subject:</strong> {subject}</p>' if subject else ''}
        <p><strong>Message:</strong></p>
        <blockquote>{message if message else msg}</blockquote>
    </div>
    
    <div class="info">
        <h2>🔧 Technical Info:</h2>
        <p><strong>Method:</strong> {os.environ.get('REQUEST_METHOD', 'Unknown')}</p>
        <p><strong>Content Type:</strong> {os.environ.get('CONTENT_TYPE', 'Unknown')}</p>
        <p><strong>Content Length:</strong> {os.environ.get('CONTENT_LENGTH', 'Unknown')}</p>
        <p><strong>User Agent:</strong> {os.environ.get('HTTP_USER_AGENT', 'Unknown')}</p>
    </div>
    
    <p><a href="/contact.html" class="back-btn">🔙 Go Back</a></p>
</body>
</html>
""")
