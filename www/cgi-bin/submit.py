#!/usr/bin/python

# Import modules for CGI handling 
import cgi, cgitb , sys, time

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
intra_id = form.getvalue('first_name')
coalition = form.getvalue('last_name')

# Make response data

print("hello\n", file = sys.stderr)
time.sleep(30)

print ("Content-type:text/html\r\n\r\n")
print ("<html>")
print ("<head>")
print ("<title>Test Form (POST) Result</title>")
print ("<style>")
print ("body {font-family: 'Fredoka One';}")
print ("</style>")
print ("</head>")
print ("<body>")
print ("<h2>Test Form (POST) ✨ Result ✨</h2>")
print ("<h3>%s's coalition is %s</h3>" %(intra_id, coalition))
print ("</body>")
print ("</html>")