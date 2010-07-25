import monome


class SimpleTest(monome.Monome):
    def button_handler(self, event):
        x, y = event.x, event.y
        self.buttons[x][y] ^= 1

        if self.buttons[x][event.y]:
            self.led_on(x, y)
        else:
            self.led_off(x, y)

    def run(self):
        self.buttons = [[0 for x in xrange(0, 16)] for x in xrange(0, 16)]
        self.register_handler(monome.BUTTON_DOWN, self.button_handler)
        self.clear()

        print ("simple.py running, press some buttons!")

        try:
            self.main_loop()
        except:
            self.clear()
            self.close()

if __name__ == "__main__":
    app = SimpleTest("osc.udp://127.0.0.1:8080/monome", 8000)
    app.run()
