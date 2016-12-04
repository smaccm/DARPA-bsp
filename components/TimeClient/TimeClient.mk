CURRENT_DIR := $(dir $(abspath $(lastword ${MAKEFILE_LIST})))

TimeClient_CFILES := $(wildcard ${CURRENT_DIR}/src/*.c)

