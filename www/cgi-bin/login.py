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
		print("<p>no ueser exists</p>")
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

def logout():
	conn = sqlite3.connect(upload_path + "users.db")
	c = conn.cursor()
	islogin = "false"
	c.execute("UPDATE users SET islogin = ?", (islogin,))
	conn.commit()
	conn.close()

def is_same_sessionid(id, username):
	conn = sqlite3.connect(upload_path + "users.db")
	c = conn.cursor()
	c.execute("SELECT id FROM users WHERE username = ?", (username,))
	saved_id = c.fetchone()
	# print(saved_id)
	if saved_id and saved_id[0] != id:
		print("defferent!")
		return False
	return True

def is_defaultid(id, username):
	conn = sqlite3.connect(upload_path + "users.db")
	c = conn.cursor()
	c.execute("SELECT id FROM users WHERE username = ?", (username,))
	saved_id = c.fetchone()
	if saved_id and saved_id[0] == "default_id":
		return True
	return False

form = cgi.FieldStorage()

username = form.getvalue("username")
password = form.getvalue("password")

print("<html>")
print("<body>")
print("<h1>Login Results</h1>")
cookie = str(os.environ.get("HTTP_COOKIE"))

conn = sqlite3.connect(upload_path + "/users.db")
c = conn.cursor()
c.execute("CREATE TABLE IF NOT EXISTS users (username TEXT, email TEXT, password TEXT, id TEXT, islogin TEXT)")
conn.close()

id = cookie.split("=")[1]



if is_same_sessionid(id, username) == False:
	if is_defaultid == False:
		print("<h2>Different session id</h2>")
		logout()
	else:
		print("<h1>Login successful!</h1>")
		update_id(id, username)
		show_user_list()
elif is_any_login() == True:
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

	else:
		print("<p>Error: Both username and password are required.</p>")

print("</body>")
print("</html>")
