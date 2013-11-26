# lmbox Tcl core
#
#  The contents of this file are subject to the Mozilla Public License
#  Version 1.1 (the "License"); you may not use this file except in
#  compliance with the License. You may obtain a copy of the License at
#  http://mozilla.org/.
#
#  Software distributed under the License is distributed on an "AS IS"
#  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
#  the License for the specific language governing rights and limitations
#  under the License.
#
#  Author Vlad Seryakov vlad@crystalballinc.com

lmbox_register_init camera_init

proc camera_init {} {

    lmbox_register_menu Personal "Live Cameras" "Watch live security camera streams" camera.jpg { lmbox setpage camerapage } 660
}

proc camera_setup { { type "" } } {

    if { [lmbox get camera_norm count] == 0 } {
      lmbox call camera_norm additem NTSC
      lmbox call camera_norm additem NTSC-M
      lmbox call camera_norm additem NTSC-M-JP
      lmbox call camera_norm additem PAL
      lmbox call camera_norm additem PAL-B
      lmbox call camera_norm additem PAL-B1
      lmbox call camera_norm additem PAL-G
      lmbox call camera_norm additem PAL-H
      lmbox call camera_norm additem PAL-I
      lmbox call camera_norm additem PAL-D
      lmbox call camera_norm additem PAL-D1
      lmbox call camera_norm additem PAL-K
      lmbox call camera_norm additem PAL-M
      lmbox call camera_norm additem PAL-N
      lmbox call camera_norm additem PAL-Nc
      lmbox call camera_norm additem PAL-60
      lmbox call camera_norm additem SECAM-B
      lmbox call camera_norm additem SECAM-D
      lmbox call camera_norm additem SECAM-G
      lmbox call camera_norm additem SECAM-H
      lmbox call camera_norm additem SECAM-K
      lmbox call camera_norm additem SECAM-K1
      lmbox call camera_norm additem SECAM-L
      lmbox call camera_norm additem ATSC-8-VSB
      lmbox call camera_norm additem ATSC-16-VSB
      for { set i 0 } { $i < 10 } { incr i } {
        lmbox call camera_input additem $i
      }
      lmbox call camera_device additem None
      for { set i 0 } { $i < 10 } { incr i } {
        lmbox call camera_device additemdata /dev/video$i /dev/video$i
      }
    }

    set camera_devices [lmbox config camera_devices]
    switch -- $type {
     mosaic {
       set idx 0
       foreach { name url } $camera_devices {
         if { [incr idx] > 4 } { break }
         lmbox set cameraview$idx data $url
         lmbox set cameraview$idx caption $name
         set x [expr [lmbox get cameraview$idx x]+3]
         set y [expr [lmbox get cameraview$idx y]+3]
         camera_play player$idx $url "$x $y 200 160" slow2
       }
     }

     default {
       lmbox call cameralist clear
       foreach { name url } $camera_devices {
         lmbox call cameralist additemdata $name $url
       }
     }
    }
}

proc camera_set { player mrl } {

    lmbox call $player clear
    lmbox set $player repeat 1
    if { [regexp {^([^\|]*)\|([^\|]*)\|([^\|]*)\|(.*)} $mrl d device input norm url] } {
      if { $device != "" } {
        lmbox set tv device $device norm $norm input $input channel [lmbox config tv_channel_default 3]
        switch -glob -- [lmbox get tv cardtype] {
         *PVR* { set url pvr:/!!0  }
         default { set url v4l:/ }
        }
      }
      lmbox call $player add $url
    }
}

proc camera_play { player mrl { area "0 0 0 0" } { speed normal } } {

    lmbox call $player stop
    eval lmbox call $player move $area
    camera_set $player $mrl
    lmbox call $player play
    lmbox set $player speed $speed
}

proc camera_stop { { all "" } } {

    lmbox_stop

    if { $all != "" } {
      for { set i 1 } { $i < 4 } { incr i } {
        lmbox call player$i stop
      }
    }
}

proc camera_pause {} {

    lmbox call player pause
}

# Go to edit page to add new camera
proc camera_add {} {

    lmbox set camera_name text ""
    lmbox set camera_url text ""
    lmbox set camera_device selectedindex 0
    lmbox set status caption "Camera added"
    lmbox setpage cameraeditpage
}

# Show camera details in the edit page
proc camera_edit {} {

    set camera [lmbox get cameralist selecteddata]
    lmbox set camera_device selectedindex 0
    lmbox set camera_norm selectedindex 0
    lmbox set camera_input selectedindex 0
    lmbox set camera_name text [lmbox get cameralist selectedtext]
    lmbox set camera_url text ""

    if { [regexp {^([^\|]*)\|([^\|]*)\|([^\|]*)\|(.*)} $camera d device input norm url] } {
      lmbox set camera_device selectedindex [lmbox call camera_device searchitemdata $device]
      lmbox set camera_norm selectedindex [lmbox call camera_norm searchitem $norm]
      lmbox set camera_input selectedindex [lmbox call camera_norm searchitem $input]
      lmbox set camera_url text $url
    }
    lmbox setpage cameraeditpage
}

# Save camera in the config
proc camera_save {} {

    set camera_name [lmbox get camera_name text]
    set camera_device [lmbox get camera_device selecteddata]
    set camera_input [lmbox get camera_input selectedtext]
    set camera_norm [lmbox get camera_norm selectedtext]
    set camera_url [lmbox get camera_url text]
    if { $camera_name == "" } {
      lmbox set status caption "Camera name should be specified"
      sound_error
      return
    }
    if { $camera_device == "" && $camera_url == "" } {
      lmbox set status caption "Camera device or url should be specified"
      sound_error
      return
    }
    set camera_devices [lmbox config camera_devices]
    set camera_url "$camera_device|$camera_input|$camera_norm|$camera_url"
    lappend camera_devices $camera_name $camera_url
    lmbox_setconfig camera_devices $camera_devices
    lmbox set status caption "Camera info saved"
    lmbox setpage camerapage
}

# Remove camera from the list
proc camera_delete {} {

    set camera_devices ""
    set camera_name [lmbox get cameralist selectedtext]
    foreach { name url } [lmbox config camera_devices] {
      if { $camera_name != $name } {
        lappend camera_devices $name $url
      }
    }
    lmbox_setconfig camera_devices $camera_devices
    lmbox setpage camerapage
}

# Enable live feed from the camera
proc camera_test {} {

    set camera_device [lmbox get camera_device selecteddata]
    set camera_input [lmbox get camera_input selectedtext]
    set camera_norm [lmbox get camera_norm selectedtext]
    set camera_url [lmbox get camera_url text]
    set camera_url "$camera_device|$camera_input|$camera_norm|$camera_url"
    camera_play player $camera_url [lmbox get cameraview area]
}

proc camera_info {} {

    set camera_device [lmbox get camera_device selecteddata]
    if { $camera_device != "" } {
      lmbox set tv device $camera_device
      lmbox_alarm "Video Device $camera_device Info" [lmbox get tv info]
    }
}
