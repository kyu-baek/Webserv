#!/usr/bin/python3

import cgi, cgitb , sys
import sys, os


content = sys.stdin.read()

# # Create instance of FieldStorage
# form = cgi.FieldStorage()
print("<html>")
print("<body>")
print(content)
print("hello")
print("<h1>")
print("delete")
print("</h1>")
print("</body>")
print("</html>")
