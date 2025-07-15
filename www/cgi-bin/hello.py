#!/usr/bin/env python3

print("Content-Type: text/html")
print()
print("""
<style>
@keyframes pulse {
  0%   { transform: scale(1);   color: #3498db; }
  50%  { transform: scale(1.2); color: #e74c3c; }
  100% { transform: scale(1);   color: #3498db; }
}

#animated-text {
  font-size: 2rem;
  font-weight: bold;
  text-align: center;
  animation: pulse 2s infinite ease-in-out;
  transition: transform 0.3s;
}
</style>

<div id="animated-text">Hello, CGI is running successfully!</div>
""")
