SSID_NAME = "set_your_ssid_name"
SSID_PASS = "set_your_ssid_pass"

import utime
import network
import urequests
import ure
import machine

#connect to wifi access point
def connect_wifi(ssid, passkey, timeout=10):
    wifi = network.WLAN(network.STA_IF)
    if wifi.isconnected():
        print('already Connected. skip')
        return wifi
    else:
        wifi.active(True)
        wifi.connect(ssid, passkey)
        while not wifi.isconnected() and timeout > 0:
            print('.')
            utime.sleep(1)
            timeout -= 1

    if wifi.isconnected():
        print('Connected')
        return wifi
    else:
        print('Connection failed!')
        return null

def init_led():
    pin12 = machine.Pin(12, machine.Pin.OUT) 
    pin14 = machine.Pin(14, machine.Pin.OUT) 
    pin27 = machine.Pin(27, machine.Pin.OUT) 
    pin26 = machine.Pin(26, machine.Pin.OUT) 
    pin25 = machine.Pin(25, machine.Pin.OUT) 
    pin33 = machine.Pin(33, machine.Pin.OUT) 
    pin32 = machine.Pin(32, machine.Pin.OUT) 
    pin5 = machine.Pin(5, machine.Pin.OUT) 
    pin18 = machine.Pin(18, machine.Pin.OUT) 
    pin19 = machine.Pin(19, machine.Pin.OUT) 
    pin21 = machine.Pin(21, machine.Pin.OUT)

    pin12.value(0)
    pin14.value(0)
    pin27.value(0)
    pin26.value(0)
    pin25.value(0)
    pin33.value(0)
    pin32.value(0)
    pin5.value(0)
    pin18.value(0)
    pin19.value(0)
    pin21.value(0)

def set_led_level(level):
    pin12 = machine.Pin(12) 
    pin14 = machine.Pin(14) 
    pin27 = machine.Pin(27) 
    pin26 = machine.Pin(26) 
    pin25 = machine.Pin(25) 
    pin33 = machine.Pin(33) 
    pin32 = machine.Pin(32) 
    pin5 = machine.Pin(5) 
    pin18 = machine.Pin(18) 
    pin19 = machine.Pin(19) 

    if level >= 10:
        pin12.value(1)
    
    if level >= 20:
        pin14.value(1)

    if level >= 30:
        pin27.value(1)

    if level >= 40:
        pin26.value(1)

    if level >= 50:
        pin25.value(1)

    if level >= 60:
        pin33.value(1)

    if level >= 70:
        pin32.value(1)

    if level >= 80:
        pin5.value(1)

    if level >= 90:
        pin18.value(1)

    if level >= 100:
        pin19.value(1)

def get_items_count():
    token = 'set_your_token'
    headers = {
            'content-type'  : 'application/json',
            'charset'       : 'utf-8',
            'Authorization' : 'Bearer ' + token
            }
    url = 'https://qiita.com/api/v2/authenticated_user'
    response = urequests.get(url, headers=headers)
    json = response.json()
    count = int(json["items_count"])
    del token
    del headers
    del url
    del response
    del json
    gc.collect()
    
    return count


def get_likes_count(num):
    token = 'set_your_token'
    headers = {
            'content-type'  : 'application/json',
            'charset'       : 'utf-8',
            'Authorization' : 'Bearer ' + token
            }

    u1 = 'https://qiita.com/api/v2/authenticated_user/items?page='
    u2 = str(num)
    u3 = '&per_page=1'
    url = u1 + u2 + u3

    del token
    del u1
    del u2
    del u3
    gc.collect()

    response = urequests.get(url, headers=headers)
    pattern = r'"likes_count":[0-9]+'
    matchOB = ure.search(pattern, response.text)

    if not matchOB:
        sys.exit(0)

    likes_str = matchOB.group(0)
    likes_str = likes_str.split(':')
    count = int(likes_str[1])

    del headers
    del pattern
    del url
    del response
    del matchOB
    del likes_str
    gc.collect()

    return count 

def set_arrivals_led_on():
    pin = machine.Pin(21) 
    pin.value(1)

def set_arrivals_led_off():
    pin = machine.Pin(21) 
    pin.value(0)

def update_likes_led():
    global prev_likes_count

    items_count = get_items_count()

    likes_count = 0
    for num in range(1, items_count+1):
        likes_count += get_likes_count(num)

    print(likes_count)

    target = 100
    rate = (likes_count/target)*100
    set_led_level(int(rate))   
    
    if (likes_count > prev_likes_count) and (prev_likes_count > 0) :
        set_arrival_led_on()

    prev_likes_count = likes_count


init_led()
prev_likes_count = 0

wifi = connect_wifi(SSID_NAME, SSID_PASS)

if not wifi:
    sys.exit(0)

update_likes_led()

t0 = machine.Timer(0)
t0.init(period=0xffff, mode=machine.Timer.PERIODIC, callback=update_likes_led)

tp = machine.TouchPad(machine.Pin(13))
while True:
    utime.sleep(0.1)
    value = tp.read()
    if value < 400:
        print('touched')
        set_arrivals_led_off()

