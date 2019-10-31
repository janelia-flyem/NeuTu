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
CONFIG(neu3) | CONFIG(flyem) | CONFIG(neutu) {
  CONFIG_SOURCE = config_flyem.xml
} else {
  CONFIG(biocytin) {
    CONFIG_SOURCE = biocytin_config.xml
  }
}

CONFIG_FOLDER = neutube_config
CONFIG(neu3) {
  CONFIG_FOLDER = neu3_config
} else {
  CONFIG(flyem) | CONFIG(neutu) {
    CONFIG_FOLDER = neutu_config
  } else {
    CONFIG(biocytin) {
      CONFIG_FOLDER = biocytin_config
    }
  }
}

DEFINES += _CONFIG_FOLDER_=\"\\\"$$CONFIG_FOLDER\\\"\"

configfolder.target = $${BIN_FOLDER}/$${CONFIG_FOLDER}
configfolder.commands = mkdir $$configfolder.target

xmlconfig.target = $${BIN_FOLDER}/$${CONFIG_FOLDER}/config.xml
xmlconfig.depends = $${PWD}/$${CONFIG_SOURCE} $$configfolder.target
xmlconfig.commands = cp $${PWD}/$${CONFIG_SOURCE} $$xmlconfig.target

jsonconfig.target = jsonconfig
jsonconfig.depends = FORCE $$configfolder.target
jsonconfig.commands = rm -rf $${BIN_FOLDER}/$${CONFIG_FOLDER}/json; cp -r $${PWD}/../json $${BIN_FOLDER}/$${CONFIG_FOLDER}/json

SPLASH_SOURCE = $${PWD}/images/neutu_splash.png
CONFIG(neu3) {
  SPLASH_SOURCE = $${PWD}/images/neu3_splash.png
}

CONFIG(neu3) {
  splash_file = neu3_splash.png
} else {
  CONFIG(flyem) | CONFIG(neutu) {
    splash_file = neutu_splash.png
  }
}

splashconfig.target = $${BIN_FOLDER}/$${CONFIG_FOLDER}/splash.png
splashconfig.depends = FORCE $$configfolder.target $${PWD}/images/$$splash_file
splashconfig.commands = cp $${PWD}/images/$$splash_file $$splashconfig.target

docconfig.target = docconfig
docconfig.depends = FORCE $$configfolder.target
CONFIG(flyem) | CONFIG(neutu) {
  docconfig.commands = cp -r $${PWD}/doc $${BIN_FOLDER}/$${CONFIG_FOLDER}; cp $${PWD}/doc/flyem_proofread_help.html $${BIN_FOLDER}/$${CONFIG_FOLDER}/doc/shortcut.html
} else {
  docconfig.commands = cp -r $${PWD}/doc $${BIN_FOLDER}/$${CONFIG_FOLDER}
}

app_config.target = app_config
app_config.depends = $$xmlconfig.target $$jsonconfig.target $$splashconfig.target $$docconfig.target

QMAKE_EXTRA_TARGETS += configfolder xmlconfig jsonconfig splashconfig docconfig app_config

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
