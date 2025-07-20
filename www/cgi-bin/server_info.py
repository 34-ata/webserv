#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import time

print("Content-Type: text/html; charset=utf-8")
print()

print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WebServ Server Info</title>
    <style>
        body {{ 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #ffffff;
            margin: 0;
            padding: 20px;
            min-height: 100vh;
        }}
        .container {{
            max-width: 800px;
            margin: 0 auto;
            background: rgba(0, 0, 0, 0.7);
            border-radius: 15px;
            padding: 30px;
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.3);
        }}
        .terminal {{ 
            background: #1a1a1a; 
            border: 2px solid #333; 
            border-radius: 10px; 
            padding: 20px; 
            margin: 20px 0; 
            position: relative;
        }}
        .terminal:before {{
            content: "● ● ●";
            position: absolute;
            top: 10px;
            left: 15px;
            color: #ff6b6b;
            font-size: 12px;
        }}
        .terminal-header {{
            margin-top: 20px;
            color: #4ecdc4;
            font-weight: bold;
            border-bottom: 1px solid #333;
            padding-bottom: 10px;
            font-size: 18px;
        }}
        .info-line {{ 
            margin: 8px 0; 
            padding-left: 20px;
            font-family: 'Courier New', monospace;
        }}
        .label {{ 
            color: #ffd93d; 
            font-weight: bold; 
        }}
        .value {{ 
            color: #6bcf7f; 
        }}
        .status-ok {{ 
            color: #6bcf7f; 
        }}
        .back-link {{
            display: inline-block;
            margin-top: 30px;
            padding: 12px 25px;
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
            text-decoration: none;
            border-radius: 8px;
            transition: transform 0.3s ease;
            font-weight: bold;
        }}
        .back-link:hover {{
            transform: translateY(-2px);
        }}
        h1 {{
            text-align: center; 
            margin-bottom: 30px;
            font-size: 28px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
        }}
        .footer {{
            text-align: center; 
            margin-top: 30px; 
            padding: 20px; 
            background: rgba(255,255,255,0.1); 
            border-radius: 10px;
            font-size: 12px;
            color: #ccc;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>WebServ System Information</h1>
        
        <div class="terminal">
            <div class="terminal-header">System Status</div>
            <div class="info-line"><span class="label">Server Time:</span> <span class="value">{time.strftime('%Y-%m-%d %H:%M:%S')}</span></div>
            <div class="info-line"><span class="label">Server Software:</span> <span class="value">WebServ/1.0</span></div>
            <div class="info-line"><span class="label">Server Port:</span> <span class="value">{os.environ.get('SERVER_PORT', '8080')}</span></div>
            <div class="info-line"><span class="label">Document Root:</span> <span class="value">{os.environ.get('DOCUMENT_ROOT', '/www')}</span></div>
            <div class="info-line"><span class="label">CGI Version:</span> <span class="value">CGI/1.1</span></div>
            <div class="info-line"><span class="label">Python Version:</span> <span class="value">{sys.version.split()[0]}</span></div>
        </div>

        <div class="terminal">
            <div class="terminal-header">Request Information</div>
            <div class="info-line"><span class="label">Request Method:</span> <span class="value">{os.environ.get('REQUEST_METHOD', 'GET')}</span></div>
            <div class="info-line"><span class="label">Request URI:</span> <span class="value">{os.environ.get('REQUEST_URI', '/cgi-bin/server_info.py')}</span></div>
            <div class="info-line"><span class="label">Query String:</span> <span class="value">{os.environ.get('QUERY_STRING', '(none)')}</span></div>
            <div class="info-line"><span class="label">User Agent:</span> <span class="value">{(os.environ.get('HTTP_USER_AGENT', 'WebServ Client')[:50] + '...') if len(os.environ.get('HTTP_USER_AGENT', '')) > 50 else os.environ.get('HTTP_USER_AGENT', 'WebServ Client')}</span></div>
            <div class="info-line"><span class="label">Remote Address:</span> <span class="value">{os.environ.get('REMOTE_ADDR', '127.0.0.1')}</span></div>
        </div>

        <div class="terminal">
            <div class="terminal-header">WebServ Features Status</div>
            <div class="info-line"><span class="status-ok">✓ GET Method: Active</span></div>
            <div class="info-line"><span class="status-ok">✓ POST Method: Active</span></div>
            <div class="info-line"><span class="status-ok">✓ DELETE Method: Active</span></div>
            <div class="info-line"><span class="status-ok">✓ CGI Support: Active</span></div>
            <div class="info-line"><span class="status-ok">✓ Static Files: Active</span></div>
            <div class="info-line"><span class="status-ok">✓ Error Pages: Active</span></div>
            <div class="info-line"><span class="status-ok">✓ Autoindex: Active (uploads dir)</span></div>
        </div>

        <div class="terminal">
            <div class="terminal-header">Path Information</div>
            <div class="info-line"><span class="label">Script Path:</span> <span class="value">{os.path.abspath(__file__)}</span></div>
            <div class="info-line"><span class="label">Working Directory:</span> <span class="value">{os.getcwd()}</span></div>
            <div class="info-line"><span class="label">Document Root:</span> <span class="value">{os.environ.get('DOCUMENT_ROOT', 'Not set')}</span></div>
            <div class="info-line"><span class="label">Script Name:</span> <span class="value">{os.environ.get('SCRIPT_NAME', 'Not set')}</span></div>
        </div>

        <div class="terminal">
            <div class="terminal-header">Environment Variables</div>
            {''.join([f'<div class="info-line"><span class="label">{key}:</span> <span class="value">{value}</span></div>' for key, value in sorted(os.environ.items()) if 'PATH' in key or 'DIR' in key or 'ROOT' in key])}
        </div>

        <div class="footer">
            Generated by: WebServ CGI Handler<br>
            Script: {os.environ.get('SCRIPT_NAME', './www/cgi-bin/server_info.py')}
        </div>

        <div style="text-align: center;">
            <a href="/" class="back-link">← Back to Home</a>
        </div>
    </div>
</body>
</html>
""")
