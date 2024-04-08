import os
import user
import cgi
import bcrypt

print()
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Form Submission Result</title>")
print("<style>h1{ text-align: center; }pre{ text-align: center; }</style>")
print("</head>")
print("<body>")

username: str
password: str
firstname: str

if os.getenv("QUERY_STRING"):
	form = cgi.FieldStorage()
	if "username" in form and "password" in form:
		username = form["username"].value
		password = form["password"].value
		firstname = form["firstname"].value

else:
	inputstring = input()
	username, password, firstname = inputstring.split("&")
	_, username = username.split("=")
	_, password = password.split("=")
	_, firstname = firstname.split("=")

print("<h1>Form Submission Result</h1>")
	# newUser = user.User("", username, password, firstname) 
newUser = user.User("", "", "", "")

if (newUser.getUserByUsername(username) == False):
	try:
		hashedpassword = bcrypt.hashpw(password.encode("utf-8"), bcrypt.gensalt())
	except:
		print(Exception)
		exit()
	newUser = user.User("", username, hashedpassword.decode("utf-8"), firstname)
	newUser.addUser()
	print("<pre>thanks for registering: " + firstname + "</pre>")
	print("<p><a href='login.py'>login</a></p>")
else:
	print("<pre>member already exists: " + firstname + " " + newUser.sessionID + "</pre>")
	print("<p><a href='login.py'>login</a></p>")

print("</body>")
print("</html>")
