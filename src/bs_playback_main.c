/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bs_tracing.h"
#include "bs_oswrap.h"
#include "bs_pc_2G4.h"
#include "bs_pc_2G4_types.h"
#include "bs_pc_2G4_utils.h"
#include "bs_playback_args.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

/**
 * This devices plays back the activity of another device as it was recorded
 * by the phy in a previous simulation
 * (Note that powers and antenna gains will have minor roundings)
 */

static FILE* tx_f= NULL, *rx_f= NULL, *RSSI_f = NULL;

static p2G4_tx_t tx_s;
uint8_t *tx_packet = NULL;
static p2G4_rssi_t RSSI_s;
static p2G4_rx_t rx_s;

static void open_one_input_file(const char* inputf, const char *type,
                                char *filename, FILE **file){
  sprintf(filename,"%s.%s.csv",inputf, type);
  *file = bs_fopen(filename, "r");
  bs_skipline(*file); //skip heading
  if ( feof(*file) ){
    bs_trace_raw(3,"%s file %s is empty => will not use it\n",type, filename);
    fclose(*file);
    *file = NULL;
  }
}

static void open_input_files(const char* inputf, playback_args_t* args){
  char *filename;
  filename = bs_calloc(strlen(inputf) + 10,sizeof(char));

  if ( args->rxoff == false ) {
    open_one_input_file(inputf, "Rx", filename, &rx_f);
  }
  if ( args->txoff == false ) {
    open_one_input_file(inputf, "Tx", filename, &tx_f);
  }
  if ( args->rssioff == false ) {
    open_one_input_file(inputf, "RSSI", filename, &RSSI_f);
  }

  free(filename);

  if ((RSSI_f == NULL) && (tx_f == NULL) && (rx_f == NULL)) {
    bs_trace_warning_line("No input in any of the files??\n");
  }
}

void close_input_files(){
  if (tx_f !=NULL)
    fclose(tx_f);
  if (rx_f !=NULL)
    fclose(rx_f);
  if (RSSI_f !=NULL)
    fclose(RSSI_f);
}

void read_next_tx(){

  if (!tx_f) {
    ;
  } else if (feof(tx_f)) {
    fclose(tx_f);
    tx_f = NULL;
  } else {
    int read;
    double center_freq = 0;
    double power = 0;
    p2G4_freq_t freq;
    p2G4_tx_t *txs = &tx_s;
    read = fscanf(tx_f,
        "%"SCNtime",%"SCNtime",\
        %lf,\
        0x%X,%"SCNu16",\
        %lf,\
        %"SCNtime",%"SCNtime",\
        %"SCNu16",",
        &txs->start_time,
        &txs->end_time,

        &center_freq,

        &txs->phy_address,
        &txs->radio_params.modulation,

        &power,

        &txs->abort.abort_time,
        &txs->abort.recheck_time,

        &txs->packet_size);

    p2G4_freq_from_d(center_freq, 0, &freq);

    txs->radio_params.center_freq = freq;
    txs->power_level = p2G4_power_from_d(power);

    if (read < 9) {
      if ((read > 0) || !feof(tx_f)) //otherwise it was probably an empty trailing line
        bs_trace_warning_line("Corrupted input Tx file disabling it\n");
      fclose(tx_f);
      tx_f = NULL;
    } else {
      if (txs->packet_size > 0) {
        char buffer[txs->packet_size*3 + 1];
        if ( tx_packet != NULL ) free(tx_packet);
        tx_packet = bs_calloc(txs->packet_size, sizeof(char));
        bs_readline(buffer, txs->packet_size*3 + 1, tx_f);
        bs_read_hex_dump(buffer,tx_packet, txs->packet_size);
      } else {
        bs_skipline(tx_f); //skip \n
      }
    }
  }

  if (tx_f == NULL){
    tx_s.start_time = TIME_NEVER;
  }
}

