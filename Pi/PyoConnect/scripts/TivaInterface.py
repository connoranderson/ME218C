scriptId = "MyoTivaInterface"
scriptTitle = "Myo Tiva Interface"
scriptDescription = "Uses GPIO to report the state of the Myo to the Tiva"
scriptDetailsUrl ="MyoTivaInterface"
import RPi.GPIO as GPIO
#import globals

def updateGPIO(pose, roll):
        #globals.GPIO_FORWARD
        #globals.GPIO_BACK
	#first deal with the forward/backward command
	if((pose == "fist") or (pose == "waveIn")): #for robustness...
		GPIO.output(38, True)
		GPIO.output(15, True) # The forward signalling LED
		GPIO.output(40, False)
		GPIO.output(13, False) #back ward signalling LED
	elif((pose == "waveOut") or (pose == "fingersSpread")): # again for robustness
		GPIO.output(40, True)
		GPIO.output(13, True)
		GPIO.output(38, False)
		GPIO.output(15, False)
	else:
		GPIO.output(38, False)
		GPIO.output(40, False)
		GPIO.output(13, False)
		GPIO.output(15, False)

        if((pose == "doubleTap")): #Double tap to reset center. Make sure arm is parallel to ground.
                myo.rotSetCenter()        
		#print("YESSSSS")

        #Output roll

        #if((roll > 0) and (roll<0.5)):
                #GPIO.output(31, False)
                #GPIO.output(33, True)
                #GPIO.output(35, False)
                #GPIO.output(37, False)
       # elif((roll>0.5)):
                #GPIO.output(31, True)
                #GPIO.output(33, False)
               # GPIO.output(35, False)
               # GPIO.output(37, False)
       # elif((roll < 0) and (roll>-0.5)):
                #GPIO.output(31, False)
                #GPIO.output(33, False)
               # GPIO.output(35, True)
               # GPIO.output(37, False)
        #elif((roll<-0.5)):
                #GPIO.output(31, False)
               # GPIO.output(33, False)
               # GPIO.output(35, False)
               # GPIO.output(37, True)

	#First scale the number, convert to integer form.
        #Make sure no control inputs occur when not calibrated/off the hand
        if(pose == "unknown"):
                Bucket = 8
        else:
                Bucket = ((roll+1)*8)

	Bucket = round(Bucket)

	#Make sure that everything stays within the correct bounds.
	if(Bucket>16):
		Bucket = 16
	elif(Bucket<0):
		Bucket = 0

	if(Bucket < 7):
		GPIO.output(11, True)
		GPIO.output(12, False)
	elif(Bucket > 8):
		GPIO.output(11, False)
		GPIO.output(12, True)
	else:
		GPIO.output(11, False)
		GPIO.output(12, False)

	#print("The bucket number is: "+str(Bucket))

	temp = Bucket

	if(temp/8 >= 1):
		GPIO.output(37, True)#set GPIO bit 3
		temp -= 8
	else:
		GPIO.output(37, False)#clear GPIO bit 3

	if(temp/4 >= 1):
		GPIO.output(35, True)#set GPIO bit 2
		temp-=4
	else:
		GPIO.output(35, False)#clear GPIO bit 2

	if(temp/2 >= 1):
		GPIO.output(33, True)#set GPIO bit 1
		temp-=2
	else:
		GPIO.output(33, False)#clear GPIO bit 1

	if(temp >= 1):
		GPIO.output(31, True)#set GPIO bit 0
	else:
		GPIO.output(31, False)#clear GPIO bit 0

                
	#Then convert the roll to analog values for the parallel transfer

 #def onPoseEdge(pose, edge): #shit ain't working, I don't know why either
 #	if((pose == "doubleTap"): #and (edge = "on")):
 #		myo.rotSetCenter() #recalibrate when the user double taps their fingers
 #              myo.unlock("hold")


def onPeriodic():
	roll = myo.rotRoll()
	pose = myo.getPose()
        #print("Pose: " + str(pose) + ", roll: "+str(roll))
	updateGPIO(pose, roll)
	

# def onLock():
# 	myo.unlock("hold")

# def onWear(arm, xdirection):
# 	myo.lock() #let's force the user to unlock once the thing is fully on
# 	#so that it centers properly

#def onUnlock():
	#myo.setLockingPolicy("none") #make sure the thing only locks when taken off
	#myo.rotSetCenter() #centers the roll upon unlock
	#print("centering roll")

# def onUnWear():
#        myo.setLockingPolicy("standard")
# 	myo.lock() #lock it on unWear
# 	updateGPIO("rest",0) #set the GPIO output to neutral
