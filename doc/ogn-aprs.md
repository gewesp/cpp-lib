# Parsing APRS packets from OGN

Packets are lines of ASCII text.  It is OK to limit line length to 200 
characters.

The general format for position reports is:
  `header:/time_position_movement [ specials ... ]` 

The general format for status reports (0.2.5 and later) is:
  `header:>time [ specials ... ]` 

"Specials" are whitespace-separated fields containing OGN-specific
"user data" not specified by APRS.

The parsing strategy to separate the header, `time_position_movement`
and special blocks is:
* Parse up to first `:` to get the header
* If a '/' follows: Parse time_position_movement
* Else, if a '>' follows: Parse time
* Split the rest of the line on whitespace to get the specials.
* Parse the specials individually, taking into account whether
  we're parsing a station or aircraft report.

## The header block

Parsing strategy:
* Parse up to `>` to get ID
* Split the rest on comma into list L
* L must have 3 or 4 elements.
* L[0] must be `APRS`
* L[-2] _(the second-last element)_ must be `qAS` or `qAC`
  If qAC, it's a station report.
  If qAS, it's an aircraft report.
* L[-1] _(the last element)_ is the server name or receiving station ID.

General format:
* Stations: `station_id>APRS,TCPIP*,qAC,server`
* Aircraft: `aircraft_id>APRS,[RELAY*,]qAS,receiving_station_id`

Examples:
* Station, connected via TCPIP to server:  `Letzi>APRS,TCPIP*,qAC,GLIDERN1`
* Aircraft, direct reception by LFLE:    `OGNA4863D>APRS,qAS,LFLE`
* Aircraft, reception by EPLR via relay: `OGNA3863D>APRS,RELAY*,qAS,EPLR`

## The `time_position_movement` block

Parsing strategy: Use `scanf()`

General format:
  `HHMMSS h DDMM.mm {NS} s1 DDDMM.mm {EW} s2 [ course_speed ] s3 A=aaaaaa`

Observed values for s1: `/`, `\` or `I`.
Observed values for s2: `z`, `'`, `&`
Observed values for s3: `/`

Examples:
  `/184222h4730.52NI00807.68E&/A=002132`
  `/123945h4534.11N/00559.49E'146/050/A=002394`

## Specials: OGN-specific "user data", not specified by APRS

`!W15!`
`id07A4863D`
`+000fpm`
`+0.0rot`
`FL004.17`
`3e`
`s6.00`
`h03`
`rDDAE09`

# Examples

## Station reports

### Pre-0.2.5

`Letzi>APRS,TCPIP*,qAC,GLIDERN1:/184222h4730.52NI00807.68E&/A=002132 CPU:1.0 RAM:130.2/1586.4MB NTP:0.5ms/-20.1ppm RF:+48+1.4ppm/-0.1dB`

### 0.2.5 onwards: Status

`LFNC>APRS,TCPIP*,qAC,GLIDERN1:>065238h v0.2.5.ARM CPU:0.1 RAM:1369.5/2121.3MB NTP:0.5ms/-4.2ppm +29.0C RF:+68+1.0ppm/+0.72dB/+11.7dB@10km[4615]`

## Aircraft position reports

```
OGNA4863D>APRS,RELAY*,qAS,EPLR:/101723h5113.10N/02223.56E'000/000/A=000794 !W15! id07A4863D +000fpm +0.0rot FL004.17
OGN113D07>APRS,qAS,Ottawa1:/171237h4521.30N/07547.23W'248/000/A=000226 !W62! id07113D07 +000fpm +0.0rot FL000.00 28.0dB 0e -12.0kHz gps3x5
FLR5102BD>APRS,qAS,LFLE:/123945h4534.11N/00559.49E'146/050/A=002394 !W87! id065102BD -058fpm +0.0rot 25.5dB 0e -2.6kHz gps2x4 s6.00 h03
ICA4B43D0>APRS,qAS,Letzi:/155643h4656.29N/00831.74EX267/128/A=005448 !W05! id0D4B43D0 -474fpm +0.1rot 3.5dB 0e -1.6kHz gps3x4 s6.01 h03 rDDAE09 +9.1dBm
ICA400EDB>APRS,qAS,DarltonN:/132511h5257.41N\00034.58W^240/077/A=000705 !W58! id21400EDB +1030fpm +0.0rot 6.8dB 0e -9.8kHz gps1x2 s6.05 3 +8.6dBm
FLRDD9824>APRS,qAS,whb:/135048h4723.86N/00944.08Ez000/000/A=001410 !W76! id02DD9824 +000fpm +0.0rot 54.8dB 0e -0.9kHz gps4x4 s6.06 h02 r982400
FLRDDAC88>APRS,qAS,Hengsen:/133458h5128.42N/00738.54Ez000/000/A=000443 !W29! id02DDAC88 -019fpm +0.0rot 54.0dB 0e +1.4kHz gps3x4 s6.07 h03 rAC8800
```



## Server response to login

```
# aprsc 2.0.14-g28c5a6a
# logresp 0 unverified, server GLIDERN1
```
or if the client provide a **valid passcode**
```
# aprsc 2.1.2-gc90ee9c
# logresp EDWJ verified, server GLIDERN3
```



## Keepalives

Every 20 seconds:
`# aprsc 2.0.14-g28c5a6a 26 Jul 2014 19:53:39 GMT GLIDERN1 37.187.40.234:14580`

### See also

* [APRS-IS Specifications](http://www.aprs-is.net/Specification.aspx)
* [APRS protocol reference. Version 1.0.1](http://www.aprs.org/doc/APRS101.PDF) 
