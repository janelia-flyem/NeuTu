#Pre-build
#Build neurolabi
NEUROLABI_DIR = $${PWD}/..
CONFIG(debug, debug|release) {
    TargetFile = $${NEUROLABI_DIR}/c/lib/libneurolabi_debug.a
} else {
    TargetFile  = $${NEUROLABI_DIR}/c/lib/libneurolabi.a
}

neurolabi.target = neurolabi
CONFIG(debug, debug|release) {
    neurolabi.commands = echo "building neurolabi"; cd $${PWD}/../; ./update_library
#make lib VERSION=
} else {
    neurolabi.commands = echo "building neurolabi"; cd $${PWD}/../; ./update_library --release
}
neurolabi.depends = FORCE
QMAKE_EXTRA_TARGETS += neurolabi

#May not work in parallel compiling
#PRE_TARGETDEPS = $${TargetFile}

###################

#Post-build
#message("$${PWD} => $${OUT_PWD}")
#CONFIG(flyem) {
#    SourceConfig = $${PWD}/config_flyem.xml
#} else {
#    CONFIG(biocytin) {
#        SourceConfig = $${PWD}/biocytin_config.xml
#    } else {
#        SourceConfig = $${PWD}/config.xml
#    }
#}
#SourceJson = $${PWD}/../json

#unix {
#    OutputDir = $${OUT_PWD}
#    macx {
#        OutputDir = $${OutputDir}/$${TARGET}.app/Contents/MacOS
#    }

#    ConfigTarget.target = ConfigTarget
#    ConfigTarget.commands = echo "copying config"; cp $${SourceConfig} $${OutputDir}/config.xml
#    ConfigTarget.depends = $${SourceConfig}

#    message("qmake target: $${OutputDir}")

#    QMAKE_EXTRA_TARGETS += ConfigTarget
#    QMAKE_POST_LINK += $$quote(echo "making config"; make ConfigTarget;)

##-r %{sourceDir}/../json %{buildDir}/neuTube_d.app/Contents/MacOS/
#    JsonTarget.target = JsonConfig
#    JsonTarget.commands = echo "copying json"; cp -r $${SourceJson} $${JsonTarget.target}
#    JsonTarget.depends = FORCE

#    QMAKE_EXTRA_TARGETS += JsonConfig
#    QMAKE_POST_LINK += $$quote(echo "making json"; make JsonConfig;)
#}




