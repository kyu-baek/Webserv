#!/usr/bin/python3

import cgi, cgitb , sys, sqlite3
from distutils.command.upload import upload
import sys, os



origin = sys.stdin.read()
upload_path = str(os.environ.get("UPLOAD_PATH"))
# # Create instance of FieldStorage
# form = cgi.FieldStorage()

cookie = str(os.environ.get("HTTP_COOKIE"))
print(cookie)
id = cookie.split("=")[1]


def make_logout(id):
	conn = sqlite3.connect(upload_path +"users.db")
	c = conn.cursor()
	c.execute("SELECT islogin FROM users WHERE id=?", (id,))
	result = c.fetchone()
	if result[0] == "true":
		islogin = "false"
		c.execute("UPDATE users SET islogin = ? WHERE id = ?", (islogin, id))
	conn.commit()
	conn.close()

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
print("<div><a href='/home'>Go to home</a></div>")
print(filename + " has been deleted")
print("<h1>")
print("</h1>")
print("</body>")
print("</html>")
