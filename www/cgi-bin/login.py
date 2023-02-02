#!/usr/bin/env python3

import cgi, sqlite3,os
import cgitb

cgitb.enable()

upload_path = str(os.environ.get("UPLOAD_PATH"))

def validate(username, password):
	conn = sqlite3.connect(upload_path +"users.db")
	c = conn.cursor()
	c.execute("SELECT * FROM users WHERE username=? AND password=?", (username, password))
	user = c.fetchone()
	if user:
		return True
	else:
		return False

form = cgi.FieldStorage()

username = form.getvalue("username")
password = form.getvalue("password")

print("<html>")
print("<body>")
print("<h1>Login Results</h1>")
cookie = str(os.environ.get("HTTP_COOKIE"))
print(cookie)
if username and password:
	# Validate the username and password here.
	result = validate(username, password)
	# Replace this with your own authentication logic.
	if result == True:
		print("<p>Login successful!</p>")
	else:
		print("<p>Login failed!</p>")
else:
	print("<p>Error: Both username and password are required.</p>")

print("</body>")
print("</html>")
