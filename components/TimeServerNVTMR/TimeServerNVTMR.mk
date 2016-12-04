CURRENT_DIR := $(dir $(abspath $(lastword ${MAKEFILE_LIST})))

TimeServerNVTMR_CFILES := $(wildcard ${CURRENT_DIR}/src/*.c)

TimeServerNVTMR_LIBS := platsupport

