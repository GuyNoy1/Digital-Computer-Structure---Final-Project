import tkinter as tk
from tkinter import filedialog
import os
import serial as serial
import threading
import pyautogui
import math

class GUI_CLASS:
    def __init__(self):
        self.root = tk.Tk()
        self.root.geometry("800x600")
        self.root.title("High-Class Themed Project")
        self.bg_color = "#FFFFFF"  # White background color
        self.btn_color = "#ADD8E6"  # Light blue button color
        self.btn_fg_color = "#000000"  # Black text on buttons
        self.font = ("Helvetica", 14)  # Modern font
        self.title_font = ("Helvetica", 18, "bold")  # Title font
        self.file_list = []  # List to store selected files
        self.burned_files = {}  # Dictionary to store burned status of files
        self.root.configure(bg=self.bg_color)
        self.state = ')'
        self.state1_counter = 0
        self.painter_state = 0
        self.init = True
        self.create_main_menu()
        self.init = False
        self.root.mainloop()

    def clear_window(self):
        for widget in self.root.winfo_children():
            widget.destroy()

    def create_main_menu(self):
        self.clear_window()
        self.root.configure(bg=self.bg_color)

        self.label = tk.Label(self.root, text="DCS Final Project - Guy Noy & Tav Sheli", font=self.title_font,
                              bg=self.bg_color, fg=self.btn_fg_color)
        self.label.pack(padx=10, pady=10)

        self.button1 = tk.Button(self.root, text="Manual Control Of Motor-Based Machine", font=self.font,
                                 bg=self.btn_color, fg=self.btn_fg_color, command=self.create_manual_control_menu)
        self.button1.pack(padx=10, pady=10, fill=tk.X)

        self.button2 = tk.Button(self.root, text="Joystick Based PC Painter", font=self.font, bg=self.btn_color,
                                 fg=self.btn_fg_color, command=self.create_joystick_painter_menu)
        self.button2.pack(padx=10, pady=10, fill=tk.X)

        self.button3 = tk.Button(self.root, text="Stepper Motor Calibration", font=self.font, bg=self.btn_color,
                                 fg=self.btn_fg_color, command=self.create_stepper_motor_menu)
        self.button3.pack(padx=10, pady=10, fill=tk.X)

        self.button4 = tk.Button(self.root, text="Script Mode", font=self.font, bg=self.btn_color, fg=self.btn_fg_color,
                                 command=self.create_script_mode_menu)
        self.button4.pack(padx=10, pady=10, fill=tk.X)
        if self.init == False:
            self.state = ')'
            self.send_state(self.state)
            
        

    def create_back_button(self):
        self.back_button = tk.Button(self.root, text="Back", font=self.font, bg=self.btn_color, fg=self.btn_fg_color,
                                     command=self.create_main_menu)
        self.back_button.pack(padx=10, pady=10, side=tk.BOTTOM, anchor='sw')

    def create_manual_control_menu(self):
        self.state1_counter = 0
        if self.init == False:
            self.state = '!'
            self.send_state(self.state)
            

        self.clear_window()
        self.root.configure(bg=self.bg_color)

        # Title
        self.label = tk.Label(self.root, text="Manual Control Of Motor-Based Machine", font=self.title_font,
                              bg=self.bg_color, fg=self.btn_fg_color)
        self.label.pack(padx=10, pady=10)


        # Frame to align the buttons horizontally
        self.button_frame = tk.Frame(self.root, bg=self.bg_color)
        self.button_frame.pack(pady=10)

        self.status_message = tk.Label(self.root, text="Bring the motor to angle = 0", font=self.font,
                                       bg=self.bg_color, fg=self.btn_fg_color)
        self.status_message.pack(pady=20)

        # Analog Joystick Control button (placed below the other two)
        self.state1_button = tk.Button(self.root, text="Start motor", font=self.font, bg=self.btn_color,
                                         fg=self.btn_fg_color, command= lambda: self.joystick_control())
        self.state1_button.pack(pady=20)     

        # Back button
        self.create_back_button()

    def joystick_control(self):       
        if self.state1_counter == 0:
            self.send_state('%')
            #change button to Stop motor 
            self.state1_button.configure(text = "Stop motor")
            self.status_message.configure(text="Stop the motor when reach angle = 0")
            self.state1_counter += 1
        elif self.state1_counter == 1:
            self.send_state('%')
            # remove button
            self.state1_button.destroy()
            self.status_message.configure(text="Motor currently controlled by joystick")

        
    def create_joystick_painter_menu(self):
        if not self.init:
            self.state = '@'
            self.send_state(self.state)  # Sending state '3' when initializin
        s.flush()
        s.reset_input_buffer()
        self.clear_window()
        self.root.configure(bg=self.bg_color)

        self.label = tk.Label(self.root, text="Joystick Based PC Painter", font=self.title_font, bg=self.bg_color,
                              fg=self.btn_fg_color)
        self.label.pack(padx=10, pady=10)
        
        # Toggle between Joystick and Mouse control
        self.control_button = tk.Button(self.root, text="Switch to Joystick Control", font=self.font,
                                        bg=self.btn_color, fg=self.btn_fg_color, command= self.toggle_control_mode)
        self.control_button.pack(padx=10, pady=10)

        self.toolbar = tk.Frame(self.root, bg=self.bg_color)
        self.toolbar.pack(side=tk.TOP, fill=tk.X)

        # Container frame to center toolbar items
        toolbar_inner_frame = tk.Frame(self.toolbar, bg=self.bg_color)
        toolbar_inner_frame.pack(expand=True, anchor=tk.CENTER)

        self.colors = ["#000000", "#FF0000", "#008000", "#0000FF", "#FFFF00", "#800080", "#FFA500", "#FFC0CB",
                       "#A52A2A"]
        self.current_color = self.colors[0]
        self.pen_size = 5
        self.erase_size = 15

        self.color_buttons = []
        for color in self.colors:
            btn = tk.Button(toolbar_inner_frame, bg=color, width=3, command=lambda col=color: self.change_color(col))
            btn.pack(side=tk.LEFT, padx=2, pady=2)
            self.color_buttons.append(btn)

        self.size_label = tk.Label(toolbar_inner_frame, text="Size:", bg=self.bg_color)
        self.size_label.pack(side=tk.LEFT, padx=2, pady=2)

        self.size_scale = tk.Scale(toolbar_inner_frame, from_=1, to=20, orient=tk.HORIZONTAL, bg=self.bg_color,
                                   fg=self.btn_fg_color, command=self.change_size)
        self.size_scale.set(self.pen_size)
        self.size_scale.pack(side=tk.LEFT, padx=2, pady=2)

        self.mode_label = tk.Label(toolbar_inner_frame, text="Mode: Pen", bg=self.bg_color, font = self.font)
        self.mode_label.pack(side=tk.LEFT, padx=2, pady=2)


        # Canvas to paint on
        self.canvas = tk.Canvas(self.root, bg=self.bg_color, width=600, height=400, cursor='pencil')
        self.canvas.pack()
        self.canvas.bind("<B1-Motion>", self.paint)
        self.center_x = self.canvas.winfo_width() // 2
        self.center_y = self.canvas.winfo_height() // 2
        
        self.create_back_button()
        # Start the joystick listener in a thread
        self.joystick_mode = False  # Start with normal mouse control
        self.is_running = True
        threading.Thread(target=self.joystick_listener, daemon=True).start()

    def toggle_control_mode(self):
        self.joystick_mode = not self.joystick_mode
        if self.joystick_mode:
            self.control_button.config(text="Switch to Mouse Control")
            # Reset mouse to center of the canvas
            self.center_x = self.canvas.winfo_width() // 2
            self.center_y = self.canvas.winfo_height() // 2
            pyautogui.moveTo(self.root.winfo_x() + self.center_x + self.canvas.winfo_rootx(),
                         self.root.winfo_y() + self.center_y + self.canvas.winfo_rooty())
            # pyautogui.moveTo(self.center_x,self.center_y)
        else:
            self.control_button.config(text="Switch to Joystick Control")

    def joystick_listener(self):
        while self.is_running:           
            while s.in_waiting > 0 :
                if self.joystick_mode:
                    try:
                        data = self.read_from_msp430()  # Read angle from UART
                        if data != '':
                            if data == '^':
                                self.painter_state = (self.painter_state + 1) % 3
                                if self.painter_state == 0:
                                    self.canvas.config(cursor='pencil')
                                    self.mode_label.config(text = 'Mode: Pen')
                                elif self.painter_state == 1:
                                    self.canvas.config(cursor='X_cursor')
                                    self.mode_label.config(text = 'Mode: Erase')
                                else:
                                    self.canvas.config(cursor = 'hand1')
                                    self.mode_label.config(text = 'Mode: Neutral')                                
                            else:
                                angle = data.strip("\x00")
                                angle = int(angle)
                                angle += 180
                                angle %= 360
                                self.move_mouse_with_joystick(angle)
                    except Exception as e:
                        print(f"Error reading from MSP430: {e}")

    def move_mouse_with_joystick(self, angle):
    # Calculate the new mouse position based on the angle and a fixed distance
        distance = 6  # Set the distance of movement based on the joystick angle
        radian_angle = math.radians(angle)

        new_x =  (self.center_x + distance * math.cos(radian_angle))
        new_y =  (self.center_y + distance * math.sin(radian_angle))

        new_x = max(0, min(new_x, self.canvas.winfo_width()))
        new_y = max(0, min(new_y, self.canvas.winfo_height()))

        # Move the mouse cursor
        pyautogui.moveTo(self.root.winfo_x() + new_x + self.canvas.winfo_rootx()+8,
                        self.root.winfo_y() + new_y + self.canvas.winfo_rooty()+8)
        if self.painter_state == 0:    
            self.canvas.create_line(self.center_x, self.center_y,
                                new_x, new_y, width = self.pen_size,
                                fill=self.current_color)
        elif self.painter_state == 1:
            self.canvas.create_oval(self.center_x, self.center_y,
                                new_x, new_y, width = self.erase_size, outline= 'white',
                                fill='white')           

        self.center_x = new_x
        self.center_y = new_y

    def change_color(self, new_color):
        self.current_color = new_color
        self.highlight_selected_color()

    def highlight_selected_color(self):
        for btn in self.color_buttons:
            btn.config(relief=tk.RAISED)
            if btn["bg"] == self.current_color:
                btn.config(relief=tk.SUNKEN)

    def change_size(self, new_size):
        self.pen_size = int(new_size)

    def paint(self, event):
        x1, y1 = (event.x - self.pen_size), (event.y - self.pen_size)
        x2, y2 = (event.x + self.pen_size), (event.y + self.pen_size)
        self.canvas.create_oval(x1, y1, x2, y2, fill=self.current_color, outline=self.current_color)

    def create_stepper_motor_menu(self):
        if not self.init:
            self.state = '#'
            self.send_state(self.state)  # Sending state '3' when initializing

        self.clear_window()
        self.root.configure(bg=self.bg_color)

        # Title (same as before)
        self.label = tk.Label(self.root, text="Stepper Motor Calibration", font=self.title_font, bg=self.bg_color,
                            fg=self.btn_fg_color)
        self.label.pack(padx=10, pady=10)

        # State message at the top (initialized to the first message)
        self.state_message = tk.Label(self.root, text="Press the joystick to start rotate.", font=self.font,
                                    bg=self.bg_color, fg=self.btn_fg_color)
        self.state_message.pack(pady=20)

        # Frame to center the counter and degree in the middle
        self.center_frame = tk.Frame(self.root, bg=self.bg_color)
        self.center_frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

        # Counter display in the center frame
        self.counter_label = tk.Label(self.center_frame, text="Counter:", font=self.font, bg=self.bg_color,
                                    fg=self.btn_fg_color)
        self.counter_label.grid(row=0, column=0, padx=10, pady=5, sticky="e")

        self.counter_value = tk.Label(self.center_frame, text="0", font=self.font, bg=self.bg_color,
                                    fg=self.btn_fg_color)
        self.counter_value.grid(row=0, column=1, padx=10, pady=5, sticky="w")

        # Degree display in the center frame
        self.degree_label = tk.Label(self.center_frame, text="Angle In Degrees:", font=self.font, bg=self.bg_color,
                                    fg=self.btn_fg_color)
        self.degree_label.grid(row=1, column=0, padx=10, pady=5, sticky="e")

        self.degree_value = tk.Label(self.center_frame, text="0.0", font=self.font, bg=self.bg_color,
                                    fg=self.btn_fg_color)
        self.degree_value.grid(row=1, column=1, padx=10, pady=5, sticky="w")

        # Back button
        self.create_back_button()

        # Use threading for sampling data from the MSP430
        self.state1_stage = '0'
        self.is_running = True  # Variable to control the thread execution

        # Start a thread to continuously read from MSP430
        thread = threading.Thread(target=self.sample_msp430_data_state3)
        thread.daemon = True  # Daemonize thread to close with the program
        thread.start()

    def sample_msp430_data_state3(self):
        while self.is_running and self.state == '#':
            data = self.read_from_msp430()
            if data != '':
                if data == '1':
                    self.state1_stage = '2'
                    self.update_gui(lambda: self.state_message.configure(text="Press again to stop the motor when reach angle = 0."))
                elif data == '2':
                    self.state1_stage = '3'
                    self.update_gui(lambda: self.state_message.configure(text="Press again to stop the motor when completing a full circle."))
                else:
                    # phi = self.state_3_read_from_msp430()
                    counter_value = data.rstrip('\0x00')
                    phi = 360/int(counter_value)

                    # Update the GUI with the new values
                    self.update_gui(lambda: self.degree_value.configure(text=str(phi)))
                    self.update_gui(lambda: self.counter_value.configure(text=str(counter_value)))
                    self.is_running = False

    def update_gui(self, update_function):
        """This method is used to update the GUI from the thread"""
        self.root.after(0, update_function)

    def read_from_msp430(self, size = None):
        if self.state == '@':       #state 2 (Painter)
            try:
                data = s.read(size = 4).decode('ascii')
                return data
            except Exception as e:
                print(f"Error reading from MSP430: {e}")
                return None
        elif (self.state == '#'):                       #state 3 (Calibration)
            try:
                data = s.readline().decode('ascii')
                return data
            except Exception as e:
                print(f"Error reading from MSP430: {e}")
                return None
        elif self.state == '$':
            try:
                data = s.read(size = 6).decode('ascii')
                return data
            except Exception as e:
                print(f"Error reading from MSP430: {e}")
                return None

    def create_back_button(self):
        self.back_button = tk.Button(self.root, text="Back", font=self.font, bg=self.btn_color, fg=self.btn_fg_color,
                                    command=self.stop_sampling)
        self.back_button.pack(padx=10, pady=10, side=tk.BOTTOM, anchor='sw')

    def stop_sampling(self):
        """Stop the data sampling and go back to the main menu"""
        self.is_running = False  # This stops the thread
        self.create_main_menu()  # Go back to the main menu


    def create_script_mode_menu(self):
        if not self.init:
            self.state = '$'
            self.send_state(self.state)  # Sending state '$' when initializing
        self.clear_window()
        self.root.configure(bg=self.bg_color)

        self.label = tk.Label(self.root, text="Script Mode", font=self.title_font, bg=self.bg_color,
                            fg=self.btn_fg_color)
        self.label.pack(padx=10, pady=10)

        # Center frame to hold all file frames
        center_frame = tk.Frame(self.root, bg=self.bg_color)
        center_frame.pack(expand=True, padx=10, pady=10)

        # Restore or initialize the attributes for each file
        if not hasattr(self, 'file1'):
            self.file1 = {"name": None, "burned": False, "label": None, "decoded_string": ""}
        if not hasattr(self, 'file2'):
            self.file2 = {"name": None, "burned": False, "label": None, "decoded_string": ""}
        if not hasattr(self, 'file3'):
            self.file3 = {"name": None, "burned": False, "label": None, "decoded_string": ""}

        # Frame for File 1
        self.create_file_frame("File 1", self.file1, center_frame)

        # Frame for File 2
        self.create_file_frame("File 2", self.file2, center_frame)

        # Frame for File 3
        self.create_file_frame("File 3", self.file3, center_frame)

        # Create a label for displaying the motor angle during stepper_deg execution
        self.motor_angle_label = tk.Label(self.root, text="", font=self.font, bg=self.bg_color, fg="blue")
        self.motor_angle_label.place(relx=0.75, rely=0.5, anchor="e")  # Position on the center-right of the window
        self.motor_angle_label.place_forget()  # Initially hide the labely

        self.create_back_button()

    def create_file_frame(self, file_name, file_data, parent_frame):
        frame = tk.Frame(parent_frame, bg=self.bg_color)
        frame.pack(pady=10, fill=tk.X)

        label = tk.Label(frame, text=file_name, font=self.font, bg=self.bg_color, fg=self.btn_fg_color)
        label.grid(row=0, column=0, columnspan=4, pady=5)

        browse_button = tk.Button(frame, text="Browse", font=self.font, bg=self.btn_color,
                                fg=self.btn_fg_color, command=lambda: self.browse_file(file_data))
        browse_button.grid(row=1, column=0, padx=5, pady=5)

        burn_button = tk.Button(frame, text="Burn", font=self.font, bg=self.btn_color,
                                fg=self.btn_fg_color, command=lambda: self.burn_file(file_data))
        burn_button.grid(row=1, column=1, padx=5, pady=5)

        execute_button = tk.Button(frame, text="Execute", font=self.font, bg=self.btn_color,
                                fg=self.btn_fg_color, command=lambda: self.execute_file(file_data))
        execute_button.grid(row=1, column=2, padx=5, pady=5)

        # Label to display the green checkmark after successful execution
        file_data["ack_label"] = tk.Label(frame, text="", font=self.font, bg=self.bg_color, fg="green")
        file_data["ack_label"].grid(row=1, column=3, padx=5, pady=5)

        # Box to display the selected file
        file_box_frame = tk.Frame(frame, bg=self.bg_color, bd=2, relief=tk.SUNKEN)
        file_box_frame.grid(row=2, column=0, columnspan=4, pady=5, padx=5, sticky="ew")

        file_data["label"] = tk.Label(file_box_frame, text=file_data["name"] if file_data["name"] else "No file selected",
                                    font=self.font, bg=self.bg_color if not file_data["burned"] else "#90EE90",
                                    fg=self.btn_fg_color)
        file_data["label"].pack(fill=tk.BOTH, expand=True, padx=5, pady=5)


    def browse_file(self, file_data):
        file_path = filedialog.askopenfilename()
        if file_path:
            file_name = os.path.basename(file_path)
            file_data["name"] = file_name
            file_data["burned"] = False
            file_data["decoded_string"] = self.decode_script(file_path)  # Decode the file immediately
            file_data["label"].config(text=file_name, bg="#FFCCCB")  # Light red for not burned


    def decode_script(self, file_name):
        opcode_map = {
            'inc_lcd': '01',
            'set_delay': '04',
            'dec_lcd': '02',
            'rra_lcd': '03',
            'clear_lcd': '05',
            'stepper_deg': '06',
            'stepper_scan': '07',
            'sleep': '08'
        }

        decoded_string = ""

        with open(file_name, 'r') as file:
            lines = file.readlines()

        for line in lines:
            command = line.strip().split()

            if len(command) == 2:
                operation, operand = command
                if operation == 'rra_lcd':
                    # Convert the ASCII character to its hexadecimal ASCII value
                    decoded_operand = format(ord(operand), '02X')
                elif ',' in operand:  # Handle multiple operands for other operations
                    operands = operand.split(',')
                    decoded_operand = ''.join(format(int(op), '02X') for op in operands)
                else:
                    # Convert the operand as a normal integer for other operations
                    decoded_operand = format(int(operand), '02X')
                decoded_line = opcode_map[operation] + decoded_operand
            else:
                operation = command[0]
                decoded_line = opcode_map[operation]

            decoded_string += decoded_line + '\n'

        return decoded_string


    def burn_file(self, file_data):
        
        if file_data["name"]:
            # Determine the identifier based on which file we're burning
            if file_data == self.file1:
                identifier = 'G'  # Recieve & Burn file 1
            elif file_data == self.file2:
                identifier = 'J'  # Recieve & Burn file 2
            elif file_data == self.file3:
                identifier = 'L'  # Recieve & Burn file 3
            # Assuming burn_to_msp430 takes the file name and segment
            self.send_state(file_data["decoded_string"] + identifier)
            
            self.is_waiting_for_ack = True
            thread = threading.Thread(target=self.wait_for_ack_and_update_gui1, args=(file_data,))
            thread.daemon = True
            thread.start()
        else:
            print("No file selected")
    
    def wait_for_ack_and_update_gui1(self, file_data, timeout=5):
        if self.wait_for_ack(timeout):
            file_data["burned"] = True
            file_data["label"].config(bg="#90EE90")  # Light green for burned
            # print(f"{file_data['name']} burned successfully")
        else:
            print("Failed to receive ACK from MSP430")
        
        self.is_waiting_for_ack = False  # Stop the thread
    
    def wait_for_ack(self, timeout=5):
        while self.is_waiting_for_ack:
            if s.in_waiting > 0:  # Check if data is available in the serial buffer
                ack = s.read().decode('ascii')
                if ack == '&':  # ACK received
                    return True
        return False  # Timeout or no ACK received

    def execute_file(self, file_data):
        if file_data["name"] and file_data["burned"]:
            # Clear the label before starting a new execution
            file_data["ack_label"].config(text="")

            # Determine the identifier based on which file we're executing
            if file_data == self.file1:
                identifier = 'H'  # Execute file 1
            elif file_data == self.file2:
                identifier = 'K'  # Execute file 2
            elif file_data == self.file3:
                identifier = 'Q'  # Execute file 3

            # Send the execute command with the identifier
            self.send_state(identifier)

            # Start a new thread to wait for the ACK from MSP430 after execution
            thread = threading.Thread(target=self.wait_for_ack_and_update_gui2, args=(file_data,))
            thread.daemon = True  # Daemonize thread to close with the program
            thread.start()

        else:
            print("No file selected or file not burned")
    
    def wait_for_ack_and_update_gui2(self, file_data):
        while True:
            while s.in_waiting > 0:  # Check if data is available in the serial buffer
                try:
                    data = self.read_from_msp430()
                # Process the buffer
                    if data == '&':
                        file_data["ack_label"].config(text="✓")  # Display the green checkmark
                        self.motor_angle_label.place_forget()
                        return  # Exit after processing the ACK
                    # Look for the 'A' character to identify the start of an angle value
                    if 'A' in data:
                        parts = data.split('A', 1)
                        if len(parts) > 1:
                            angle_data = parts[1].split('&')[0].strip()  # Get the angle value until '&' or end of string
                            self.motor_angle_label.config(text=f"Motor Angle: {angle_data}°")
                            self.motor_angle_label.place(relx=0.7, rely=0.5, anchor="w")  # Show and position the label
                except Exception as e:
                        print(f"Error reading from MSP430: {e}")
                    # If no complete angle or ACK is found, wait for more data


    def send_state(self, message=None):
        s.reset_output_buffer()       
        bytesMenu = bytes(message, 'ascii')
        s.write(bytesMenu)

if __name__ == '__main__':
    # MSP430_port = port_search()

    s = serial.Serial('COM3', baudrate=9600, bytesize=serial.EIGHTBITS,
                   parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
                   timeout=1)
    s.flush()
    enableTX = True
    s.set_buffer_size(1024, 1024)
    s.reset_input_buffer()
    s.reset_output_buffer()
    state = 0
    GUI_CLASS()