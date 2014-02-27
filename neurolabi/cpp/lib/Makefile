all: CMakeLists.txt

CMakeLists.txt: CMakeLists.txt.qc
	python ../../python/qcmake.py --qt ../../gui/gui_free.pri --cmake $< --output $@
