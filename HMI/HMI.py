from SCARA import Scara
from Widgets import *
import tkinter as tk
from tkinter import ttk


class Application(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        self.title("SCARA HMI")
        self.resizable(0, 0)

        self.scara = Scara()

        # Tabs:
        self.notebook = ttk.Notebook(self.master, padding=5)
        self.notebook.grid(sticky='NW')

        self.tab_names = ["Manual", "Auto", "Settings"]
        self.tabs = {}

        for name in self.tab_names:
            self.tabs[name] = tk.Frame(self.notebook)
            self.notebook.add(self.tabs[name], text=name, padding=10)
        # self.notebook.hide(0)
        # self.notebook.hide(1)

        # Message box:
        self.message_box = MessageBox(self.master)
        self.message_box.grid(column=0, row=1, rowspan=2, sticky='NEW', padx=10)
        self.bind("<Button-1>", self.message_box.clear)

        # Status bar:
        self.connected_status = StatusLED(self.master, text='Connected')
        self.ready_status = StatusLED(self.master, text='Ready')
        self.connected_status.grid(column=0, row=1, sticky='SE', padx=10)
        self.ready_status.grid(column=0, row=2, sticky='SE', padx=10)

        # Manual tab:
        if True:
            name = self.tab_names[0]

            self.tabs[name].columnconfigure(1, weight=1)

            self.status_bar_manual = StatusBar(self.tabs[name], scara=self.scara)
            self.status_bar_manual.grid(column=0, row=0, columnspan=2, sticky='EW')

            self.simulation_manual = Simulation(self.tabs[name])
            self.simulation_manual.grid(column=0, row=1, sticky='W')

            self.control_frame = tk.Frame(self.tabs[name])
            self.control_frame.grid(column=1, row=1, sticky='NSEW')
            self.control_frame.columnconfigure(0, weight=1)
            self.control_frame.columnconfigure(1, weight=1)
            self.control_frame.rowconfigure(1, weight=1)

            self.mdi = MDI(self.control_frame, scara=self.scara)
            self.mdi.grid(column=0, row=0, columnspan=2, sticky='NEW')
            self.bind("<Return>", self.mdi.start_mdi)

            self.step_size_frame = StepSize(self.control_frame)
            self.step_size_frame.grid(column=0, row=1, sticky='NEW')

            self.gripper_frame = GripperButton(self.control_frame)
            self.gripper_frame.grid(column=1, row=1, sticky='NEW')

            self.home_button = HomeButton(self.control_frame,
                                          scara=self.scara)
            self.home_button.grid(column=1, row=2, sticky='S', pady=20)

            self.position_frame_manual = PositionPanel(self.tabs[name], self.scara, self.step_size_frame.step_size)
            self.position_frame_manual.grid(column=0, row=2, columnspan=2, sticky='EW')

        # Auto tab:
        if True:
            name = self.tab_names[1]

            self.status_bar_auto = StatusBar(self.tabs[name], scara=self.scara)
            self.status_bar_auto.grid(column=0, row=0, columnspan=2, sticky='EW')

            self.simulation_manual = Simulation(self.tabs[name])
            self.simulation_manual.grid(column=0, row=1, sticky='W')

            self.position_frame_auto = PositionPanel(self.tabs[name],
                                                     self.scara,
                                                     self.step_size_frame.step_size,
                                                     read_only=True)
            self.position_frame_auto.grid(column=0, row=2, columnspan=2, sticky='EW')

        # Settings tab:
        if True:
            name = self.tab_names[2]

            self.tabs[name].columnconfigure(1, weight=1)
            self.tabs[name].rowconfigure(0, weight=1)

            self.menu_frame = tk.Frame(self.tabs[name])
            self.menu_frame.grid(column=0, row=0, sticky='nsew')

            self.content_frame = tk.Frame(self.tabs[name], borderwidth=2, relief='groove')
            self.content_frame.columnconfigure(0, weight=1)
            self.content_frame.grid(column=1, row=0, sticky='nsew', padx=5)

            self.contents = {"Tool table": None,
                             "Point table": None,
                             "Machine": None,
                             "Connection": Connection(self.content_frame, scara=self.scara, app=self),
                             "Service mode": Service(self.content_frame, scara=self.scara, app=self),
                             "Help": None}

            self.menu_options = {}
            self.selected = tk.StringVar()

            for option, i in zip(self.contents, range(len(self.contents))):
                self.menu_options[option] = tk.Radiobutton(self.menu_frame,
                                                           indicator=0,
                                                           variable=self.selected,
                                                           value=option,
                                                           text=option,
                                                           bg='grey75',
                                                           selectcolor='grey90',
                                                           font='none 10 bold',
                                                           width=25,
                                                           pady=5,
                                                           command=self.display_content)
                self.menu_options[option].grid(column=0, row=i, padx=10, sticky='N')

    def display_content(self):
        for content in self.contents.values():
            try:
                content.grid_forget()
            except AttributeError:
                pass
        try:
            self.contents[self.selected.get()].grid(column=0, row=0, sticky='W', padx=5, pady=5)
        except AttributeError:
            pass

    def display(self):
        # If connected update status:
        if root.scara.communication.connected:
            root.connected_status.activate()
        else:
            root.connected_status.deactivate()

        if not root.scara.homed:
            return

        # If homed:

        # If movement finished
        # update ready status:
        if root.scara.communication.ready:
            root.ready_status.activate()
            root.scara.update_position()
        else:
            root.ready_status.deactivate()

        # Update current position:
        self.position_frame_manual.display_pos()
        self.position_frame_auto.display_pos()
        self.status_bar_manual.refresh()
        self.status_bar_auto.refresh()


def update():
    root.scara.communication.connection_check()
    root.scara.communication.ready_check()
    root.scara.communication.send()
    root.scara.homing_finished()
    root.display()
    # Run update() each 10 ms:
    root.after(10, update)


if __name__ == "__main__":
    root = Application()
    update()
    root.mainloop()
