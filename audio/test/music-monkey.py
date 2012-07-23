#!/usr/bin/env monkeyrunner
# Imports the monkeyrunner modules used by this program
from com.android.monkeyrunner import MonkeyRunner, MonkeyDevice
import time
import random

# Connects to the current device, returning a MonkeyDevice object
device = MonkeyRunner.waitForConnection()

# sets a variable with the package's internal name
package = 'com.android.music'

# sets a variable with the name of an Activity in the package
activity = 'com.android.music.MediaPlaybackActivity'

# sets the name of the component to start
runComponent = package + '/' + activity

# Runs the component
device.startActivity(component=runComponent)

def play():
	print "push play"
	device.press('KEYCODE_MEDIA_PLAY','DOWN_AND_UP')

def pause():
	print "push pause"
	device.press('KEYCODE_MEDIA_PAUSE', 'DOWN_AND_UP')

def skip_next(n):
	print "skip the next", n, "tracks"
	for x in range(n):
		device.press('KEYCODE_MEDIA_NEXT', 'DOWN_AND_UP')

def skip_previous(n):
        print "skip backwards", n, "tracks"
        for x in range(n):
                device.press('KEYCODE_MEDIA_PREVIOUS', 'DOWN_AND_UP')

def ff(n):
	print "fast forward for", n, "seconds"
	if n < 1:
		device.press('KEYCODE_MEDIA_FAST_FORWARD', 'DOWN_AND_UP')
	else:
		device.press('KEYCODE_MEDIA_FAST_FORWARD', 'DOWN')
		time.sleep(n)
		device.press('KEYCODE_MEDIA_FAST_FORWARD', 'UP')

def rw(n):
        print "rewind for", n, "seconds"
        if n < 1:
                device.press('KEYCODE_MEDIA_REWIND', 'DOWN_AND_UP')
        else:
                device.press('KEYCODE_MEDIA_REWIND', 'DOWN') 
                time.sleep(n)
                device.press('KEYCODE_MEDIA_REWIND', 'UP')



## main flow starts here
actions = ['play', 'pause', 'skip_next', 'skip_prev', 'ff', 'rw', 'wait']

start_time = time.time()
run_time = 60 * 15
end_time = start_time + run_time

while True:
	action_id = random.randint(1, 10)
	action_time = random.randint(1, 15)
	if action_id == 1:
		play()
	elif action_id == 2:
		pause()
	elif action_id == 3:
		skip_next(action_time)
	elif action_id == 4:
		skip_previous(action_time)
	elif action_id == 5:
		pass
		#ff(action_time)
	elif action_id == 6:
		pass
		#rw(action_time)
	elif action_id == 7:
		time.sleep(action_time)
	else:
		pass

	time.sleep(1)

	if time.time() > end_time:
		break

pause()
