all: CMakeLists.txt

CMakeLists.txt: CMakeLists.txt.qc ../../gui/gui_free.pri
	python ../../python/qcmake.py --qt ../../gui/gui_free.pri --cmake $< --output $@
