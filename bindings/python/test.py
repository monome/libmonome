import time
import monome

m = monome.Monome("/dev/ttyUSB0")
m.clear()
print("got a {0}x{1} monome".format(m.rows, m.columns))

frame = """\

 .    .
 ..  ..
 . .. .
 .    .
 .    .
"""

def frame_from_str(s):
    def row_from_str(s):
        return ((1 if x != " " else 0) for x in s[:8])

    return (row_from_str(x) for x in s.split("\n")[:8])

m.led_frame(0, frame_from_str(frame))
