import tkinter as tk
from tkinter import ttk
import SCARA
# import Communication


# ******************** Custom widgets: ********************
class Title(tk.Label):
    style = {"font": "none 10 bold",
             "anchor": "w",
             "bg": "grey75",
             "padx": "8",
             "pady": "3"}

    def __init__(self, master=None, **kwargs):
        self.style.update(**kwargs)
        tk.Label.__init__(self, master, self.style)


class MovePanel(tk.Frame):
    def __init__(self, master=None, name=None, scara=None, display_function=None):
        tk.Frame.__init__(self, master)
        self.name = name
        self.scara = scara
        self.display_function = display_function
        self.step_size = 0
        self.is_pressed = False

        # Decrement button:
        self.dec_icon = tk.PhotoImage(file="Icons/negative.png")
        self.dec_button = tk.Label(self, image=self.dec_icon)
        self.dec_button.grid(column=0, row=0)
        self.dec_button.bind("<ButtonRelease-1>", lambda event: self.button_released())

        # Axis label:
        self.axis_label = tk.Label(self,
                                   text=name,
                                   font='none 16 bold',
                                   bg='orange',
                                   width=2)
        self.axis_label.grid(column=1, row=0, ipadx=10, ipady=5)

        # Position label:
        self.position_label = tk.Label(self,
                                       font='none 16 bold',
                                       bg='white',
                                       width=10,
                                       padx=10)
        self.position_label.grid(column=2, row=0, ipady=5)

        # Increment button:
        self.inc_icon = tk.PhotoImage(file="Icons/positive.png")
        self.inc_button = tk.Label(self, image=self.inc_icon)
        self.inc_button.grid(column=3, row=0)
        self.inc_button.bind("<ButtonRelease-1>", lambda event: self.button_released())

    def step(self, direction):
        # if is XYZ Panel:
        if self.name in self.scara.cartesian_names:
            displacement = dict.fromkeys(self.scara.cartesian_names, 0)
            displacement[self.name] = direction * self.step_size
            displacement = self.scara.inverse_kinematics(displacement, self.scara.G91)
        # if is Axes Panel:
        elif self.name in self.scara.axes_names:
            displacement = dict.fromkeys(self.scara.axes_names, 0)
            displacement[self.name] = direction * self.step_size
        else:
            return
        self.scara.move(displacement, self.scara.G91)  # Run function
        self.display_function()

    def button_pressed(self, direction):
        self.is_pressed = True
        print("PRESSED")
        # while self.is_pressed:
        #     self.step(direction)

    def button_released(self):
        self.is_pressed = False
        print("RELEASED")
        # interrupt

    def continuous(self):
        self.dec_button.bind("<Button-1>", lambda event, d=-1: self.button_pressed(d))
        self.inc_button.bind("<Button-1>", lambda event, d=1: self.button_pressed(d))

    def single(self):
        self.dec_button.bind("<Button-1>", lambda event, d=-1: self.step(d))
        self.inc_button.bind("<Button-1>", lambda event, d=1: self.step(d))
# *********************************************************


