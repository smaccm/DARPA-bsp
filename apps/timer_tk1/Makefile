#
# Copyright 2014, NICTA
#
# This software may be distributed and modified according to the terms of
# the GNU General Public License version 2. Note that NO WARRANTY is provided.
# See "LICENSE_GPLv2.txt" for details.
#
# @TAG(NICTA_GPL)
#

TARGETS := $(notdir ${SOURCE_DIR}).cdl
ADL := timer.camkes

# Subdirectory containing extra templates.
TEMPLATES := templates/global-templates templates/darpa-templates

include TimeServerTK1/TimeServer.mk
include TimeClient/TimeClient.mk

include ${PWD}/tools/camkes/camkes.mk
