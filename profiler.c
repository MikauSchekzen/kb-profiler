void CKbdclassHook::Hook(void){
	UNICODE_STRING usKbdClassDriverName;

	RtlInitUnicodeString(&usKbdClassDriverName, m_wsClassDrvName.c_str());

  //Get pointer to class driver object
	NTSTATUS lStatus = ObReferenceObjectByName
			(&usKbdClassDriverName, OBJ_CASE_INSENSITIVE,
		 	NULL,
		  	0,
		   	(POBJECT_TYPE)IoDriverObjectType,
		   	KernelMode,
		   	NULL,
		   	(PVOID*)&m_pClassDriver);

		if (!NT_SUCCESS(lStatus)){
			throw(std::exception("[KBHookLib]
				Cannot get driver object by name."));
		}

	KIRQL oldIRQL;

	KeRaiseIrql(HIGH_LEVEL, &oldIRQL);

  //IRP_MJ_READ patching
	m_pOriginalDispatchRead = m_pClassDriver->MajorFunction[IRP_MJ_READ];
	m_pClassDriver->MajorFunction[IRP_MJ_READ] = m_pHookCallback;

	m_bEnabled = true;

	KeLowerIrql(oldIRQL);

	return;
}
