import tkinter as tk
from tkinter import ttk

# style = {"font": "none 10 bold",
#          "anchor": "w",
#          "bg": "grey75",
#          "padx": "8",
#          "pady": "3"}


class Title(tk.Label):
    def __init__(self, master=None, **kwargs):
        self.style = {"font": "none 10 bold",
                      "anchor": "w",
                      "bg": "grey75",
                      "padx": "8",
                      "pady": "3"}
        self.style.update(**kwargs)
        tk.Label.__init__(self, master, self.style)


class MessageBox(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)

        self.box = tk.Label(self,
                            font="none 14 bold",
                            fg='red',
                            anchor='w',
                            justify='left')
        self.box.grid(sticky='EW')
        self.box.bind("<Button-1>", self.clear)

    def throw(self, message):
        if len(message) > 64:
            line1 = str(message[0:64])
            line2 = str(message[65:128])
        else:
            line1 = str(message)
            line2 = ""
        self.box.configure(text=(line1 + '\n' + line2))

    def clear(self, event=None):
        self.box.configure(text="")


class StatusLED(tk.Frame):
    def __init__(self, master=None, text=""):
        tk.Frame.__init__(self, master)

        self.led_active = tk.PhotoImage(file="Icons/LED_active.png")
        self.led_inactive = tk.PhotoImage(file="Icons/LED_inactive.png")

        self.text = tk.Label(self, text=text,
                             font="none 10",
                             anchor="w",
                             padx=8,
                             pady=3)
        self.led = tk.Label(self, image=self.led_inactive)
        self.text.grid(column=0, row=0, sticky='NE')
        self.led.grid(column=1, row=0, sticky='NE')

    def activate(self):
        self.led.configure(image=self.led_active)

    def deactivate(self):
        self.led.configure(image=self.led_inactive)


class StatusBar(tk.Frame):
    def __init__(self, master=None, scara=None):
        tk.Frame.__init__(self, master)
        self.scara = scara

        self.columnconfigure(0, weight=1)

        self.zero_point = Title(self, text='Active zero point: ---')
        self.zero_point.grid(column=0, row=0, sticky='EW')

        self.feed = Title(self, text='Feed: ---.-- mm/min', anchor='e')
        self.feed.grid(column=1, row=0, sticky='EW')

    def refresh(self):
        zp = "Active zero point: "
        feed = "Feed: " + "{:.2f}" + " mm/min"

        self.zero_point.configure(text=zp)
        self.feed.configure(text=feed.format(self.scara.feed))


class Simulation(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)

        self.img = tk.PhotoImage(file="pybot.png")
        self.position_label = tk.Label(self, image=self.img)
        self.position_label.grid(column=0, row=0, padx=5, pady=5)


class MDI(tk.Frame):
    def __init__(self, master=None, scara=None):
        tk.Frame.__init__(self, master)
        self.scara = scara

        self.columnconfigure(0, weight=1)

        self.mdi_label = Title(self, text='MDI:')
        self.mdi_label.grid(column=0, row=0, sticky='EW', pady=5)

        self.text_field = tk.Text(self, width=1, height=6, padx=5)
        self.text_field.grid(column=0, row=1, sticky='EW')

    def start_mdi(self, event=None):
        if not self.scara.homed:
            return
        if not self.scara.is_ready():
            return
        line = self.text_field.get("1.0", "end-2c")
        self.text_field.delete("1.0", "end")
        print(line)
        self.scara.g_code(line)


