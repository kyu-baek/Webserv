#!/usr/bin/python3
from distutils.command.upload import upload
import sys, os

content = sys.stdin.read()
content = content.split("\r\n")

# print(content)

print("<html>")
print("<body>")
print("<div><a href=\"/home\">Go to index</a></div>")

content_type = content[2]

# print filename
filename = content[1].split(";")[2].split("=")[1].strip('"')
print("<h2>")
print("filename : " + filename)
print("</h2>")

# # save the file
file_content = content[4]
upload_path = str(os.environ.get("UPLOAD_PATH"))
print("<h4>")
print(upload_path)
print("</h4>")
print("<div>")
if (content_type.split(': ')[1] == "text/plain"):
    upload_file = open(upload_path + filename, "wt")
    upload_file.write(file_content)
    upload_file.close()
    print("<h1>")
    print("FILE UPLOADED!!")
    print("</h1>")
else:
    print("<h1>")
    print("ONLY TEXT UPLOAD")
    print("</h1>")
print("</div>")
# "r" - Read - Default value. Opens a file for reading, error if the file does not exist
# "a" - Append - Opens a file for appending, creates the file if it does not exist
# "w" - Write - Opens a file for writing, creates the file if it does not exist
# "x" - Create - Creates the specified file, returns an error if the file exist
# "t" - Text - Default value. Text mode
# "b" - Binary - Binary mode (e.g. images)


# print contents
file_contents = content[4].split('\n')
for line in file_contents:
    print("<div>")
    print("%s" %line)
    print("</div>")
print("</body>")
print("</html>")
