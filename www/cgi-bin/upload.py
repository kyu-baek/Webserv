#!/usr/bin/python

# Import modules for CGI handling 
import cgi, cgitb , sys, time

# Create instance of FieldStorage 
form = cgi.FieldStorage() 
# print("FORM", form)

# Get data from fields

filename = form.getvalue('filename')

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
print ("<h2>Test Form (POST) ✨ Result ✨</h2>")
print ("<h3>Uploaded file : [%s] </h3>" %(filename))
print ("</body>")
print ("</html>")