class Application:
    def __init__(self, master=None):
        self.master = master
        self.master.title("SCARA HMI")
        self.scara = SCARA.Scara()

        # ******************* Create tabs *********************
        self.notebook = ttk.Notebook(self.master, padding=5)
        self.notebook.pack(expand=1, fill='both')

        self.tabs = {"Manual": None,
                     "Auto": None,
                     "Edit": None,
                     "Settings": None}

        for name in self.tabs:
            self.tabs[name] = ttk.Frame(self.notebook)
            self.notebook.add(self.tabs[name], text=name, padding=10)

        """
        # Message box:
        self.msg_label = Title(self.master, text='MSG', font='none 14 bold', fg='red', height=2)
        self.msg_label.pack(side='bottom', fill='x')
        """

        # ******************* Manual tab *********************
        name = "Manual"

        # Tab structure:
        # Zero point (top):
        self.zero_point_label = Title(self.tabs[name], text='Active zero point: Global')
        self.zero_point_label.grid(column=0, row=0, columnspan=3, sticky='EW')

        # Current position graphics (left):
        self.img = tk.PhotoImage(file="pybot.png")
        self.position_label = tk.Label(self.tabs[name], image=self.img)
        self.position_label.grid(column=0, row=1, padx=5, pady=5)

        # MDI, Step size change, Gripper (right):
        self.control_frame = tk.Frame(self.tabs[name])
        self.control_frame.grid(column=1, row=1, columnspan=2, sticky='N')

        # Control (bottom):
        self.axis_frame = tk.Frame(self.tabs[name])
        self.axis_frame.grid(column=0, row=2, columnspan=3)

        # Create Widgets:
        # MDI:
        self.mdi_label = Title(self.control_frame, text='MDI:')
        self.mdi_label.grid(column=0, row=0, columnspan=2, sticky='EW', pady=5)
        self.text_field = tk.Text(self.control_frame, width=41, height=6, padx=5)
        self.text_field.grid(column=0, row=1, columnspan=2)
        self.master.bind("<Return>", self.start_mdi)

        # Step size change:
        self.step_size_frame = tk.Frame(self.control_frame)
        self.step_size_frame.grid(column=0, row=2, sticky='W')
        self.step_size_label = Title(self.step_size_frame, width=16, text='Step size:')
        self.step_size_label.grid(column=0, row=0, pady=5)
        self.step_size = tk.DoubleVar()
        self.step_size_radio = [0, 0.1, 1, 10]
        for i in range(len(self.step_size_radio)):
            self.step_size_radio[i] = tk.Radiobutton(self.step_size_frame,
                                                     variable=self.step_size,
                                                     value=self.step_size_radio[i],
                                                     text=str(self.step_size_radio[i]),
                                                     command=self.set_step_size)
            self.step_size_radio[i].grid(column=0, row=(3+i), sticky='W')
            self.step_size_radio[i].deselect()
        self.step_size_radio[0].configure(text="Continuous")
        self.step_size_radio[0].select()

        # Gripper:
        self.gripper_state = 0
        self.gripper_frame = tk.Frame(self.control_frame)
        self.gripper_frame.grid(column=1, row=2, sticky='N')
        self.gripper_label = Title(self.gripper_frame, width=16, text='Gripper:')
        self.gripper_label.grid(column=0, row=0, sticky='EW', pady=5)
        self.gripper_active = tk.PhotoImage(file="Icons/Active.png")
        self.gripper_inactive = tk.PhotoImage(file="Icons/Inactive.png")
        self.gripper_button = tk.Label(self.gripper_frame, image=self.gripper_inactive)
        self.gripper_button.grid(column=0, row=1)
        self.gripper_button.bind("<Button-1>", self.gripper_onoff)

        self.subpanels = {}

        # XYZ control:
        self.xyz_panel = tk.Frame(self.axis_frame)
        self.xyz_panel.grid(column=0, row=0, padx=30, pady=20)

        for key, i in zip(self.scara.cartesian_names, range(len(self.scara.axes_names))):
            self.subpanels[key] = MovePanel(self.xyz_panel,
                                            name=key,
                                            scara=self.scara,
                                            display_function=self.display_pos)
            self.subpanels[key].grid(column=0, row=i)

        # Axes control:
        self.axes_panel = tk.Frame(self.axis_frame)
        self.axes_panel.grid(column=1, row=0, padx=30, pady=20)

        for key, i in zip(self.scara.axes_names, range(len(self.scara.axes_names))):
            if key == "VERTICAL":
                continue
            self.subpanels[key] = MovePanel(self.axes_panel,
                                            name=key,
                                            scara=self.scara,
                                            display_function=self.display_pos)
            self.subpanels[key].grid(column=0, row=i)
            self.subpanels[key].axis_label.configure(text='A' + str(i + 1))

        self.set_step_size()
        self.display_pos()

        # ******************* Auto tab *********************
        name = "Auto"

        # ******************* Edit tab *********************
        name = "Edit"

        # ******************* Settings tab ******************
        name = "Settings"

        # Tab structure:
        # Menu (left side):
        self.menu = tk.Frame(self.tabs[name])
        self.menu.grid(column=0, row=0)

        # Contents (right side):
        self.contents = tk.Frame(self.tabs[name])
        self.menu.grid(column=1, row=0)

        # Create Widgets:
        # Menu:
        self.menu_options = {"Tool table": None,
                             "Point table": None,
                             "Config": None,
                             "Help": None}
        self.menu_selected = tk.StringVar()

        for name, i in zip(self.menu_options, range(len(self.menu_options))):
            self.menu_options[name] = tk.Radiobutton(self.menu,
                                                     indicator=0,
                                                     variable=self.menu_selected,
                                                     value=name,
                                                     text=name,
                                                     bg='grey75',
                                                     selectcolor='grey90',
                                                     font='none 10 bold',
                                                     width=30,
                                                     pady=5)
            self.menu_options[name].grid(column=0, row=i, pady=2)

        # Contents:

    # ********************* Methods: ************************
    # Display current position in xyz and angles
    def display_pos(self):
        for key in self.scara.cartesian_names:
            self.subpanels[key].position_label.configure(text=str(self.scara.xyz[key]))
        for key in self.scara.axes_names:
            if key == "VERTICAL":
                continue
            self.subpanels[key].position_label.configure(text=str(self.scara.axes[key].current_position))

    # MDI Cycle Start
    def start_mdi(self, event=None):
        task = self.text_field.get("1.0", "end-2c")
        self.text_field.delete("1.0", "end")
        # Decode task...

    # Gripper ON/OFF:
    def gripper_onoff(self, event):
        if self.gripper_state == 0:
            # Close the gripper:
            self.gripper_state = 1
            self.gripper_button.configure(image=self.gripper_active)
            print("Gripper closed")
        else:
            # Open the gripper:
            self.gripper_state = 0
            self.gripper_button.configure(image=self.gripper_inactive)
            print("Gripper opened")

    # Change step size:
    def set_step_size(self):
        step_size = self.step_size.get()
        for panel in self.subpanels.values():
            if step_size == 0:
                panel.step_size = 1
                panel.continuous()
            else:
                panel.step_size = step_size
                panel.single()


if __name__ == "__main__":
    root = tk.Tk()
    app = Application(master=root)
    root.mainloop()
