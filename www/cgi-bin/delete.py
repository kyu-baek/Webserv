#!/usr/bin/python3

import cgi, cgitb , sys
from distutils.command.upload import upload
import sys, os


origin = sys.stdin.read()
upload_path = str(os.environ.get("UPLOAD_PATH"))
# # Create instance of FieldStorage
# form = cgi.FieldStorage()


print("<html>")
print("<body>")
# print(origin)
content = origin.split("\r\n")
# print(content)
filename = content[3]
# print(filename)
# print(upload_path)
target = upload_path + filename
os.unlink(target)
print(filename + " has been deleted")
print("<h1>")
print("</h1>")
print("</body>")
print("</html>")

