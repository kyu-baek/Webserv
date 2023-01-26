#!/usr/bin/python3

import cgi, cgitb , sys, time

# Create instance of FieldStorage
form = cgi.FieldStorage()
# print("FORM", form)
# # Get data from fields

first_name = form.getvalue('first_name')
last_name = form.getvalue('last_name')

print ("<html>")
print ("<head>")
print ("</head>")
print ("<body>")
print ("<h2> POST Result </h2>")
print ("<h3>%s's last name is %s</h3>" %(first_name, last_name))
print ("</body>")
print ("</html>")
