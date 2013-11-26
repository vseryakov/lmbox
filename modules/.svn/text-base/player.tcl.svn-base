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


lmbox_register_stop player_stop

proc player_onstart { { player player } } {

    lmbox puts "$player playing: [player_status $player]"
}

proc player_onstop { { player player } } {

    lmbox call $player settimer 0 ""
    lmbox set $player audiofilter ""
}

proc player_onfinish { { player player } } {

    lmbox puts "finished: [player_status $player]"
    lmbox call $player stop
    player_savepos $player 0
}

proc player_status { { player player } } {

    return "[lmbox get $player title] [lmbox get $player artist]"
}

proc player_stop {} {

    player_savepos
    lmbox call player stop
    lmbox varset player_pvr_flag 0
}

proc player_play {} {

    lmbox varset player_pvr_flag 0
    lmbox set player audiofilter ""
    if { [lmbox get player status] != "play" } {
      # Set volume before actual playing
      player_setvolume [lmbox config player_volume 100]
      return [lmbox call player play]
    }
    # Make sure player window is on top
    lmbox call player sync
    return 0
}

proc player_pause {} {

    lmbox call player pause
}

# Setup visual filter for audio player according to config
proc player_audiofilter { { player player } } {

    lmbox set $player audiofilter ""
    switch -- [lmbox config music_effects] {
     goom {
         lmbox call $player move 0 0 0 0
         lmbox set $player audiofilter goom
     }
    }
}

# Minimize player for displaying in the corner
proc player_minimize { { type "" } } {

    if { [lmbox get player status] != "play" } { return }
    switch -- $type {
     init {
       if { [lmbox get player x] == 0 ||
            [lmbox newpage] == "mainpage" } {
         player_minimize
       }
     }

     full {
       lmbox setpage videopage
     }

     topleft {
       lmbox call player move 20 30 140 105
     }

     topright {
       lmbox call player move 450 30 140 105
     }

     bottomright {
       lmbox call player move 450 285 140 105
     }

     bottomleft -
     default {
       lmbox call player move 20 285 140 105
     }
    }
}

proc player_record {} {

    switch -regexp -- [set file [lmbox get player file]] {
     {^v4l:|^pvr:} {
        switch -- [lmbox var player_pvr_flag 0] {
         0 {
           set now [clock seconds]
           set channel [lmbox get tv channel]
           set date [clock format $now -format "%m-%d-%y %H:%M"]
           lmbox call player event pvr:0 "$channel $date"
           player_osdtext 30 30 "Rec: $channel"
           lmbox varset player_pvr_flag 1
         }

         1 {
           # Close current session and set it to no-record state
           player call event v4l:-1 -1 -1 -1
           # Re-open new session
           lmbox call player event v4l:-1 -1 -1 1
           player_osdtext 30 30 "Stop: $channel"
           lmbox varset player_pvr_flag 0
         }
        }
     }
    }
}

proc player_next {} {

    lmbox call player next
}

proc player_prev {} {

    lmbox call player prev
}

proc player_event { event args } {

    switch -regexp -- [set file [lmbox get player file]] {
     {^v4l:|^pvr:} {
        # These events are Next/Prev or CH+/CH- buttons
        switch -glob -- $event {
         next {
            tvguide_channel 1
         }
         prev {
            tvguide_channel -1
         }
         number* {
            tvguide_keys [string index $event 6] 1
         }
         default {
            eval $args
         }
        }
     }

     {^dvd:|^vcd:} {
        # DVD/VCD menu events
        if { [lmbox call player event $event] != "" } {
          eval $args
        }
     }

     default {
        eval $args
     }
    }
}

proc player_controls { name level } {

    lmbox set videotimer enabled 0
    set level [expr [lmbox get player $name]+$level]
    lmbox set player $name $level
    player_osdslider [lmbox get player $name] 65535 "[string totitle $name]:"
}

proc player_avoffset { offset } {

    lmbox set videotimer enabled 0
    set offset [expr [lmbox get player avoffset]+$offset]
    lmbox set player avoffset $offset
    player_osdtext 30 30 "A/V Offset: $offset"
}

proc player_audiochannel { ch } {

    lmbox set videotimer enabled 0
    set ch [expr [lmbox get player audiochannel]+$ch]
    lmbox set player audiochannel $ch
    player_osdtext 30 30 "Audio Ch: $ch"
}

proc player_setvolume { level } {

    set volume_type [lmbox config volume_type amplevel]
    if { [regexp {^[\+\-]} $level] } {
      set level [expr [lmbox get player $volume_type] $level]
    }
    lmbox set player $volume_type $level
    return $level
}

proc player_osdtext { x y text { interval "" } } {

    lmbox call player osd clear
    lmbox call player osdtext $x $y $text
    lmbox call player osd show
    if { $interval > 0 } {
      lmbox set videotimer interval $interval
    }
    lmbox set videotimer enabled 1
}

proc player_osdslider { val max text } {

    lmbox call player osdslider $val $max $text
    lmbox call player osd show
    lmbox set videotimer enabled 1
}

# Additional info for OSD output
proc player_osdinfo {} {

    if { [lmbox var player_pvr_flag 0] } {
      return Recording
    }
}

proc player_info {} {

    lmbox set videotimer enabled 0
    switch -- [lmbox call player osd status] {
     0 {
       lmbox call player osd clear
       lmbox call player osdinfo [player_osdinfo]
       lmbox call player osd show
     }
     1 {
       lmbox call player osd hide
     }
    }
}

