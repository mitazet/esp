SSID_NAME = "set_your_ssid"
SSID_PASS = "set_your_pass"

import utime
import network
import urequests
import ure
import machine
import ntptime

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

#get string of sunrise/sunset time
def get_time_rise_set(year, month, day, lat,lng):
    str_addr = 'http://labs.bitmeister.jp/ohakon/api/?mode=sun_moon_rise_set&year'
    str_mode = 'mode=sun_rise_set'
    str_year = '&year=' + str(year)
    str_month = '&month=' + str(month)
    str_day = '&day=' + str(day)
    str_lat = '&lat=' + str(lat)
    str_lng = '&lng=' + str(lng)
    url = str_addr + str_mode + str_year + str_month + str_day + str_lat + str_lng 
    response = urequests.get(url)

    pattern = r'<sunrise>[0-9]+.[0-9]+'
    match_sunrise = ure.search(pattern, response.text)

    if not match_sunrise:
        sys.exit(0)

    sunrise_str = str(match_sunrise.group(0))
    sunrise_str = sunrise_str.split('>')

    pattern = r'<sunset>[0-9]+.[0-9]+'
    match_sunset = ure.search(pattern, response.text)
    
    if not match_sunset:
        sys.exit(0)

    sunset_str = str(match_sunset.group(0))
    sunset_str = sunset_str.split('>')
    return (sunrise_str[1], sunset_str[1]) 

#get lat, lng
def get_geocode(zipcode):
    url = 'https://maps.googleapis.com/maps/api/geocode/json?'
    key = 'set_your_token'
    url2 = url + 'address=' + str(zipcode) + '&key=' + key
    response = urequests.get(url2)
    str_json = response.json()
    item_list = str_json['results'][0]
    location = item_list['geometry']['location']
    lat = location['lat']
    lng = location['lng']
    return (lat,lng)

# Time control class
class Time_ctrl:
    def __init__(self):
        utime.set_time(ntptime.time())
        self.year = 2000 
        self.month = 1 
        self.day = 1 
        self.time_led_on = 0
        self.time_led_off = 0

    def update_date(self):
        now = utime.localtime()
        self.year = now[0]
        self.month = now[1]
        self.day = now[2]
    
    def get_date(self):
        return (self.year, self.month, self.day)

    def is_time_update(self):
        if utime.time() > utime.mktime((self.year, self.month, self.day+1, 0, 0, 0, 0, 0)): 
            return True
        else:
            return False

    def update_time_led_on(self, str_time_sunset):
        hour_led_on = float(str_time_sunset)
        sec_led_on = int(hour_led_on * 60 * 60)
        self.time_led_on = utime.mktime((year, month, day, 0, 0, 0, 0, 0)) + sec_led_on

    def update_time_led_off(self, str_time_sunrise):
        hour_led_off = float(str_time_sunrise)
        sec_led_off = int(hour_led_off * 60 * 60)
        self.time_led_off = utime.mktime((year, month, day, 0, 0, 0, 0, 0)) + sec_led_off

    def get_time_led_on(self):
        return self.time_led_on

    def get_time_led_off(self):
        return self.time_led_off

# LED control class
class Led_ctrl:
    def __init__(self, num_pin):
        self.num_pin = num_pin
        self.t0 = machine.Timer(0)
        self.pin = machine.Pin(self.num_pin, machine.Pin.OUT)
        self.pin.value(0)
        self.is_working = False

    def pin_toggle(self, t):
        cur_data = self.pin.value()
        self.pin.value(not cur_data)

    def set_led_on(self): 
        if self.is_working == False:
            self.t0.init(period=1300, mode = machine.Timer.PERIODIC, callback = self.pin_toggle)
        self.is_working = True
        
    def set_led_off(self): 
        if self.is_working == True:
            self.t0.deinit()
            self.pin.value(0)

        self.is_working = False

# main program
led = Led_ctrl(19)
wifi = connect_wifi(SSID_NAME, SSID_PASS)

if not wifi:
    sys.exit(0)

time = Time_ctrl()

lat, lng = get_geocode('set_your_zipcode')

#confirm initialize result
print('test start')
led.set_led_on()
utime.sleep(15)
print('test end')
led.set_led_off()

while True:
    if time.is_time_update(): 
        time.update_date()
        year, month, day = time.get_date()
        print(str(year) + '/' + str(month) + '/' + str(day))

        time_sunrise, time_sunset = get_time_rise_set(year, month, day, lat, lng)

        time.update_time_led_off(time_sunrise) 
        print(time.get_time_led_off())

        time.update_time_led_on(time_sunset)
        print(time.get_time_led_on())
    
    if utime.time() < time.get_time_led_off():
        led.set_led_on()

    if time.get_time_led_on() > utime.time() and utime.time() >= time.get_time_led_off():
        led.set_led_off()

    if utime.time() >= time.get_time_led_on():
        led.set_led_on()
    
    utime.sleep(60)     
