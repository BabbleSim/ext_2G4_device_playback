This devices plays back the activity of another device as it was recorded by the
Phy in a previous simulation

Note: This device is fed with the old/v1 2G4 phy dump format.
If you want to use the v2 format, check ext_2G4_device_playbackv2

It takes as command line arguments:
  -inputf=<inputfile>: Path and begining of the dump files names to be played
                       back. For example, the Tx file will be <inputfile>.Tx.csv
  -txoff : Do not send Tx requests
  -rxoff : Do not send Rx requests
  -rssioff : Do not send RSSI requests
