import monome

device = "osc.udp://127.0.0.1:8080/monome"

def button_handler(event):
    x, y = event.x, event.y
    buttons[x][y] ^= 1

    if buttons[x][event.y]:
        m.led_on(x, y)
    else:
        m.led_off(x, y)

buttons = [[0 for x in range(0, 16)] for x in range(0, 16)]
m = monome.Monome(device, 8000)
m.register_handler(monome.BUTTON_DOWN, button_handler)

print("simple.py running, press some buttons!")

try:
	m.event_loop()
except KeyboardInterrupt:
	m.led_all(0)