proc player_mute {} {

    lmbox set videotimer enabled 0
    set mute_type [lmbox config mute_type ampmute]
    set mute [expr 1 - [lmbox get player $mute_type]]
    lmbox set player $mute_type $mute
    if { $mute } {
      set mute On
      # For some reason xine mute/ampmute functions are not working
      # properly with alsa driver
      lmbox set player amplevel 0
    } else {
      set mute Off
      lmbox set player amplevel [lmbox config player_volume 100]
    }
    player_osdtext 30 30 "Mute: $mute"
}

proc player_seek { pos } {

    lmbox set videotimer enabled 0
    lmbox call player seek $pos
    player_osdslider [lmbox get player position] [lmbox get player length] "Position:"
}

proc player_aspect { level } {

    lmbox set videotimer enabled 0
    set aspect [expr [lmbox get player aspect]+$level]
    lmbox set player aspect $aspect
    player_osdtext 30 30 "Aspect: $aspect"
}

proc player_volume { level } {

    lmbox set videotimer enabled 0
    set volume [player_setvolume +$level]
    lmbox_setconfig player_volume $volume
    player_osdslider $volume 100 "Volume:"
}

proc player_deinterlace {} {

    set filters [lmbox get player deinterlacefilters]
    set index [expr [lmbox var tv_deinterlace_index 0] + 1]
    if { $index >= [expr [llength $filters]/3] } { set index -1 }
    lmbox varset tv_deinterlace_index $index
    lmbox set player videofilter [lindex $filters [expr $index * 3 + 1]]
    player_osdtext 30 30 "Deinterlace: [lmbox_nvl [lindex $filters [expr $index * 3]] None]"
}

proc player_exec { files { id "" } { size "0 0 0 0" } } {

    if { $files == "" } { return }
    set last_pos 0
    set last_time 0
    set last_file ""
    set last_length 0
    set now [clock seconds]
    lmbox call player clear
    foreach file $files {
      # Remove our custom tags from the mrl
      set skip ""
      set filename $file
      regsub {\#pos\:[0-9]+} $file {} file
      regsub {\#title\:[^# ]+} $file {} file
      regsub {\#audio\:[0-9]+} $file {} file
      regsub {\#skip\:[0-9,]+} $file {} file
      # Check for last watched file from the list so we
      # can continue from where we've stopped
      set rec [lmbox dbget system position:$file]
      set file_time [lmbox_nvl [lindex $rec 1] 0]
      set file_length [lmbox_nvl [lindex $rec 2] 0]
      if { $now - $file_time < 86400*7 && $file_time > $last_time } {
        set last_file $file
        set last_time $file_time
        set last_length $file_length
        set last_pos [lmbox_nvl [lindex $rec 0] 0]
      }
      # Positioning support, if one file given seek to that position, otherwise
      # choose the latest one
      if { [regexp {\#pos\:([0-9]+)} $filename d pos] &&
           ([llength $files] == 1 || $file_time > $last_time) } {
        set last_file $file
        set last_pos [expr $pos*1000]
        set last_length 0
      }
      # Check for subtitles file(s)
      foreach ext { srt sub } {
        set sub [file rootname $file].$ext
        if { ![file exists $sub] } { continue }
        append file #subtitle:$sub
        lmbox puts "Using subtitles file $sub"
        break
      }
      # Audio channel
      if { [regexp {\#audio\:([0-9]+)} $filename d audio] } {
        lmbox set player audiochannel $audio
      }
      # Skip positions, list of: start,duration,start,duration ...
      if { [regexp {\#skip\:([0-9,]+)} $filename d skip] && ![lmbox_admin_mode] } {
        lmbox call player addskip $file $skip
      } else {
        lmbox call player add $file
      }
    }
    if { $last_file != "" && $last_pos > 0 } {
      if { $last_length > 0 } {
        lmbox set player ongotfocus "player_osdtext 30 30 {Continue from [expr $last_pos*100/$last_length]%}"
      }
      lmbox puts "player_player: $last_file: continue from $last_pos out of $last_length, stopped on [clock format $last_time]"
      lmbox set player file $last_file
      lmbox set player position $last_pos
    }
    eval lmbox call player move $size
    lmbox set player data $id
    lmbox setpage videopage
}

# Position in the stream as percentage
proc player_pos {} {

    set len [lmbox get player length]
    if { $len > 0 } {
      set pos [lmbox get player position]
      return [expr $pos*100/$len]
    }
    return 0
}

# Save the current position, it will continue from that position
# next time this file will be played
proc player_savepos { { player player } { pos "" } { len "" } } {

    if { [lmbox get $player status] == "play" } {
      set len [lmbox get $player length]
      set pos [lmbox get $player position]
      # If at the end of the file, reset the save position
      #if { $len > 0 && $pos*100/$len >= 99 && [llength [lmbox get player playlist]] == 1 } { set pos 0 }
    }
    if { $pos != "" } {
      set file [lmbox get $player file]
      lmbox set player position $pos
      lmbox puts "player_savepos: $player: $file: position <$pos> timestamp [clock seconds] length <$len>"
      lmbox dbput system position:$file "$pos [clock seconds] $len"
    }
}

# Take a snopshot, in case of movie, save as id.png otherwise
# save as filename.png
proc player_snapshot { { id "" } } {

   set file ""
   if { $id == "" } { set id [lmbox get player data] }
   if { [string is integer -strict $id] } {
     set file [lmbox config cover_path data/Movies]/$id.png
   }
   lmbox puts "player_snapshot: $id: $file"
   lmbox call player snapshot $file
}

