#!/usr/bin/env python3

import cgi
import os
import sys
import urllib.parse

print("Content-Type: text/plain")
print()

try:
    # Parse form data
    form = cgi.FieldStorage()
    
    filename = form.getfirst("filename", "")
    content = form.getfirst("content", "")
    
    if not filename or not content:
        print("ERROR: Missing filename or content")
        sys.exit(1)
    
    # Ensure filename is safe
    filename = os.path.basename(filename)
    if not filename.endswith('.txt'):
        filename += '.txt'
    
    # Create file in uploads directory
    uploads_dir = "/home/fata/webserv/www/uploads"
    file_path = os.path.join(uploads_dir, filename)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"SUCCESS: File '{filename}' created with {len(content)} bytes")
    
except Exception as e:
    print(f"ERROR: {str(e)}")
