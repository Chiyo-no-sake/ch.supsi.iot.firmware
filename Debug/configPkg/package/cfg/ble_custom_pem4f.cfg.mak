# invoke SourceDir generated makefile for ble_custom.pem4f
ble_custom.pem4f: .libraries,ble_custom.pem4f
.libraries,ble_custom.pem4f: package/cfg/ble_custom_pem4f.xdl
	$(MAKE) -f D:\Documenti\University\IoT\ble_custom/src/makefile.libs

clean::
	$(MAKE) -f D:\Documenti\University\IoT\ble_custom/src/makefile.libs clean

