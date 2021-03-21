import math
from Communication import *


"""
class Parameters:
    min_range = 0
    max_range = 0
    reduction = 0
    current_position = 0
    displacement = 0
"""


class Scara:
    axes_names = ["Z", "A1", "A2", "A3"]
    coordinates_names = ["X", "Y"]

    # Absolute or incremental mode:
    G90 = True          # is_absolute = True
    G91 = False         # is_absolute = False
    is_absolute = G90   # Absolute as default

    # Positions:
    current_position = dict.fromkeys(axes_names, 0.0)
    displacement = dict.fromkeys(axes_names, 0.0)
    target_position = dict.fromkeys(axes_names, 0.0)
    current_xy = dict.fromkeys(coordinates_names, 0.0)

    # Feed [mm/min]:
    default_feed = 200
    feed = 0
    min_feed = 100
    max_feed = 10000
    z_max_feed = 3500

    # Tool:
    current_tool = {"x_offset": 15.5, "z_offset": 0}

    # PARAMETERS:
    l1 = 93.0
    l2 = 106.5
    basement_height = 80.0
    min_range = {"Z": 0,
                 "A1": -230/2,
                 "A2": -280/2,
                 "A3": 0}
    max_range = {"Z": 135,
                 "A1": 230/2,
                 "A2": 280/2,
                 "A3": 0}
    reduction = {"Z": 360/8,        # Z Axis reduction 8 mm/rev screw pitch
                 "A1": 72.0/16.0,   # Arm reduction
                 "A2": 62.0/16.0,   # Forearm reduction
                 "A3": 1,           # Wrist reduction
                 "A21": 62.0/33.0}  # Arm to Forearm superposition

    homed = False
    homing_started = False
    service_mode = False

    def __init__(self):
        self.communication = Communication()
        self.current_xy = self.forward_kinematics(self.current_position)

    def is_in_range(self, displacement):
        for name in self.axes_names:
            target = self.current_position[name] + displacement[name]
            if target > self.max_range[name] or target < self.min_range[name]:
                return False
        return True

    def is_ready(self):
        if not self.communication.ready:
            return False
        elif not self.communication.is_buffer_empty():
            return False
        else:
            return True

    def wait_until_ready(self):
        while not self.communication.ready_check():
            pass

    def home(self):
        self.homed = False

        # Set default feed:
        self.set_feed(self.default_feed)

        # Move Z 10 mm up:
        self.current_position['Z'] = 0
        displacement = dict.fromkeys(self.axes_names, 0.0)
        displacement['Z'] = 5
        self.move(displacement)

        # Move Z down:
        self.current_position['Z'] = self.max_range['Z']
        displacement = dict.fromkeys(self.axes_names, 0.0)
        displacement['Z'] = -self.max_range['Z']
        self.move(displacement)

        self.displacement = dict.fromkeys(self.axes_names, 0.0)
        self.current_position['Z'] = self.min_range['Z']
        self.current_position['A1'] = self.max_range['A1']
        self.current_position['A2'] = self.min_range['A2']
        self.current_position['A3'] = 0.0

        self.homing_started = True

    def homing_finished(self):
        if not self.homing_started:
            return
        if not self.is_ready():
            return
        print("Homed")
        self.homing_started = False
        self.homed = True

    def set_feed(self, feed=0.0):
        if not feed:
            return

        if feed > self.max_feed:
            feed = self.max_feed
        if feed < self.min_feed:
            feed = self.max_feed
        self.feed = feed

        # Send task:
        packet = ['F']
        feed = {}
        for name in self.axes_names:
            feed[name] = self.feed
            if name is 'Z' and self.feed > self.z_max_feed:
                feed[name] = self.z_max_feed
            feed[name] = feed[name] * self.reduction[name] / 360  # 1rev/360deg
            if name is 'A2':
                feed[name] = feed[name] * self.reduction['A21']
            packet.append(round(feed[name], 1))
        self.communication.to_buffer(packet)

    def wait(self, time=0.0):
        if not time:
            return
        time = abs(round(time, 2))
        packet = ['W', time]
        self.communication.to_buffer(packet)

    def move(self, displacement):
        # Check if in range:
        if not self.is_in_range(displacement):
            print("Out of range!")
            return

        # Save displacement:
        self.displacement.update(displacement)

        # Send task:
        packet = ['G']
        # Add A1:A2 superposition:
        displacement['A2'] = displacement['A2'] * self.reduction['A21'] + displacement['A1']
        for name in self.axes_names:
            displacement[name] = displacement[name] * self.reduction[name]
            packet.append(round(displacement[name], 1))
        self.communication.to_buffer(packet)

    def update_position(self):
        for name in self.axes_names:
            self.current_position[name] = self.current_position[name] + self.displacement[name]
            self.displacement[name] = 0.0
        self.current_xy = self.forward_kinematics(self.current_position)

    # Calculate absolute xy position
    # from angles A1 and A2 absolute values:
    def forward_kinematics(self, position):
        try:
            a1 = math.radians(position["A1"])
            a2 = math.radians(position["A2"])
        except KeyError:
            return {}
        l1 = self.l1
        l2 = self.l2 + self.current_tool["x_offset"]

        x = l1 * math.cos(a1) + l2 * math.cos(a1 + a2)
        y = l1 * math.sin(a1) + l2 * math.sin(a1 + a2)

        return {"X": x, "Y": y}

    def inverse_kinematics(self, xy):
        l1 = self.l1
        l2 = self.l2
        x = xy["X"]
        y = xy["Y"]

        try:
            cos_th2 = (x*x + y*y - l1*l1 - l2*l2) / (2*l1*l2)
            sin_th2 = math.sqrt(1 - cos_th2 * cos_th2)
            cos_th1_nominator = x*(l1 + l2*cos_th2) + y*l2*sin_th2
            sin_th1_nominator = y*(l1 + l2*cos_th2) + x*l2*sin_th2
            # denominator = (l1 + l2*cos_th2)*(l1 + l2*cos_th2) + (l2*sin_th2)*(l2*sin_th2)

            th1 = math.degrees(math.atan2(sin_th1_nominator, cos_th1_nominator))
            th2 = math.degrees(math.acos((x*x + y*y - l1*l1 - l2*l2) / (2*l1*l2)))
        except ValueError:
            return {}

        return {"A1": th1, "A2": th2}

    def g_code(self, line):
        # Remove spaces, everything uppercase:
        line = line.replace(" ", "").upper()
        print(line)

        """
        The order of actions:
        G54                     # G54 Px - Select zero point
        F                       # F - Change feed
        G90/G91                 # G90 - Absolute mode, G91 - Increment mode
        P=                      # P=x - Save point
        X, Y, Z, A1=, A2=, A3=  # Motion target point
        P(x)                    # P(x) - Move to x point if such exits
        M                       # M - Open or Close the gripper
        G4.                     # G4.x - Wait x seconds
        """

        if 'F' in line:
            feed = get_value(line, 'F')
            self.set_feed(feed)

        if 'G90' in line:
            self.is_absolute = self.G90

        if 'G91' in line:
            self.is_absolute = self.G91

        if 'P(' in line:
            pass

        # Motion programming:
        displacement = dict().fromkeys(self.axes_names, 0.0)
        is_motion_programmed = False

        for name in self.axes_names:
            if name in line:
                value = get_value(line, name)
                if self.is_absolute:
                    value = value - self.current_position[name]
                displacement[name] = value
                is_motion_programmed = True

        # If A1 or A2 programmed, skip X and Y:
        if 'A1=' not in line or 'A2=' not in line:
            if 'X' in line or 'Y' in line:
                xy = self.forward_kinematics(self.current_position)
                for name in xy:
                    if name in line:
                        value = get_value(line, name)
                        if not self.is_absolute:
                            value = value + xy[name]
                        xy[name] = value
                target = self.inverse_kinematics(xy)
                for name in target:
                    displacement[name] = target[name] - self.current_position[name]
                is_motion_programmed = True

        if is_motion_programmed:
            print(displacement)
            self.move(displacement)

        if 'M' in line:
            pass

        if 'G4.' in line:
            time = get_value(line, 'G4.')
            self.wait(time)


def get_value(line, word):
    i = line.index(word) + len(word)
    value = ""

    # line[i].isalpha() means that the next word begins:
    while not line[i].isalpha():
        if line[i].isnumeric() or line[i] is "-" or line[i] is ".":
            value = value + line[i]
        i = i + 1
        if i >= len(line):
            # End of line, break loop:
            break

    return float(value)
