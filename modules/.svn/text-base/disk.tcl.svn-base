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

# This mount assumes that /etc/fstab has entry for dvd device with option user whcih
# will allow to mount DVD by ordinary user

# Returns 1 if disk is present in the DVD tray
proc disk_ready {} {

    switch -- [lmbox cdromstatus] {
     error - unknown - empty { return 0 }
     default { return 1 }
    }
}

proc disk_mount {} {

    set dvd [lmbox config dvd_device /dev/dvd]
    if { [disk_ready] && [catch { exec mount $dvd } errmsg] } {
      # Ignore some errors
      if { [regexp {already mounted} $errmsg] } { return }
      # Fatal errors
      lmbox puts "disk_mount: $dvd: $errmsg"
      return -1
    }
}

proc disk_umount {} {

    set dvd [lmbox config dvd_device /dev/dvd]
    if { [catch { exec umount $dvd } errmsg] } {
      # Ignore some errors
      if { [regexp {not mounted} $errmsg] } { return }
      # Fatal errors
      lmbox puts "disk_umount: $dvd: $errmsg"
    }
}

proc disk_open {} {

    disk_umount
    lmbox cdromeject
}

proc disk_close {} {

    lmbox cdromclose
}

proc disk_play {} {

    lmbox set status caption "Determinning media type..."
    lmbox refresh
    lmbox_stop
    disk_umount
    # Continue with playing video
    if { [disk_mount] == -1 && [lmbox get player position 0] > 0 } {
      lmbox setpage videopage
      return
    }
    # Disk present, determine media type
    set dvd_type ""
    set dvd_path [lmbox config dvd_path /mnt/dvd]
    set dvd_files [lmbox_files $dvd_path]

    if { [regexp -nocase video_ts $dvd_files] } {
      set dvd_type dvd
    } elseif { [regexp -nocase mpegav $dvd_files] } {
      set dvd_type vcd
    } else {
      foreach file $dvd_files {
        switch -- [string tolower [file extension $file]] {
         .mp3 - .wma - .wav { set dvd_type mp3 }
         .avi - .mpg { set dvd_type avi }
        }
      }
    }
    switch -- $dvd_type {
     dvd {
       lmbox set status caption "Playing DVD disk"
       lmbox refresh
       disk_umount
       player_exec dvd://
     }
     vcd {
       lmbox set status caption "Playing Video CD disk"
       lmbox refresh
       disk_umount
       player_exec vcd://
     }
     mp3 {
      lmbox set status caption "Playing music files"
      music_setup $dvd_path
      lmbox setpage musicpage
      return
     }
     avi {
      lmbox set status caption "Playing movie files"
      movie_setup $dvd_path
      lmbox setpage moviepage
     }
     default {
      lmbox set status caption "Playing CD disk"
      lmbox refresh
      # Put all tacks into playlist
      lmbox call music clear
      lmbox call music autoplay CD
      lmbox call music play
     }
    }
}
