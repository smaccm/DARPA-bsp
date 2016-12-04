CURRENT_DIR := $(dir $(abspath $(lastword ${MAKEFILE_LIST})))

include TimeServerNVTMR/TimeServerNVTMR.mk

