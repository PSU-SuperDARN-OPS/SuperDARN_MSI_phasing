

find_package(PkgConfig)
pkg_check_modules(PC_APCI QUIET APCI)

find_path(APCI_INCLUDE_DIR
        NAMES apcilib.h
        PATHS ${PC_APCI_INCLUDE_DIR}
        PATH_SUFFIX APCI
        )

find_library(APCI_LIBRARY
        NAMES APCI
        PATHS ${PC_APCI_LIBRARY_DIRS}
        )

