icon: neutu.icns neu3.icns

neutu.icns: neutu.png
	sips -s format icns $< --out $@

neutu.png: neutu_512.png
	sips -Z 256 $< --out $@

neu3.icns: neu3.png
	sips -s format icns $< --out $@

neu3.png: neu3_512.png
	sips -Z 256 $< --out $@
