import math
from Communication import *


class GCode:
    # Absolute or incremental mode:
    G90 = True          # is_absolute = True
    G91 = False         # is_absolute = False
    is_absolute = G90   # Absolute as default

    def __init__(self, scara=None):
        self.scara = scara

    def operate(self, line):
        # Check if ready:
        if not self.scara.communication.ready:
            return

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
        P(x)                    # P(x) - Move to x point if exits
        M                       # M - Open or Close the gripper
        G4.                     # G4.x - Wait x seconds
        """

        if 'F' in line:
            feed = self.get_value(line, 'F')
            self.scara.set_feed(feed)

        if 'G90' in line:
            self.is_absolute = self.G90

        if 'G91' in line:
            self.is_absolute = self.G91

        if 'P(' in line:
            pass

        # Motion programming:
        displacement = dict().fromkeys(self.scara.axes_names, 0.0)
        is_motion_programmed = False

        for name in self.scara.axes_names:
            if name in line:
                value = self.get_value(line, name)
                if self.is_absolute:
                    value = value - self.scara.current_position[name]
                displacement[name] = value
                is_motion_programmed = True

        # If A1 or A2 programmed, skip X and Y:
        if 'A1=' not in line or 'A2=' not in line:
            if 'X' in line or 'Y' in line:
                xy = self.scara.forward_kinematics(self.scara.current_position)
                for name in xy:
                    if name in line:
                        value = self.get_value(line, name)
                        if not self.is_absolute:
                            value = value + xy[name]
                        xy[name] = value
                target = self.scara.inverse_kinematics(xy)
                for name in target:
                    displacement[name] = target[name] - self.scara.current_position[name]
                is_motion_programmed = True

        if is_motion_programmed:
            print(displacement)
            self.scara.move(displacement)

        if 'M' in line:
            pass

        if 'G4.' in line:
            time = self.get_value(line, 'G4.')
            self.scara.wait(time)

    @staticmethod
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


class Scara:
    axes_names = ["Z", "A1", "A2", "A3"]
    coordinates_names = ["X", "Y"]

    # Positions:
    current_position = dict.fromkeys(axes_names, 0.0)
    displacement = dict.fromkeys(axes_names, 0.0)
    current_xy = dict.fromkeys(coordinates_names, 0.0)

    # Feed:
    feed = 10           # Default feed 10 mm/s or deg/s

    # Tool:
    current_tool = {"x_offset": 0, "z_offset": 0}

    # PARAMETERS:
    l1 = 93.0
    l2 = 106.5
    min_range = {"Z": 0,
                 "A1": -190,
                 "A2": -280,
                 "A3": 0}
    max_range = {"Z": 300,
                 "A1": 190,
                 "A2": 280,
                 "A3": 0}
    reduction = {}

    def __init__(self):
        self.gcode = GCode(self)
        self.communication = Communication()

        self.current_xy = self.forward_kinematics(self.current_position)
        self.set_feed(self.feed)

    def is_in_range(self, displacement):
        for name in self.axes_names:
            target = self.current_position[name] + displacement[name]
            if target > self.max_range[name] or target < self.min_range[name]:
                return False
        return True

    def home(self):
        pass

    def set_feed(self, feed=0.0):
        if not feed:
            return
        feed = abs(round(feed, 2))
        self.feed = feed
        packet = ['F', feed]
        self.communication.send(packet)

    def wait(self, time=0.0):
        if not time:
            return
        time = abs(round(time, 2))
        packet = ['W', time]
        self.communication.send(packet)

    def move(self, displacement):
        # Check if in range:
        if not self.is_in_range(displacement):
            print("Out of range!")
            return

        # Save displacement:
        self.displacement.update(displacement)

        # Send task:
        packet = ['G']
        for name in self.axes_names:
            displacement[name] = round(displacement[name], 1)
            packet.append(displacement[name])
        self.communication.send(packet)

    def update_position(self):
        for name in self.axes_names:
            self.current_position[name] = self.current_position[name] + self.displacement[name]
            self.displacement[name] = 0.0
        self.current_xy = self.forward_kinematics(self.current_position)

    # Calculate absolute xy position
    # from angles A1 and A2 absolute values:
    def forward_kinematics(self, position):
        if 'A1' not in position or 'A2' not in position:
            return {}

        a1 = math.radians(position["A1"])
        a2 = math.radians(position["A2"])
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

        cos_th2 = (x*x + y*y - l1*l1 - l2*l2) / (2*l1*l2)
        if cos_th2 < -1.0 or cos_th2 > 1.0:
            return {}
        sin_th2 = math.sqrt(1 - cos_th2 * cos_th2)
        cos_th1_nominator = x*(l1 + l2*cos_th2) + y*l2*sin_th2
        sin_th1_nominator = y*(l1 + l2*cos_th2) + x*l2*sin_th2
        # denominator = (l1 + l2*cos_th2)*(l1 + l2*cos_th2) + (l2*sin_th2)*(l2*sin_th2)

        th1 = math.degrees(math.atan2(sin_th1_nominator, cos_th1_nominator))
        th2 = math.degrees(math.acos((x*x + y*y - l1*l1 - l2*l2) / (2*l1*l2)))

        return {"A1": th1, "A2": th2}
