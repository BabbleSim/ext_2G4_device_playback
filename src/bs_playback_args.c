/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdlib.h>
#include "bs_playback_args.h"
#include "bs_tracing.h"
#include "bs_oswrap.h"

char executable_name[] = "bs_device_2G4_playback";
void component_print_post_help(){
  fprintf(stdout,"This devices plays back the activity of another device as it was recorded by the\n"
          "phy in a previous simulation\n\n");
}

playback_args_t *args_g;

static void cmd_trace_lvl_found(char * argv, int offset){
  bs_trace_set_level(args_g->verb);
}

static void cmd_gdev_nbr_found(char * argv, int offset){
  bs_trace_set_prefix_dev(args_g->global_device_nbr);
}

/**
 * Check the arguments provided in the command line: set args based on it or defaults,
 * and check they are correct
 */
void bs_playback_argsparse(int argc, char *argv[], playback_args_t *args)
{
  args_g = args;
  bs_args_struct_t args_struct[] = {
      BS_BASIC_DEVICE_2G4_FAKE_OPTIONS_ARG_STRUCT,
      { false, true  , false,"inputf",  "inputfile", 's', (void*)&args->inputf,    NULL, "Path and begining of the dump files names to be played back: <inputfile>.Tx.csv <inputfile>.Rx.csv  <inputfile>.RSSI.csv"},\
      { false, false , true, "txoff",   "txoff",     'b', (void*)&(args->txoff),   NULL, "Dont send Tx requests"},
      { false, false , true, "rxoff",   "rxoff",     'b', (void*)&(args->rxoff),   NULL, "Dont send Rx requests"},
      { false, false , true, "rssioff", "rssioff",   'b', (void*)&(args->rssioff), NULL, "Dont send RSSI requests"},
      ARG_TABLE_ENDMARKER
  };

  bs_args_typical_dev_set_defaults((bs_basic_dev_args_t *)args, args_struct);
  static char default_phy[] ="2G4";

  bs_args_parse_cmd_line(argc, argv, args_struct);

  bs_args_typical_dev_post_check((bs_basic_dev_args_t *)args, args_struct, default_phy);
  if ( args->inputf == NULL ) {
    bs_args_print_switches_help(args_struct);
    bs_trace_error_line("The input file needs to be specified\n");
  }
}
