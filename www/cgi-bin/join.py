#!/usr/bin/env python3

import cgi, os
import cgitb
import sqlite3

upload_path = str(os.environ.get("UPLOAD_PATH"))

def register_user(username, email, password, id):
	conn = sqlite3.connect(upload_path + "/users.db")
	c = conn.cursor()
	c.execute("CREATE TABLE IF NOT EXISTS users (username TEXT, email TEXT, password TEXT, id TEXT)")
	c.execute("SELECT * FROM users WHERE username=? OR email=?", (username, email))
	if c.fetchone():
		print("Error: The username or email already exists.")
	else:
		c.execute("INSERT INTO users (username, email, password, id) VALUES (?,?,?,?)", (username, email, password, id))
		conn.commit()
		print("Registration successful!")
	conn.close()

def show_user_list():
	conn = sqlite3.connect(upload_path + "users.db")
	c = conn.cursor()
	c.execute("SELECT * FROM users")
	print("<h2>Username / Email</h2>")
	for row in c.fetchall():
		# print("<li>" + row[0] + " / " + row[1]+ "</li>")
		print(row)
		print("<br>")
	conn.close()

cgitb.enable()

form = cgi.FieldStorage()

username = form.getvalue("username")
email = form.getvalue("email")
password = form.getvalue("password")
confirm_password = form.getvalue("confirm_password")
id = "default_id"

print("<html>")
print("<head>")
print("<title>Join Results</title>")
print("</head>")
print("<body>")
print("<h1>Join Results</h1>")

if username and email and password and confirm_password:
	if password == confirm_password:
		register_user(username, email, password, id)
	else:
		print("<p>Error: Passwords do not match.</p>")
else:
	print("<p>Error: All fields are required.</p>")

print("<div>")
show_user_list()
print("</div>")

print("</body>")
print("</html>")
