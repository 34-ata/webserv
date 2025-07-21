#!/usr/bin/env python3
import sys
import os
import cgi

content_length = int(os.environ.get('CONTENT_LENGTH', 0))
if content_length > 0:
    post_data = sys.stdin.read(content_length)
else:
    post_data = ""

print("Content-Type: text/html")
print()

print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>POST Test Result - WebServ</title>
    <style>
        body {{ font-family: monospace; margin: 20px; }}
        .header {{ background: #e7f3ff; padding: 15px; border-radius: 5px; margin-bottom: 20px; }}
        .data-section {{ background: #f8f9fa; border: 1px solid #dee2e6; padding: 15px; margin: 10px 0; }}
        .raw-data {{ background: #f1f1f1; padding: 10px; white-space: pre-wrap; font-family: monospace; }}
        .success {{ color: #28a745; }}
        .info {{ color: #17a2b8; }}
    </style>
</head>
<body>
    <div class="header">
        <h1 class="success">✅ POST Request Processed Successfully</h1>
        <p class="info">WebServ CGI POST Handler - Detailed Analysis</p>
    </div>

    <div class="data-section">
        <h2>📊 Request Environment</h2>
        <p><strong>Method:</strong> {os.environ.get('REQUEST_METHOD', 'Unknown')}</p>
        <p><strong>Content-Type:</strong> {os.environ.get('CONTENT_TYPE', 'Unknown')}</p>
        <p><strong>Content-Length:</strong> {content_length} bytes</p>
        <p><strong>Query String:</strong> {os.environ.get('QUERY_STRING', '(none)')}</p>
        <p><strong>Server Software:</strong> {os.environ.get('SERVER_SOFTWARE', 'WebServ/1.0')}</p>
        <p><strong>User Agent:</strong> {os.environ.get('HTTP_USER_AGENT', 'Unknown')}</p>
    </div>

    <div class="data-section">
        <h2>📥 Raw POST Data ({len(post_data)} chars)</h2>
        <div class="raw-data">{post_data if post_data else '(No data received)'}</div>
    </div>

    <div class="data-section">
        <h2>🔍 Parsed Form Data</h2>
""")

try:
    form = cgi.FieldStorage()
    if form.list:
        print("<table border='1' style='border-collapse: collapse; width: 100%;'>")
        print("<tr><th>Field Name</th><th>Value</th></tr>")
        for field in form.list:
            field_name = field.name if field.name else "(unnamed)"
            field_value = field.value if field.value else "(empty)"
            print(f"<tr><td>{field_name}</td><td>{field_value}</td></tr>")
        print("</table>")
    else:
        print("<p>No form fields parsed (might be raw data)</p>")
except Exception as e:
    print(f"<p style='color: red;'>Error parsing form data: {e}</p>")

print(f"""
    </div>

    <div class="data-section">
        <h2>🌍 All Environment Variables</h2>
        <div style="max-height: 200px; overflow-y: auto;">
            <table border="1" style="border-collapse: collapse; width: 100%; font-size: 12px;">
                <tr><th>Variable</th><th>Value</th></tr>
""")

for key, value in sorted(os.environ.items()):
    print(f"<tr><td>{key}</td><td>{value}</td></tr>")

print("""
            </table>
        </div>
    </div>

    <p><a href="/contact.html" style="background: #007bff; color: white; padding: 10px 15px; text-decoration: none; border-radius: 5px;">🔙 Back to Forms</a></p>
</body>
</html>
""")
