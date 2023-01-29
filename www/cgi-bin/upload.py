#!/usr/bin/python3
import sys, os, base64

origin = sys.stdin.read()
content = origin.split("\r\n")

# print(content)

print("<html>")
print("<body>")
print("<div><a href=\"/home\">Go to index</a></div>")
# print(content)
# print(origin)
boundary = content[0]
content_disposition = content[1]
content_type = content[2]


print("<div>")
print("boundary : ", boundary)
print("</div>")

print("<div>")
print(content_disposition)
print("</div>")

print("<div>")
print(content_type)
print("</div>")

# print filename
filename = content[1].split(";")[2].split("=")[1].strip('"')
print("<h2>")
print("filename : " + filename)
print("</h2>")

idx1 = origin.find(content_type)
start_idx = idx1 + len(content_type) + 2 + 2
end_idx = origin.rfind(boundary) - 2

# print(origin)
# print(start_idx)
# print(end_idx)
bfile_content = origin[start_idx:end_idx]
# print(bfile_content)

# # save the file
file_content = content[4]


upload_path = str(os.environ.get("UPLOAD_PATH"))
# upload_path = "/Users/jinhyeok/Desktop/42seoul/git_webserver/cpp_webserver/05_refact/database/"
print("<h4>")
print(upload_path)
print("</h4>")
print("<h4>")
# print(file_content)

print("</h4>")
print("<div>")

if (content_type.split(': ')[1] != "image/jpeg" and content_type.split(': ')[1] != "image/png"):
	upload_file = open(upload_path + filename, "w", encoding='utf-8')
	upload_file.write(bfile_content)
	upload_file.close()
	print("<h1>")
	print("FILE UPLOADED!!")
	print("</h1>")
	print(bfile_content)
else:
	print(bfile_content)
	f = open(upload_path + filename, "wb")
	res = f.write(bfile_content)
	if (res == -1):
		print("FAILED")
	print("<h1>")
	print("FILE UPLOADED!!")
	print("</h1>")
	# print(bfile_content)

print("</div>")
print("</body>")
print("</html>")
# "r" - Read - Default value. Opens a file for reading, error if the file does not exist
# "a" - Append - Opens a file for appending, creates the file if it does not exist
# "w" - Write - Opens a file for writing, creates the file if it does not exist
# "x" - Create - Creates the specified file, returns an error if the file exist
# "t" - Text - Default value. Text mode
# "b" - Binary - Binary mode (e.g. images)
