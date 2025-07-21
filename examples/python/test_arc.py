import monome
import time

KNOBS = 4
BPM = 98

def chill(speed):
    time.sleep(60000. / (BPM * speed) / 1000)

def test_ring(m, r):
    map = [0]*64

    for b in range(16):
        m.led_ring_all(r, b)
        chill(12)

    for i in range(-64, 8):
        m.led_ring_set(r, i & 63, (i >= 0) * 15)
        chill(40)

    for i in range(1, 64):
        m.led_ring_range(r, i, i + (8 - (i - 55 if i >= 55 else 0)), 15)
        chill(40)
        m.led_ring_all(r, 0)

    for i in range(80):
        for b in range(64):
            map[b] = (b - i - 1) & 0xF

        m.led_ring_map(r, map)
        chill(24)

    for i in range(16):
        for b in range(64):
            map[b] = (map[b] + 1) % (16 - i)

        m.led_ring_map(r, map)
        chill(32)

def clear_rings(m):
    for i in range(KNOBS):
        m.led_ring_all(i, 0)

def main():
    m = monome.Monome('/dev/ttyUSB0')
    print(m.serial)
    print(m.devpath)
    clear_rings(m)
    chill(4)

    test_ring(m, 0)

if __name__ == '__main__':
    main()
