#Pre-build
#Build neurolabi
NEUROLABI_DIR = $${PWD}/..
CONFIG(debug, debug|release) {
    equals(SANITIZE_BUILD, "address") {
      TargetFile = $${NEUROLABI_DIR}/c/lib/libneurolabi_sanitize.a
    } else {
      TargetFile = $${NEUROLABI_DIR}/c/lib/libneurolabi_debug.a
    }
} else {
    TargetFile  = $${NEUROLABI_DIR}/c/lib/libneurolabi.a
}

exists($${CONDA_ENV}) {
  CONDA_CONFIG = "CONDA_ENV=$${CONDA_ENV}"
}

message("Config: " $${CONDA_CONFIG})

neurolabi.target = neurolabi
CONFIG(debug, debug|release) {
    equals(SANITIZE_BUILD, "address") {
      neurolabi.commands = echo "building neurolabi"; cd $${PWD}/../; ./update_library --sanitize "'$${CONDA_CONFIG}'"
    } else {
      neurolabi.commands = echo "building neurolabi"; cd $${PWD}/../; ./update_library "'$${CONDA_CONFIG}'"
    }
#make lib VERSION=
} else {
    neurolabi.commands = echo "building neurolabi"; cd $${PWD}/../; ./update_library --release "'$${CONDA_CONFIG}'"
}

neurolabi.depends = FORCE
QMAKE_EXTRA_TARGETS += neurolabi

message($${neurolabi.commands})

macx {
  BIN_FOLDER = $${OUT_PWD}/$${app_name}.app/Contents/MacOS
} else {
  BIN_FOLDER = $${OUT_PWD}
}

CONFIG_SOURCE = config.xml
CONFIG(neu3) | CONFIG(flyem) {
  CONFIG_SOURCE = config_flyem.xml
} else {
  CONFIG(biocytin) {
    CONFIG_SOURCE = biocytin_config.xml
  }
}

xmlconfig.target = $${BIN_FOLDER}/config.xml
xmlconfig.depends = $${PWD}/$${CONFIG_SOURCE}
xmlconfig.commands = cp $${PWD}/$${CONFIG_SOURCE} $$xmlconfig.target

jsonconfig.target = jsonconfig
jsonconfig.depends = FORCE
jsonconfig.commands = cp -r $${PWD}/../json $${BIN_FOLDER}

splashconfig.target = splash
splashconfig.depends = FORCE
splashconfig.commands = cp -r $${PWD}/images/neutu_splash.png $${BIN_FOLDER}

app_config.target = app_config
app_config.depends = $$xmlconfig.target $$jsonconfig.target $$splashconfig.target

QMAKE_EXTRA_TARGETS += xmlconfig jsonconfig splashconfig app_config

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

unix {
  OutputDir = $${OUT_PWD}
  macx {
    OutputDir = $${OutputDir}/$${app_name}.app/Contents/MacOS
    exists($${CONDA_ENV}) {
#      QMAKE_POST_LINK += install_name_tool -add_rpath $${CONDA_ENV}/lib $${OutputDir}/$$TARGET
    }
  }


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
}
