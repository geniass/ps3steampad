# ps3steampad

Attempt at getting a Steam Controller to work as a gamepad on the PS3.

The reason this doesn't work out of the box is that the controller by default behaves like a keyboard and mouse ("Lizzard Mode" apparently).

Inspired by ps3xpad and the hid-steam Linux driver.

# ps3xpad

Reverse engineering notes

## Outline

1.  PRX startup
2.  Start a PPU thread
3.  init_usb
4.  while running:
        wait on mutex
        for each connected controller, read_input
        check_pad_status
        unblock mutex
5.  on exit start stop_thread
6.  in stop_thread set running=false; join start_thread, stop thread, exit
7.  in start_thread detach all controllers, shutdown_usb, stop thread


### USB
### c_pipe
EP0 control pipe

#### o_pipe (not required)
output pipe
used by write_xpad
not present on steam controller (endpoint 0x01 or 0x02)

used by:
* xpad(w)_set_led
* xpad(w)_set_rumble


#### unit_alloc
called by `xpad_attach` and `get_endpoint_desc` (callbacks)

#### xpad_attach
callback passed to `CellUsbdLddOps`

#### get_endpoint_desc (not required)
appears to be for wireless only
part of usb descriptor callbacks



#### init_usb
*  init 2 mutexes
*  init controller handles to -1
*  `CellUsbdLddOps xpad_ops`
   set name to controller name (from xpad linux driver)
   `cellUsbdRegisterExtraLdd` for each possible controller
*  same for wireless controllers



#### read_input
```
for each connected controller (pad):
  read_input into xpad_data
```

read_input is a function pointer which differs for wired/wireless controllers.

`xpad_read_input` for wired controllers:
```
block ring buffer mutex

# xpadbuf[0] == count
# xpadbuf[1] == size
memcpy ringbuf[2:] to p (new buffer)

cast p to XBOX360_IN_REPORT (HID report)

xpad_read_report

unblock ring buffer mutex
```

`xpad_read_report`:
maps xbox360 controller HID report to virtual gamepad
```
CellPadData data;
# bunch of mapping logic. pretty self-explanatory

// send pad data to virtual pad
cellPadLddDataInsert
```


#### check_pad_status
basically checks that PS3 virtual gamepads are alive
sets LEDs appropriately







# hid_steam
