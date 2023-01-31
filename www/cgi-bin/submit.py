#!/usr/bin/python

# Import modules for CGI handling
import cgi, cgitb , sys, time

# Create instance of FieldStorage
form = cgi.FieldStorage()
# print("FORM", form)

# Get data from fields

first_name = form.getvalue('first_name')
last_name = form.getvalue('last_name')

# Make response data

print("hello\n", file = sys.stderr)
# time.sleep(30)

# print ("Content-type:text/html\r\n\r\n")
print ("<html>")
print ("<head>")
print ("<title>Test Form (POST) Result</title>")
print ("<style>")
print ("body {font-family: 'Fredoka One';}")
print ("</style>")
print ("</head>")
print ("<body>")
print ("<h2> POST Result </h2>")
print ("<h3>Your First name is [%s] Last name is [%s]</h3>" %(first_name, last_name))
print ("</body>")
print ("</html>")
