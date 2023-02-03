#!/usr/bin/python3

import cgi, sqlite3,os
import cgitb

cgitb.enable()

upload_path = str(os.environ.get("UPLOAD_PATH"))
cookie = str(os.environ.get("HTTP_COOKIE"))
id = cookie.split("=")[1]

def logout():
	conn = sqlite3.connect(upload_path + "users.db")
	c = conn.cursor()
	islogin = "false"
	c.execute("UPDATE users SET islogin = ?", (islogin,))
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

print("<html>")
print("<body>")

logout()
print("<h1>You logged out successfully</h1>")
show_user_list()

print("</body>")
print("</html>")
