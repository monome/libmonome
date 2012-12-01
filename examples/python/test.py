import time
import monome

Y = """
 .   .
  . .
   .
   .
   .
   .\n"""

E = """
 ......
 .
 .
 ....
 .
 ......\n"""

A = """
  .....
 .    .
 .    .
 ......
 .    .
 .    .\n"""

H = """
  .  .
  .  .
  .  .
  ....
  .  .
  .  .\n"""


def str_to_frame(str):
    def row_to_bits(str):
        return [(0 if x == " " else 1) for x in str]
    return [monome._bitmap_data(row_to_bits(x)) for x in str.split("\n")]


class CommandTest(monome.Monome):
    def test_led(self, func):
        for i in xrange(0, 16):
            for j in xrange(0, 16):
                func(j, i)
                time.sleep(0.005)

    def test_width_8(self, on, func):
        on = int(not not on)

        for i in xrange(8):
            func(i, on)
            time.sleep(0.04)

            on |= on << 1
            on &= 255

        for i in xrange(8, 16):
            func(i, on)
            on >>= 1

            time.sleep(0.04)

    def test_width_16(self, on, func):
        on = int(not not on)

        for i in xrange(16):
            func(i, on)
            time.sleep(0.04)

            on |= on << 1

    def test_frame(self, *rest):
        def invert_frame(frame):
            return [x ^ 0xFF for x in frame]

        comp = [str_to_frame(x) or 0 for x in rest]

        for i in xrange(len(rest)):
            self.led_frame(i, comp[i])
            time.sleep(0.5)
            self.led_frame(i, invert_frame(comp[i]))
            time.sleep(0.5)

    def test_mode(self):
        self.mode = monome.MODE_SHUTDOWN
        time.sleep(0.5)
        self.mode = monome.MODE_TEST
        time.sleep(0.5)
        self.mode = monome.MODE_NORMAL
        time.sleep(0.5)

    def fade_out(self):
        for i in reversed(xrange(16)):
            self.intensity = i
            time.sleep(0.05)

    def __call__(self):
        self.led_all(0)

        for i in xrange(0, 2):
            self.test_width_8(1, self.led_row)
            self.test_width_8(1, self.led_col)

        for i in xrange(0, 2):
            self.test_width_16(1, self.led_row)
            self.test_width_16(1, self.led_col)

        self.test_width_16(0, self.led_col)

        self.test_led(self.led_on)
        self.test_led(self.led_off)

        self.test_frame(Y, E, A, H)

        self.test_mode()
        self.fade_out()

        self.led_all(0)
        self.intensity = 0xF

if __name__ == "__main__":
    app = CommandTest("osc.udp://127.0.0.1:8080/monome", 8000)
    app()
