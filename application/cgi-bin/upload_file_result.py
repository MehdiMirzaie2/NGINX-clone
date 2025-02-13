#!/usr/bin/env python
import cgi
import os
from io import BytesIO as IO

# Set target directory for uploads
application_directory = "application"
save_folder_name = "uploaded-files"

def html_print(key, val):
    print(f"<!-- key: {key}    val: {val}-->")

def handle_get():
    print_page_with_content('Please go to <a href=uploadFile.py>this page</a> to upload a file')

def handle_post():
    form = cgi.FieldStorage(
        headers={"content-type": os.getenv("Content-Type"), "content-length": os.getenv("Content-Length")},
        environ={"REQUEST_METHOD": os.getenv("Method")}
    )
    item = form.value[0]
    field_name = item.name
    file_name = item.filename
    binary_data = item.value

    current_pwd = os.getcwd()
    upload_directory = os.path.join(os.getcwd(), application_directory, save_folder_name)
    if not os.path.exists(upload_directory):
        os.makedirs(upload_directory)

    file_path = os.path.join(upload_directory, os.path.basename(file_name))

    if os.path.exists(file_path):
        print_page_with_content("Sorry, file already exists.")
    else:
        # Try to upload file
        try:
            with open(file_path, "wb") as f:
                f.write(binary_data)
            print_page_with_content(f"The file {file_name} has been uploaded.")
        except IOError as e:
            print_page_with_content(f"Sorry there was an error uploading your file")

def print_page_with_content(content):
    print(f'''
<html>
    <body>
        <h1>File Upload</h1>
        <p>{content}</p>
        <p><a href="/cgi-bin/upload_file.py">Upload a new file</a></p>
        <p><a href="/cgi-bin/all_files.py">View all uploaded files</a></p>
    </body>
</html>
''')

if os.getenv("Method") == "POST":
    handle_post()
else:
    handle_get()

# #!/usr/bin/env python
# import cgi
# import os
# from io import BytesIO as IO
# import sys

# # Set target directory for uploads
# application_directory = "application"
# save_folder_name = "uploaded-files"

# def html_print(key, val):
#     print(f"<!-- key: {key}    val: {val}-->")

# def handle_get():
#     print_page_with_content('Please go to <a href=upload_file.py>this page</a> to upload a file')

# def handle_post():
#     form = cgi.FieldStorage(
#         headers={"content-type": os.getenv("Content-Type"), "content-length": os.getenv("Content-Length")},
#         environ={"METHOD": os.getenv("METHOD")}
#     )
#     item = form.value[0]
#     field_name = item.name
#     file_name = item.filename
#     binary_data = item.value
#     # binary_data = input()
#     # with open("../error.txt", "wb") as f:
#     #     f.write(binary_data)
#     #     f.write("my name is mehdi mirzaie")

#     current_pwd = os.getcwd()
#     upload_directory = os.path.join(os.getcwd(), application_directory, save_folder_name)
#     if not os.path.exists(upload_directory):
#         os.makedirs(upload_directory)

#     file_path = os.path.join(upload_directory, os.path.basename(file_name))

#     if os.path.exists(file_path):
#         print_page_with_content("Sorry, file already exists.")
#     else:
#         # Try to upload file
#         try:
#             with open(file_path, "wb") as f:
#                 f.write(binary_data)
#             print_page_with_content(f"The file {file_name} has been uploaded hahahhaahhhahhahahah.")
#         except IOError as e:
#             print_page_with_content(f"Sorry there was an error uploading your file")

# def print_page_with_content(content):
#     print(f'''
# <html>
#     <body>
#         <h1>File Upload</h1>
#         <p>{os.getenv("METHOD")}<p>
#         <p>{content}</p>
#         <p><a href="/cgi-bin/upload_file.py">Upload a new file</a></p>
#         <p><a href="/cgi-bin/all_files.py">View all uploaded files</a></p>
#     </body>
# </html>
# ''')

# if os.getenv("METHOD") == "POST":
#     handle_post()
# else:
#     handle_get()