#GUI code... 
#Importing tkinter libraries for UI
from tkinter import *
from tkinter import filedialog
import subprocess
import ttkbootstrap as tb

root = tb.Window(themename= "vapor")    #Setting a Theme for the application (vapor seemed cool)

root.title("NotNORD")
root.geometry("1440x900")

file_path = ""          #Just Initializing a filepath
upload_type = "file"    #Setting a default upload type

#Function Definitions

#Function to open and display the file path
def file_dialog():
    global file_path
    if upload_type == "file":
        file_path = filedialog.askopenfilename()
    else:
        file_path = filedialog.askdirectory()
    if file_path:
        file_label.config(text = "Chosen Path : " + file_path)
    else:
        file_label.config(text = "No Path Selected")

#Function to
def execute_engine(file_path):
    if file_path:
        # Execute engine.exe and capture output
        file_label.config(text=file_path)
        result = subprocess.run(["C:/Users/ssric/Documents/MalDev/Antivirus/engine/x64/Debug/engine.exe", file_path], stdout=subprocess.PIPE)
        output_text.delete(1.0, END)  # Clear previous output
        output_text.insert(END, result.stdout.decode())  # Insert captured output
        output_text.see(END)
    else: 
        file_label.config(text = "No Path Selected")

#Function to toggle between the Upload buttons
def toggle_upload_type():
    global upload_type
    if upload_type == "file":
        upload_type = "directory"
        toggle_button.config(text = "Switch to File Upload")
    else:
        upload_type = "file"
        toggle_button.config(text = "Switch to Directory Upload")


setting_label = tb.Label(text = "NotAvast", font = ("Consolas",70), bootstyle = "default")
setting_label.pack(pady = 10)

#Toggling between file and directory to upload
toggle_button = tb.Button(text = "Switch to Directory Upload", bootstyle = "secondary", command = toggle_upload_type)
toggle_button.pack(pady = 10)

#Label to the file path chosen
file_label = tb.Label(text = "", font = ("Consolas", 15), bootstyle = "default")
file_label.pack(pady = 10)

#Loading intial Image
image = PhotoImage(file = "images/upload_image.png")

#Creating a Label for the Image
image_button = tb.Label(image = image)
image_button.pack(pady = 10)

#Binding the Image Button to the dialog function
image_button.bind("<Button-5>", lambda event : file_dialog())

#Use the lambda function to pass file path to execute_engine function when the button is clicked
click_button = tb.Button(text = "Upload", bootstyle = "warning, outline", command = lambda: execute_engine(file_path))
click_button.config(padding = "40 15")
click_button.pack(pady = 20)

#Changing main label to scan results
setting_label = tb.Label(text = "Scan Results", font = ("Consolas" , 45), bootstyle = "success")
setting_label.pack(pady = 20)

#Text Widget to display the output
output_text = Text(root, width = 200, height = 30, wrap = "word", font = ("Helvetica" ,14))
output_text.pack(pady = 10)

root.mainloop()
