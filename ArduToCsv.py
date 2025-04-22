import serial
import csv
import time
from pynput.keyboard import Key, Listener


key_buffer = []
exit_keys = [Key.shift_r, Key.shift_l]
run = True
ardu = serial.Serial(port='COM3')

def press(key):
    global exit_keys
    global key_buffer
    global run

    if key not in key_buffer:
        key_buffer.append(key)
        #print(key_buffer)
    if check_keys(*exit_keys):
        run = False

def release(key):
    global exit_keys
    global key_buffer
    global run
    if key in key_buffer:
        key_buffer.pop(key_buffer.index(key))
        #print(key_buffer)
    if check_keys(*exit_keys):
        run = False

def check_keys(*args):
    global key_buffer
    for i in args: 
        if not i in key_buffer:
            return False
    return True

def read_data():
    cur = ardu.readline()
    cur = str(cur).split("'")[1].split("\\")[0].split(",")
    if cur == '':
        return None
    cur = [float(i) for i in cur]
    return cur
        
def do_stuff(interval: int):
    with open("Data_arduino.csv", "w") as f:
        f.write("")
    keep_going=Listener(on_press=press, on_release=release)
    keep_going.start()
    last_line = ""
    last_interval = int(time.time())

    avr_pool = {"s_time": 0,
                "n_time":0,
                "temp": 0, 
                "hum": 0,
                "mess": 0
                }
    
    #           {"s_time": 0,
    #             "n_time":0,
    #             "temp": 0, 
    #             "hum": 0
    #             mess: 0}

    avr_pool["s_time"] = read_data()[0]

    while run:
        cur = read_data()
        if cur == None: 
            continue
        if cur != last_line:
            avr_pool["n_time"] = cur[0]
            avr_pool["temp"] += cur[1]
            avr_pool["hum"] += cur[2]
            avr_pool["mess"] += 1
            last_line = cur

        if (int(time.time()) - last_interval)>=interval:
            with open("Data_arduino.csv", "a") as file: 
                file.write(f"{(avr_pool['n_time']-avr_pool['s_time'])/60},{avr_pool["temp"]/avr_pool['mess']},{avr_pool["hum"]/avr_pool['mess']}\n")
            print(avr_pool)
            avr_pool["temp"] = 0
            avr_pool["hum"] = 0
            avr_pool["mess"] = 0
            last_interval = int(time.time())

if __name__ == '__main__':
    Intervals = input("Intervals?: ")
    print("Messuring...")
    do_stuff(int(Intervals))