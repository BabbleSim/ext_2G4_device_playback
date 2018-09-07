/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BS_PLAYBACK_ARGS_H
#define BS_PLAYBACK_ARGS_H

#include "bs_types.h"
#include "bs_cmd_line.h"
#include "bs_cmd_line_typical.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
  BS_BASIC_DEVICE_OPTIONS_FIELDS
  bool txoff;
  bool rxoff;
  bool rssioff;
  char* inputf;
} playback_args_t;

void bs_playback_argsparse(int argc, char *argv[], playback_args_t *args);

#ifdef __cplusplus
}
#endif

#endif
