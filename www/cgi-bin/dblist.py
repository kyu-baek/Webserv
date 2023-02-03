import os
from os import listdir
from os.path import isfile, join
from os import walk

upload_path = str(os.environ.get("UPLOAD_PATH"))

files = [f for f in listdir(upload_path) if isfile(join(upload_path, f))]

print ("<html>")
print ("<body>")
print("<div><a href='/home'>Go to home</a></div>")
if files:
	for item in files:
		item_path = "/database/" + item
		print('''
		<li> %s
		<a href='%s' download>
		<button>DOWNLOAD</button>
		</a></li>''' %(item, item_path))
else:
	print("<h2>NO FILES IN DB</h2>")

print("<a href='upload'> upload to db</a>")
print ("</body>")
print ("</html>")