void read_next_rx(){

  if ( !rx_f ){
    ;
  } else if (feof(rx_f)) {
    fclose(rx_f);
    rx_f = NULL;
  } else {
    int read;
    double center_freq = 0;
    double ant_gain = 0;
    p2G4_freq_t freq;
    p2G4_rx_t *Req = &rx_s;
    read = fscanf(rx_f,
        "%"SCNtime",%"SCNu32",%"SCNx32",%"SCNu16",\
        %lf,%lf,"
        "%"SCNu16",%"SCNu16"\
        ,%"SCNu16",\
        %"SCNu16",%"SCNu32",%"SCNtime",%"SCNtime"",
        &Req->start_time,
        &Req->scan_duration,
        &Req->phy_address,
        &Req->radio_params.modulation,

        &center_freq,
        &ant_gain,

        &Req->sync_threshold,
        &Req->header_threshold,

        &Req->pream_and_addr_duration,

        &Req->header_duration,
        &Req->bps,
        &Req->abort.abort_time,
        &Req->abort.recheck_time
        );

    p2G4_freq_from_d(center_freq, 0, &freq);

    Req->radio_params.center_freq = freq;
    Req->antenna_gain = p2G4_power_from_d(ant_gain);

    if ( read < 12 ){
      if ( ( read > 0 ) || !feof(rx_f) ) //otherwise it was probably an empty trailing line
        bs_trace_warning_line("Corrupted input Rx file disabling it (%i %i)\n",read, feof(rx_f));
      fclose(rx_f);
      rx_f = NULL;
    } else {
      bs_skipline(rx_f); //skip remainder of the line
    }
  }

  if (rx_f == NULL){
    rx_s.start_time = TIME_NEVER;
  }
}

void read_next_RSSI() {

  if (!RSSI_f){
    ;
  } else if (feof(RSSI_f)) {
    fclose(RSSI_f);
    RSSI_f = NULL;
  } else {
    int read;
    double center_freq = 0;
    double ant_gain = 0;
    p2G4_rssi_t *Req = &RSSI_s;
    p2G4_freq_t freq;
    read = fscanf(RSSI_f,
        "%"SCNtime","
        "%"SCNu16","
        "%lf,"
        "%lf",
        &Req->meas_time,
        &Req->radio_params.modulation,
        &center_freq,
        &ant_gain
        );

    p2G4_freq_from_d(center_freq, 0, &freq);

    Req->radio_params.center_freq = freq;
    Req->antenna_gain = p2G4_power_from_d(ant_gain);

    if ( read < 3 ){
      if ( ( read > 0 ) || !feof(RSSI_f) ) //otherwise it was probably an empty trailing line
        bs_trace_warning_line("Corrupted input RSSI file disabling it\n");
      fclose(RSSI_f);
      RSSI_f = NULL;
    } else {
      bs_skipline(RSSI_f); //skip remainder of the line
    }
  }

  if (RSSI_f == NULL){
    RSSI_s.meas_time = TIME_NEVER;
  }
}

int main(int argc, char *argv[]) {
  playback_args_t args;
  uint8_t *packet_ptr;

  bs_playback_argsparse(argc, argv, &args);

  open_input_files(args.inputf, &args);

  p2G4_dev_initcom_c(args.device_nbr, args.s_id, args.p_id, NULL);

  read_next_tx();
  read_next_rx();
  read_next_RSSI();

  while ((tx_f !=  NULL) || (rx_f!= NULL) || (RSSI_f != NULL)) {
    int result = -1;
    if ((tx_f !=  NULL) &&
        (tx_s.start_time < rx_s.start_time) &&
        (tx_s.start_time < RSSI_s.meas_time)) {
      p2G4_tx_done_t tx_done_s;
      result =  p2G4_dev_req_tx_c_b(&tx_s, tx_packet, &tx_done_s);
      read_next_tx();
    } else if ((rx_f !=  NULL) &&
              (rx_s.start_time < tx_s.start_time) &&
              (rx_s.start_time < RSSI_s.meas_time)) {
      p2G4_rx_done_t rx_done_s;
      packet_ptr = NULL;
      result = p2G4_dev_req_rx_c_b(&rx_s, &rx_done_s, &packet_ptr, 0, NULL);
      free(packet_ptr);
      read_next_rx();
    } else if ((RSSI_f !=  NULL) &&
              (RSSI_s.meas_time < rx_s.start_time) &&
              (RSSI_s.meas_time < tx_s.start_time)) {
      p2G4_rssi_done_t RSSI_done_s;
      result = p2G4_dev_req_RSSI_c_b(&RSSI_s, &RSSI_done_s);
      read_next_RSSI();
    }
    if (result == -1) {
      bs_trace_raw(3,"We have been disconnected\n");
      break;
    }
  }

  close_input_files();
  if (tx_packet != NULL) free(tx_packet);

  p2G4_dev_disconnect_c();
  return 0;
}
