# Copyright 2018 Oticon A/S
# SPDX-License-Identifier: Apache-2.0

BSIM_BASE_PATH?=$(abspath ../ )
include ${BSIM_BASE_PATH}/common/pre.make.inc

2G4_libPhyComv1_COMP_PATH?=$(abspath ${BSIM_COMPONENTS_PATH}/ext_2G4_libPhyComv1)

EXE_NAME:=bs_device_2G4_playback
SRCS:=src/bs_playback_main.c \
      src/bs_playback_args.c

A_LIBS:=${BSIM_LIBS_DIR}/libUtilv1.a \
        ${BSIM_LIBS_DIR}/libPhyComv1.a \
        ${BSIM_LIBS_DIR}/lib2G4PhyComv1.a

INCLUDES:=-I${libUtilv1_COMP_PATH}/src/ \
          -I${libPhyComv1_COMP_PATH}/src/ \
          -I${2G4_libPhyComv1_COMP_PATH}/src

SO_LIBS:=
DEBUG:=-g
OPT:=
ARCH:=
WARNINGS:=-Wall -pedantic
COVERAGE:=
CFLAGS:=${ARCH} ${DEBUG} ${OPT} ${WARNINGS} -MMD -MP -std=c99 ${INCLUDES}
LDFLAGS:=${ARCH} ${COVERAGE}
CPPFLAGS:=

include ${BSIM_BASE_PATH}/common/make.device.inc
