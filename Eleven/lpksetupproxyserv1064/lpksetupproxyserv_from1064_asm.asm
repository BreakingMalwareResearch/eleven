.code
extern mProcs:QWORD
DllCanUnloadNow_wrapper proc
	jmp mProcs[0*8]
DllCanUnloadNow_wrapper endp
DllGetClassObject_wrapper proc
	jmp mProcs[1*8]
DllGetClassObject_wrapper endp
end
