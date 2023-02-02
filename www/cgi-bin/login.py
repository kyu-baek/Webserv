#!/usr/bin/env python3

import cgi, sqlite3,os
import cgitb

cgitb.enable()

upload_path = str(os.environ.get("UPLOAD_PATH"))

def validate(username, password):
	conn = sqlite3.connect(upload_path + "users.db")
	c = conn.cursor()
	c.execute("SELECT * FROM users WHERE username=? AND password=?", (username, password))
	user = c.fetchone()
	if user:
		conn.close()
		return True
	else:
		conn.close()
		return False

def update_id(id, username):
	islogin = "true"
	conn = sqlite3.connect(upload_path + "users.db")
	cursor = conn.cursor()
	cursor.execute("UPDATE users SET id = ? WHERE username = ?", (id, username))
	cursor.execute("UPDATE users SET islogin = ? WHERE username = ?", (islogin, username))
	conn.commit()
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

def is_any_login():
	conn = sqlite3.connect(upload_path + "users.db")
	c = conn.cursor()
	c.execute("SELECT islogin FROM users")
	islogins = c.fetchall()
	for item in islogins:
		if item[0] == "true":
			return True
	return False

def check_login(username):
	conn = sqlite3.connect(upload_path + "users.db")
	c = conn.cursor()
	c.execute("SELECT islogin FROM users WHERE username = ?", (username,))
	result = c.fetchone()
	if result[0] == "true":
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
# print(cookie)
id = cookie.split("=")[1]
if is_any_login() == True:
	print("<h1>Log out first!</h1>")

else:
	if username and password:
		# Validate the username and password here.
		result = validate(username, password)
		# Replace this with your own authentication logic.
		if result == True:
			# check_login(username)
			if check_login(username) == True:
				print("<h1>You already logged in!</h1>")
			else:
				print("<h1>Login successful!</h1>")
				update_id(id, username)
				show_user_list()
		else:
			print("<h1>Login failed!</h1>")
			print("<h2>Wrong username or password</h2>")
	else:
		print("<p>Error: Both username and password are required.</p>")

print("</body>")
print("</html>")
