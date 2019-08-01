This devices plays back the activity of another device as it was recorded by the
Phy in a previous simulation

It takes as command line arguments:
  -inputf=<inputfile>: Path and begining of the dump files names to be played
                       back. For example, the Tx file will be <inputfile>.Tx.csv
  -txoff : Do not send Tx requests
  -rxoff : Do not send Rx requests
  -rssioff : Do not send RSSI requests