class StepSize(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.columnconfigure(0, weight=1)

        self.step_size = tk.DoubleVar()

        self.label = Title(self, text='Step size:')
        self.label.grid(column=0, row=0, pady=5, sticky='EW')
        self.radio = [0, 0.1, 1, 10]
        for i in range(len(self.radio)):
            self.radio[i] = tk.Radiobutton(self,
                                           variable=self.step_size,
                                           value=self.radio[i],
                                           text=str(self.radio[i]),
                                           command=self.set_step_size)
            self.radio[i].grid(column=0, row=i + 1, sticky='W')
            self.radio[i].deselect()
        self.radio[0].configure(text="Continuous")
        self.radio[0].select()

    # Change step size:
    def set_step_size(self):
        print(str(self.step_size.get()))

    """
        step_size = self.step_size.get()
        if step_size == 0:
            self.step_size = 1
            # panel.continuous()
        else:
            self.step_size = step_size
            # panel.single()
    """


class GripperButton(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.columnconfigure(0, weight=1)

        self.gripper_state = 0

        self.label = Title(self, text='Gripper:')
        self.label.grid(column=0, row=0, sticky='EW', pady=5)

        self.gripper_active = tk.PhotoImage(file="Icons/Active.png")
        self.gripper_inactive = tk.PhotoImage(file="Icons/Inactive.png")
        self.button = tk.Label(self, image=self.gripper_inactive)
        self.button.grid(column=0, row=1)
        self.button.bind("<Button-1>", self.gripper_onoff)

    def gripper_onoff(self, event=None):
        if self.gripper_state == 0:
            # Close the gripper:
            self.gripper_state = 1
            self.button.configure(image=self.gripper_active)
            print("Gripper closed")
        else:
            # Open the gripper:
            self.gripper_state = 0
            self.button.configure(image=self.gripper_inactive)
            print("Gripper opened")


class HomeButton(tk.Button):
    def __init__(self, master=None, scara=None, **kwargs):
        self.style = {"text": "HOME",
                      "font": "none 14 bold",
                      "bg": "orange",
                      "activebackground": "orange",
                      "padx": 20}
        self.style.update(**kwargs)
        tk.Button.__init__(self, master, self.style, command=self.home)
        self.scara = scara

    def home(self):
        if not self.scara.communication.connected:
            return

        # If not first use:
        if self.scara.homed:
            # Check if ready for homing:
            if not self.scara.is_ready():
                return

        # Message:
        """
        "Homing procedure"
        message = "Move manually:\n" \
                  "Arm to its positive endpoint\n" \
                  "Forearm to its negative endpoint\n" \
                  "Z axes will be positioned automatically\n" \
                  "Press OK to continue"
        
        # First use of home:
        if not self.scara.is_homed:
            # Enable Auto and Manual tabs:
            self.show_tab_function()
            self.configure(bg="orange", activebackground="orange")
        """
        self.scara.home()
        print(self.scara.homed)


class PositionBar(tk.Frame):
    def __init__(self, master=None, name=None, read_only=False):
        tk.Frame.__init__(self, master)
        self.panel = master
        self.name = name
        self.is_pressed = False

        # Decrement button:
        self.dec_button = tk.Label(self)
        self.dec_button.grid(column=0, row=0)
        if not read_only:
            self.dec_icon = tk.PhotoImage(file="Icons/negative.png")
            self.dec_button.bind("<Button-1>", lambda event, direction=-1: self.step(direction=direction))
        else:
            self.dec_icon = tk.PhotoImage(file="Icons/negative_inactive.png")
        self.dec_button.configure(image=self.dec_icon)

        # Axis label:
        self.axis_label = tk.Label(self,
                                   text=name,
                                   font='none 16 bold',
                                   bg='orange',
                                   width=2)
        self.axis_label.grid(column=1, row=0, ipadx=10, ipady=5)

        # Current position label:
        self.current_position_label = tk.Label(self,
                                               font='none 16 bold',
                                               bg='white',
                                               width=6,
                                               padx=10)
        self.current_position_label.grid(column=2, row=0, ipady=5)

        # Displacement label:
        self.displacement_label = tk.Label(self,
                                           font='none 16 bold',
                                           bg='white',
                                           width=6,
                                           padx=10)
        self.displacement_label.grid(column=3, row=0, ipady=5)

        # Increment button:
        self.inc_button = tk.Label(self)
        self.inc_button.grid(column=4, row=0)
        if not read_only:
            self.inc_icon = tk.PhotoImage(file="Icons/positive.png")
            self.inc_button.bind("<Button-1>", lambda event, direction=1: self.step(direction=direction))
        else:
            self.inc_icon = tk.PhotoImage(file="Icons/positive_inactive.png")
        self.inc_button.configure(image=self.inc_icon)

    def step(self, direction=1, event=None):
        if not self.panel.scara.homed:
            return
        if not self.panel.scara.is_ready():
            return
        step = self.panel.step_size.get() * direction
        is_absolute = self.panel.scara.is_absolute
        line = "G91" + self.name
        if self.name[0] is 'A':
            line = line + "="
        line = line + str(step)
        self.panel.scara.g_code(line)
        self.panel.scara.is_absolute = is_absolute

    def display_current_position(self, position):
        self.current_position_label.configure(text=str(position))

    def display_displacement(self, displacement):
        delta = '\u0394'
        self.displacement_label.configure(text=delta + str(displacement))

    """
    def button_pressed(self, direction):
        self.is_pressed = True
        # while self.is_pressed:
        #     self.step(direction)

    def button_released(self):
        self.is_pressed = False
        # interrupt

    def continuous(self):
        self.dec_button.bind("<Button-1>", lambda event, d=-self.step_size: self.button_pressed(d))
        self.inc_button.bind("<Button-1>", lambda event, d=self.step_size: self.button_pressed(d))

    def single(self):
        self.dec_button.bind("<Button-1>", lambda event, d=-self.step_size: self.step(d))
        self.inc_button.bind("<Button-1>", lambda event, d=self.step_size: self.step(d))
    """


class PositionPanel(tk.Frame):
    def __init__(self, master=None, scara=None, step_size=None, read_only=False):
        tk.Frame.__init__(self, master)
        self.scara = scara
        self.step_size = step_size

        self.bars = {}
        keys = self.scara.coordinates_names + self.scara.axes_names
        c = 0
        r = 0
        for key in keys:
            self.bars[key] = PositionBar(self, name=key, read_only=read_only)
            self.bars[key].grid(column=c, row=r, padx=30)
            self.bars[key].display_current_position('---.--')
            self.bars[key].display_displacement('---.--')
            r = r + 1
            if r == 3:
                r = 0
                c = 1

    def display_pos(self):
        names = self.scara.coordinates_names + self.scara.axes_names
        for key in names:
            if key in self.scara.coordinates_names:
                position = round(self.scara.current_xy[key], 1)
                displacement = round(0.0, 1)
            else:
                position = round(self.scara.current_position[key], 1)
                displacement = round(self.scara.displacement[key], 1)
            self.bars[key].display_current_position(position)
            self.bars[key].display_displacement(displacement)


class SettingsMenu(tk.Frame):
    def __init__(self, master=None, app=None):
        tk.Frame.__init__(self, master)
        self.app = app

        self.menu_options = {"Tool table": None,
                             "Point table": None,
                             "Machine": None,
                             "Connection": None,
                             "Service mode": None,
                             "Help": None}
        self.selected = tk.StringVar()

        for option, i in zip(self.menu_options, range(len(self.menu_options))):
            self.menu_options[option] = tk.Radiobutton(self,
                                                       indicator=0,
                                                       variable=self.selected,
                                                       value=option,
                                                       text=option,
                                                       bg='grey75',
                                                       selectcolor='grey90',
                                                       font='none 10 bold',
                                                       width=25,
                                                       pady=5,
                                                       command=app.display_content)
            self.menu_options[option].grid(column=0, row=i, padx=20, sticky='N')


class Connection(tk.Frame):
    def __init__(self, master=None, scara=None, app=None):
        tk.Frame.__init__(self, master)
        self.scara = scara
        self.app = app

        # Select device:
        self.port_label = tk.Label(self, text="Port: ")
        self.port_label.grid(column=0, row=0, sticky='E')
        self.selected_port = tk.StringVar()
        self.selected_port.set("")
        self.ports = ttk.Combobox(self,
                                  textvariable=self.selected_port,
                                  width=12)
        self.ports.grid(column=1, row=0, pady=5)
        self.ports.bind("<Button-1>", self.refresh_devices)

        self.connect = tk.Button(self,
                                 text='Connect',
                                 font='none 10 bold',
                                 bg='gray75',
                                 activebackground='gray75',
                                 width=10,
                                 height=2,
                                 command=self.connect)
        self.connect.grid(column=0, row=1, pady=5)

        self.disconnect = tk.Button(self,
                                    text='Disconnect',
                                    font='none 10 bold',
                                    bg='gray75',
                                    activebackground='gray75',
                                    width=10,
                                    height=2,
                                    command=self.disconnect)
        self.disconnect.grid(column=1, row=1, pady=5)

    def refresh_devices(self, event=None):
        options = self.scara.communication.get_devices()
        self.ports.configure(values=options)

    def connect(self):
        c = self.scara.communication.serial_open(port=self.selected_port.get())
        if c:
            self.app.message_box.throw("Connected to the device")
        else:
            self.app.message_box.throw("Connection failed")

    def disconnect(self):
        self.scara.communication.serial_port.close()
        if self.scara.communication.connected:
            self.app.message_box.throw("Device disconnected")
        self.scara.communication.connected = False


class Service(tk.Frame):
    PASSWORD = '1234'

    def __init__(self, master=None, scara=None, app=None):
        tk.Frame.__init__(self, master)
        self.scara = scara
        self.app = app

        self.is_service = False
        self.pwd = tk.StringVar()

        self.password_entry = tk.Entry(self, textvariable=self.pwd, show="*")
        self.password_entry.grid(column=0, row=0)
        self.button = tk.Button(self,
                                text="LOG IN",
                                font='none 10 bold',
                                bg='gray75',
                                activebackground='gray75',
                                width=10,
                                height=2,
                                command=self.service_log)
        self.button.grid(column=0, row=1, pady=10)

    def service_log(self):
        if not self.is_service:
            if not self.pwd.get() == self.PASSWORD:
                self.app.message_box.throw("WRONG PASSWORD")
                self.password_entry.delete(0, "end")
                return
            # Start service mode:
            self.app.message_box.throw("SERVICE ON")
            self.is_service = True
            self.button.configure(text="LOG OUT")
            self.password_entry.delete(0, "end")
            self.password_entry.insert(0, "SERVICE ON")
            self.password_entry.configure(state='disabled', show="")
            # Enable all tabs:
            self.app.notebook.add(self.app.tabs['Manual'])
            self.app.notebook.add(self.app.tabs['Auto'])
        elif self.is_service:
            self.app.message_box.throw("")
            self.is_service = False
            self.button.configure(text="LOG IN")
            self.password_entry.configure(state='normal', show="*")
            self.password_entry.delete(0, "end")
            if not self.scara.communication.is_connected:
                self.app.notebook.hide(0)
                self.app.notebook.hide(1)
            if not self.scara.is_homed:
                self.app.notebook.hide(1)
