BLE Central Explorer
===================

This app scans for advertisements, and connects to a device with the specified
service UUID in its advertisements.

It then discovers and prints all services and characteristics, along with the handle
numbers.

Example
-------

```
Service [1801] - handle: 0x01
  -Characteristic [2a05] - handle: 0x02
  ---Value handle: 0x03
  ---CCC - handle: 0x04
  -Characteristic [2b29] - handle: 0x05
  ---Value handle: 0x06
  -Characteristic [2b2a] - handle: 0x07
  ---Value handle: 0x08

Service [1800] - handle: 0x09
  -Characteristic [2a00] - handle: 0x0a
  ---Value handle: 0x0b
  -Characteristic [2a01] - handle: 0x0c
  ---Value handle: 0x0d
  -Characteristic [2a04] - handle: 0x0e
  ---Value handle: 0x0f
  ---Descriptor [2a04] - handle: 0x0e
```