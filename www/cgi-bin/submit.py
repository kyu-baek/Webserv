#!/usr/bin/python3
import sys, urllib.parse

# Read the form data from the stdin stream
post_data = sys.stdin.read()
print(post_data)
# convert to string and strip the trailing null characters
post_data = post_data.rstrip("\x00")

# parse the form data
form_data = urllib.parse.parse_qs(post_data)

print("<html>")
print("<body>")
print("<div><a href=\"/home\">Go to index</a></div>")
# Print the contents of the POST request
print("<div>POST Data:</div>")
print("<div>")
if "first_name" in form_data:
	first_name = form_data["first_name"][0]
	print("first name : %s" % (first_name ))
print("</div>")
print("<div>")
if "last_name" in form_data:
    last_name = form_data["last_name"][0]
    print("last name : %s" % (last_name))
print("</div>")
if "first_name" not in form_data and "last_name" not in form_data:
	print("name not found")

print("</body>")
print("</html>")
