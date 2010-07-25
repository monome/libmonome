import monome


class SimpleTest(object):
    def __init__(self):
        self.buttons = [[0 for x in xrange(0, 16)] for x in xrange(0, 16)]

        self.m = monome.Monome("osc.udp://127.0.0.1:8080/monome", 8000)
        self.m.register_handler(monome.BUTTON_DOWN, self.button_handler)

        self.m.clear()

    def button_handler(self, event):
        x, y = event.x, event.y
        self.buttons[x][y] ^= 1

        if self.buttons[x][event.y]:
            self.m.led_on(x, y)
        else:
            self.m.led_off(x, y)

    def run(self):
        try:
            self.m.main_loop()
        except:
            self.m.clear()
            self.m.close()

if __name__ == "__main__":
    app = SimpleTest()
    app.run()
