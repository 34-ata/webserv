#!/usr/bin/env python3
import os
import sys
import urllib.parse

print("Content-Type: text/html\r\n\r\n")

try:
    # Read POST data
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    post_data = sys.stdin.read(content_length)
    
    # Parse form data
    params = urllib.parse.parse_qs(post_data)
    filename = params.get('filename', [''])[0]
    content = params.get('content', [''])[0]
    
    if not filename or not content:
        print("ERROR: Missing filename or content")
        sys.exit(1)
    
    # Debug information
    print(f"DEBUG: Current working directory: {os.getcwd()}")
    print(f"DEBUG: Script location: {os.path.abspath(__file__)}")
    
    # Use working directory relative path - more portable
    uploads_dir = 'www/uploads'
    os.makedirs(uploads_dir, exist_ok=True)
    
    # Write file
    file_path = os.path.join(uploads_dir, filename)
    print(f"DEBUG: Creating file at: {os.path.abspath(file_path)}")
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"SUCCESS: File {filename} created successfully")
    
except Exception as e:
    print(f"ERROR: {str(e)}")
