import os
from os import listdir
from os.path import isfile, join
from os import walk

# upload_path = "/Users/jinhyeok/Desktop/42webserv/database/"
upload_path = str(os.environ.get("UPLOAD_PATH"))

files = [f for f in listdir(upload_path) if isfile(join(upload_path, f))]

# print(onlyfiles)

print ("<html>")
print ("<body>")
for item in files:
	print(item)
	item_path = "/database/" + item
	print('''
	<div>
	<a href='%s' download>
 	<button value="download" />
	<img src='%s' alt='Download'>
	</a></div>''' %(item_path, item_path))

print ("</body>")
print ("</html>")
