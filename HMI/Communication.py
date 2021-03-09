import serial


class Communication:
    serial_port = None
    is_connected = False

    # Serial defaults:
    PORT = '/dev/ttyUSB0'
    BAUDRATE = 38400
    TIMEOUT = 0.1
    ENCODING = 'UTF-8'

    ready = False

    def __init__(self):
        self.serial_open()

    def serial_config(self,
                      port=PORT,
                      baudrate=BAUDRATE,
                      parity=serial.PARITY_NONE,
                      stopbits=serial.STOPBITS_ONE,
                      bytesize=serial.EIGHTBITS,
                      timeout=TIMEOUT):
        self.serial_port = serial.Serial()
        self.serial_port.port = port
        self.serial_port.baudrate = baudrate
        self.serial_port.parity = parity
        self.serial_port.stopbits = stopbits
        self.serial_port.bytesize = bytesize
        self.serial_port.timeout = timeout

    def serial_open(self):
        # If serial not configured yet
        # initialize it with default parameters:
        if not self.serial_port:
            self.serial_config()

        try:
            self.serial_port.open()
            if self.serial_port.isOpen():
                self.is_connected = True
                self.ready = True
                self.serial_port.flush()
            else:
                raise
        except:
            self.is_connected = False
            print("Connection failed")

    def is_ready(self):
        # Try:
        if not self.is_connected:
            return False

        if not self.ready:
            # If there is data to receive:
            if self.serial_port.in_waiting:
                ack = self.serial_port.read(1).decode(self.ENCODING)
                # If '1' received port is ready
                if ack is '1':
                    self.ready = True
            self.serial_port.reset_input_buffer()

        return self.ready

    def send(self, packet):
        # If packet is not a list:
        if not isinstance(packet, list):
            packet = [packet]

        # Check if there is something to send in the buffer
        if not len(packet):
            # Nothing to send
            return

        # Send packet:
        for value in packet:
            value = str(value) + '\r'
            self.serial_port.write(value.encode(self.ENCODING))

        # Busy...:
        self.ready = False
