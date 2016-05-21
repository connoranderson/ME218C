def angleConversion(roll):
	#First scale the number, convert to integer form.
	Bucket = ((roll+1)*16)
	Bucket = round(Bucket)

	#Make sure that everything stays within the correct bounds.
	if(Bucket>16):
		Bucket = 16
	elif(Bucket<0):
		Bucket = 0


	print("The bucket number is: "+str(Bucket))

	temp = Bucket

	if(temp/8 > 1):
		#set GPIO bit 3
		temp -= 8
	else:
		#clear GPIO bit 3

	if(temp/4 > 1):
		#set GPIO bit 2
		temp-=4
	else:
		#clear GPIO bit 2

	if(temp/2 > 1):
		#set GPIO bit 1
		temp-=2
	else:
		#clear GPIO bit 1

	if(temp > 1):
		#set GPIO bit 0
	else:
		#clear GPIO bit 0