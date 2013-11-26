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


lmbox_register_init tvguide_init

proc tvguide_init {} {

    lmbox dbopen tvguide
    lmbox dbopen tvguide_channels
    lmbox dbopen tvguide_programs
    lmbox dbopen tvguide_timestamp -dups
    # Check for tvguide refresh every 30 mins
    lmbox_register_check TVGuide tvguide_checker
    # Check if tvguide database is empty
    if { [lmbox dbstat tvguide count] <= 0 } {
      lmbox schedule "tvguide_checker" -interval 60 -once 1
    }
    # Menu items
    lmbox_register_menu TV "Watch Live TV" "Watch live television" tv.jpg { lmbox setpage tvpage } 600
    lmbox_register_menu TV "Search" "Search TV programs" search.jpg { lmbox setpage tvsearchpage } 610
    lmbox_register_menu TV "Recorded" "Watch recorded TV programs" recorded.jpg { lmbox setpage tvrecpage } 630
    # Config registration
    lmbox_register_config "TV Guide/Tuner Settings" tvguide_config
}

proc tvguide_config {} {

    if { [lmbox get tv_out count] == 0 } {
      lmbox call tv_out additem VGA
      lmbox call tv_out additem S-Video
    }
    if { [lmbox get tv_player count] == 0 } {
      lmbox call tv_player additem PVR
      lmbox call tv_player additem V4L
      lmbox call tv_player additem Other
    }
    if { [lmbox get tv_norm count] == 0 } {
      lmbox call tv_norm additem NTSC
      lmbox call tv_norm additem NTSC-M
      lmbox call tv_norm additem NTSC-M-JP
      lmbox call tv_norm additem PAL
      lmbox call tv_norm additem PAL-B
      lmbox call tv_norm additem PAL-B1
      lmbox call tv_norm additem PAL-G
      lmbox call tv_norm additem PAL-H
      lmbox call tv_norm additem PAL-I
      lmbox call tv_norm additem PAL-D
      lmbox call tv_norm additem PAL-D1
      lmbox call tv_norm additem PAL-K
      lmbox call tv_norm additem PAL-M
      lmbox call tv_norm additem PAL-N
      lmbox call tv_norm additem PAL-Nc
      lmbox call tv_norm additem PAL-60
      lmbox call tv_norm additem SECAM-B
      lmbox call tv_norm additem SECAM-D
      lmbox call tv_norm additem SECAM-G
      lmbox call tv_norm additem SECAM-H
      lmbox call tv_norm additem SECAM-K
      lmbox call tv_norm additem SECAM-K1
      lmbox call tv_norm additem SECAM-L
      lmbox call tv_norm additem ATSC-8-VSB
      lmbox call tv_norm additem ATSC-16-VSB
    }
    if { [lmbox get tv_chanlist count] == 0 } {
      lmbox call tv_chanlist additem us-cable
      lmbox call tv_chanlist additem us-bcast
      lmbox call tv_chanlist additem us-cable-hrc
      lmbox call tv_chanlist additem japan-bcast
      lmbox call tv_chanlist additem japan-cable
      lmbox call tv_chanlist additem europe-west
      lmbox call tv_chanlist additem europe-east
      lmbox call tv_chanlist additem italy
      lmbox call tv_chanlist additem newzealand
      lmbox call tv_chanlist additem australia
      lmbox call tv_chanlist additem ireland
      lmbox call tv_chanlist additem france
      lmbox call tv_chanlist additem china-bcast
      lmbox call tv_chanlist additem southafrica
      lmbox call tv_chanlist additem argentina
      lmbox call tv_chanlist additem canada-cable
      lmbox call tv_chanlist additem australia-optus
    }
    if { [lmbox get tv_deinterlace count] == 0 } {
      lmbox call tv_deinterlace additemdata None ""
      foreach { name type descr } [lmbox get player deinterlacefilters] {
        lmbox call tv_deinterlace additemdata $name $type
        lmbox varset tv_deinterlace:$name $descr
      }
    }
    if { [lmbox get tv_device count] == 0 } {
      for { set i 0 } { $i < 10 } { incr i } {
        lmbox call tv_device additem /dev/video$i
      }
    }
    if { [lmbox get tv_input count] == 0 } {
      for { set i 0 } { $i < 10 } { incr i } {
        lmbox call tv_input additem $i
      }
    }
    if { [lmbox get tv_audio count] == 0 } {
      for { set i -1 } { $i < 10 } { incr i } {
        lmbox call tv_audio additem $i
      }
    }
    lmbox setpage tvconfigpage
}

proc tvguide_move { args } {

    set object [lmbox get this name]
    set num1 [string index $object end-1]
    set num2 [string index $object end]
    set tv_row [lmbox config tv_row 0]
    set tv_nrows [lmbox config tv_rows 4]
    set tv_count [lmbox var tv_count 0]
    set event [lmbox_nvl [lindex $args 0] [lmbox getevent]]

    switch -glob -- $event {
     prevpage {
       if { $tv_row-$tv_nrows >= 0 } {
         incr tv_row -$tv_nrows
         lmbox_setconfig tv_row $tv_row
         tvguide_setup matrix 0
         tvguide_status [lmbox get this data]
       }
     }

     nextpage {
       if { $tv_row+$tv_nrows < $tv_count } {
         incr tv_row $tv_nrows
         lmbox_setconfig tv_row $tv_row
         tvguide_setup matrix 0
         tvguide_status [lmbox get this data]
       }
     }

     up {
       if { $num1 > 0 } {
         lmbox call tvshow[incr num1 -1]$num2 setfocus
       } else {
         if { $tv_row > 1 } {
           incr tv_row -1
           lmbox_setconfig tv_row $tv_row
           tvguide_setup matrix 0
           tvguide_status [lmbox get this data]
         }
       }
     }

     down {
       if { $num1 < $tv_nrows-1 } {
         lmbox call tvshow[incr num1]$num2 setfocus
       } else {
         if { $tv_row < $tv_count } {
           incr tv_row
           lmbox_setconfig tv_row $tv_row
           tvguide_setup matrix 0
           tvguide_status [lmbox get this data]
         }
       }
     }

     next {
       if { $num2 < 3 } {
         lmbox call tvshow$num1[incr num2] setfocus
       } else {
         tvguide_setup matrix 1800
         tvguide_status [lmbox get this data]
       }
     }

     prev {
       if { $num2 > 1 } {
         lmbox call tvshow$num1[incr num2 -1] setfocus
       } else {
         tvguide_setup matrix -1800
         tvguide_status [lmbox get this data]
       }
     }

     accept {
       tvguide_play [lmbox get this data]
     }

     number* {
       tvguide_keys [string index $event 6]
     }

     default {
       return lmbox_ignore
     }
    }
}

# Handle numeric keys pressed during tvguide or tv watch
proc tvguide_keys { key { channel 0 } } {

    set now [clock seconds]
    set tv_keys [lmbox var tv_keys]
    if { $now - [lmbox var tv_keystime 0] > 3 } { set tv_keys "" }
    append tv_keys $key
    if { $channel } {
      tvguide_channel $tv_keys
    } else {
      tvguide_setup matrix 0 [format "%05d" $tv_keys]
    }
    lmbox varset tv_keys $tv_keys
    lmbox varset tv_keystime $now
}

# Prepare to displaying TV listings
# 2 types are supported: list and matrix
proc tvguide_setup { type { time "" } { channel "" } { genres "" } } {

    # Calculate time
    set now [lmbox var tv_time [clock seconds]]
    if { $time != "" } {
      incr now $time
    }
    lmbox varset tv_time $now
    lmbox set tvchan caption [clock format $now -format "%m/%d/%y"]

    # Plain shows list
    if { $type == "list" } {
      # Remember the channel we were looking at
      set index [lmbox get tvlist selectedindex]
      lmbox call tvlist clear
      lmbox call tvlist2 clear
      # Current TV guide filter
      set filter [lmbox var tv_filter]
      if { $filter == "" } {
        # Search within current shows only if no filter
        set filter "$now >= \$rec(start_time) && $now <= \$rec(start_time)+\$rec(duration)"
      }
      lmbox dbeval tvguide {
        if { ![info exists rec(program_id)] } { continue }
        array set rec [lmbox dbget tvguide_programs $rec(program_id)]
        if { $filter != "" && ![expr $filter] } { continue }
        set time "$rec(start_time),$rec(channel_id)"
        set label "[format "%3d" $rec(channel_id)]: $rec(station_label)"
        set title "[clock format $rec(start_time) -format "%m/%d/%Y %H:%M"]: $rec(program_title) $rec(program_subtitle)"
        set show_list($time) [list $label $title $lmbox_dbkey]
      } -array rec
      # Now sort the list and add shows to the page
      foreach key [lsort [array names show_list]] {
        set show $show_list($key)
        lmbox call tvlist2 additem [lindex $show 1]
        lmbox call tvlist additemdata [lindex $show 0] [lindex $show end]
      }
      # Select the channel we were looking at
      lmbox set tvlist selectedindex $index
      lmbox set tvtitle caption "lmBox: [lmbox get tvlist count] TV Shows: [lmbox_clock $now]"
      return
    }

    # Matrix like format:
    #  ch0   01 02 03
    #  ch1   11 12 13
    #  ch2   21 22 23
    #  ch3   31 32 33

    # Program genre filter
    set genres [lmbox_nvl $genres [lmbox var tvguide_genre]]
    if { $genres != [lmbox var tvguide_genre] } {
      lmbox varset tv_showlist ""
      lmbox setconfig tv_row 0
    }
    # Regexps for different genres
    switch -- $genres {
     all {
        set genres ""
     }
     movie {
        set genres "comedy|drama|thriller|sitcom|soap|horror|mistery|fiction"
     }
     sport {
        set genres "sport|football|basketball|tennis|wresling|boxing|soccer|racing|baseball"
     }
     kids {
        set genres "animated|anime|children"
     }
    }
    lmbox varset tvguide_genre $genres
    set tv_count 0
    set show_list ""
    set timeslots ""
    set tv_end [expr $now+3600]
    set tv_start [expr $now-3600]
    set tv_row [lmbox config tv_row 0]
    # Time slots
    for { set i 0 } { $i < 3 } { incr i } {
      foreach { hour min } [clock format [expr $now+$i*1800] -format "%H %M"] {}
      if { $min < 30 } { set min "00" } else { set min 30 }
      lappend timeslots "$hour:$min"
      lmbox set tvslot$i caption "$hour:$min"
    }
    # Check cache first
    set tv_showlist [lmbox var tv_showlist]
    if { [lindex $tv_showlist 0] == "$tv_start-$tv_end" } {
      set show_list [lindex $tv_showlist 1]
      array set tv_cache [lindex $tv_showlist 2]
    } else {
      # First pass, retrieve all records
      lmbox dbeval tvguide_timestamp {
        array set rec [lmbox dbget tvguide $lmbox_dbdata]
	if { ![info exists rec(program_id)] } {
          continue
        }
        array set rec [lmbox dbget tvguide_programs $rec(program_id)]
        # Program title by id
        set tv_cache($lmbox_dbdata) $rec(program_title)
        # Cached list of programs
        lappend show_list [list [list [format "%05d" $rec(channel_id)] $rec(start_time)] \
                                $rec(station_label) \
                                $rec(duration) \
                                $rec(program_title) \
                                [string tolower $rec(genre)] \
                                $lmbox_dbdata]
      } -start $tv_start -end $tv_end
      # Save in cache for the same time range
      lmbox varset tv_showlist [list "$tv_start-$tv_end" $show_list [array get tv_cache]]
    }
    # Second pass, sort by channel, allocate slots
    foreach show [lsort -index 0 $show_list] {
      foreach { sort station duration title genre id } $show {}
      # Genres filter
      if { $genres != "" && ![regexp -nocase $genres $genre] } {
        continue
      }
      foreach { channel_id start_time } $sort {}
      # Prepare channel/program info
      if { ![info exists tvmap($channel_id)] } {
        set tvmap($channel_id) "$tv_count {$station} {$genre}"
        set tvrow($tv_count) $channel_id
        incr tv_count
      }
      set rowid [lindex $tvmap($channel_id) 0]
      # Fill all the timeslots be the program
      for { set i $start_time } { $i <= $start_time+$duration } { incr i 1800 } {
        foreach { hour minute } [clock format $i -format "%H %M"] {}
        if { $minute < 30 } { set minute "00" } else { set minute 30 }
        set tvslot($rowid:$hour:$minute) $id
      }
      # Start with given channel
      if { $channel != "" && $channel == $channel_id } {
        set tv_row $rowid
        lmbox setconfig tv_row $rowid
        lmbox call tvshow01 setfocus
        set channel ""
      }
    }
    # Third pass, put programs on the page
    set tv_nrows [lmbox config tv_rows 4]
    lmbox varset tv_count $tv_count
    for { set i 0 } { $i < $tv_nrows } { incr i } {
      set row [expr $i + $tv_row]
      if { [info exists tvrow($row)] } {
        set channel_id $tvrow($row)
        foreach { rowid label genre } $tvmap($channel_id) {}
        set label "[string trimleft $channel_id 0]\n$label"
      } else {
        set rowid N/A
        set label N/A
        set genre ""
      }
      lmbox set tvch${i} caption $label
      set nslot 0
      foreach slot $timeslots {
        if { [info exists tvslot($rowid:$slot)] } {
          set id $tvslot($rowid:$slot)
          set title [lmbox_var tv_cache($id)]
        } else {
          set id ""
          set title ""
        }
        incr nslot
        lmbox set tvshow${i}$nslot caption $title
        lmbox set tvshow${i}$nslot data $id
        # Different colors by genre
        set color #4985f3
        set focusedcolor #e0e2e5
        set disabledcolor #3569a9
        set fontcolor #1a2d5b
        if { $id != "" } {
          switch -regexp -- $genre {
           comedy|drama|horror|thriller|sitcom|soap|mistery|fiction {
             set color #49f385
           }

           animated|anime|children {
             set color #99ff00
          }

           sport|football|basketball|tennis|wresling|boxing|soccer|racing|baseball {
             set color #6600ff
             set fontcolor #e0e2e5
           }

           adult {
             set color #f34955
             set disabledcolor #a9a235
           }
          }
        }
        lmbox set tvshow${i}$nslot color $color
        lmbox set tvshow${i}$nslot focusedcolor $focusedcolor
        lmbox set tvshow${i}$nslot disabledcolor $disabledcolor
        lmbox set tvshow${i}$nslot fontcolor $fontcolor
      }
    }
}

proc tvguide_status { id } {

    array set rec [lmbox dbget tvguide $id]
    if { [info exists rec(program_id)] } {
      array set rec [lmbox dbget tvguide_programs $rec(program_id)]
    }
    if { [info exists rec(program_title)] } {
      lmbox set tvstatus caption "$rec(program_title)\n[tvguide_genre rec]\n^#FFFFFF$rec(description)"
    }
}

proc tvguide_genre { name } {

    upvar $name rec
    set info "[clock format $rec(start_time) -format "%m/%d/%Y %H:%M"], [lmbox_date uptime $rec(duration)] "
    if { [lmbox_var rec(genre)] != "" } { append info "[join $rec(genre)] " }
    if { $rec(closecaptioned) == "true" } { append info "CC " }
    if { $rec(stereo) == "true" } { append info "Stereo " }
    if { $rec(part_number) != "" } { append info "Part $rec(part_number) " }
    if { $rec(episode_number) != "" } { append info "Episode $rec(episode_number) " }
    if { $rec(tvrating) != "" } { append info "$rec(tvrating) " }
    if { $rec(mpaarating) != "" } { append info "$rec(mpaarating) " }
    if { $rec(starrating) != "" } { append info "$rec(starrating) " }
    return $info
}

proc tvguide_info { id } {

    array set rec [lmbox dbget tvguide $id]
    if { ![info exists rec(program_id)] } { return }
    array set rec [lmbox dbget tvguide_programs $rec(program_id)]
    set title "$rec(channel_id) $rec(station_label): $rec(program_title) [lmbox_var rec(program_subtitle]"
    if { [set crew [lmbox_var rec(crew)]] != "" } {
      append rec(description) "\n\nProduction Crew:\n"
      foreach { role name } $crew { append rec(description) "$role $name\n" }
    }
    lmbox set tvplay enabled [expr $rec(start_time) <= [clock seconds]]
    lmbox set tvinfotitle caption $title
    lmbox set tvinfogenre caption [tvguide_genre rec]
    lmbox set tvinfotext caption [lmbox_var rec(description)]
    lmbox set tvrecstatus fontcolor #FFFFFF
    lmbox set tvrecstatus caption "No recording conflicts"
    lmbox set tvrecord caption Record
    lmbox set tvrecord enabled [expr [clock seconds] < $rec(start_time)+$rec(duration)]
    # Recording status
    lmbox dbeval calendar {
      array set cal $lmbox_dbdata
      if { $cal(timestamp) >= $rec(start_time) &&
           $cal(timestamp) <= $rec(start_time)+$rec(duration) &&
           $cal(type) == "TVGuide" } {
        if { $rec(program_id) == $cal(program_id) && $rec(channel_id) == $cal(channel_id) } {
          # Already scheduled
          lmbox set tvrecord enabled 1
          lmbox set tvrecord caption Cancel
        } else {
          # We have conflict here
          lmbox set tvrecord enabled 0
        }
        set timestamp [clock format $cal(timestamp) -format "%m/%d/%Y %H:%M"]
        lmbox set tvrecstatus caption "Schedule Conflict: Time: $timestamp, Channel: $cal(channel_id) $cal(station_name), Title: $cal(name)"
        lmbox set tvrecstatus fontcolor #f4093a
        break
      }
    }
    lmbox set tvinfopage data $id
    lmbox setpage tvinfopage
}

proc tvguide_settings { { channel_id "" } } {

    # Start with previous or first channel if not given
    set channel_id [lmbox_nvl $channel_id [lmbox config tv_channel 3]]
    # TV defaults
    foreach { key dflt } { device /dev/video0 norm NTSC input 6 audio -1 chanlist us-cable hue "" contrast "" saturation "" brightness "" } {
      if { [set val [lmbox config tv_$key $dflt]] != "" } {
        lmbox set tv $key $val
      }
    }
    if { $channel_id != "" } {
      lmbox set tv channel $channel_id
    }
}

proc tvguide_channel { level { id "" } } {

    if { $level == "" && $id != "" } {
      array set rec [lmbox dbget tvguide $id]
      if { ![info exists rec(channel_id)] } { return }
      lmbox set tv channel $rec(channel_id)
    } else {
      switch -- $level {
       1 {
         lmbox call tv next
       }
       -1 {
         lmbox call tv prev
       }
       default {
         lmbox set tv channel $level
       }
      }
    }
    set channel [lmbox get tv channel]
    lmbox_setconfig tv_channel $channel
    player_osdtext 30 30 "Ch: $channel"
    set station [lmbox dbget tvguide_channels $channel]
    lmbox set tvchannel caption "$channel: [lindex $station 1]"
}

proc tvguide_icon { { channel_id 0 } { title "" } } {

    if { [lmbox get player status] != "play" } {
      tvguide_play "" 1 30 30 200 140
      lmbox set player aspect 0.8
      tvguide_channel $channel_id
    } else {
      lmbox call player move 30 30 200 140
    }
}

proc tvguide_play { id { play 0 } { x 0 } { y 0 } { w 0 } { h 0 } { channel_id "" } } {

    # Cannot watch TV while recording
    if { [set rid [lmbox var tv_recorder]] != "" } {
      array set rrec [lmbox dbget calendar $rid]
      if { [info exists rrec(channel_id)] } {
        lmbox set tvstatus label "Recording $rrec(name) on channel $rrec(channel_id) ..."
      }
      return
    }
    set pvr_params ""
    set start_time ""
    if { $id != "" } {
      array set rec [lmbox dbget tvguide $id]
      if { ![info exists rec(channel_id)] } { return }
      set channel_id $rec(channel_id)
      set start_time $rec(start_time)
    }
    if { $start_time > [clock seconds] } {
      tvguide_info $id
      return
    }
    set tv_url [lmbox get player file]
    set tv_player [lmbox config tv_player pvr]
    lmbox call player clear
    lmbox call player move $x $y $w $h
    lmbox set player videofilter [lmbox config tv_deinterlace]
    switch -- [string tolower $tv_player] {
     pvr {
       set tv_path [lmbox config tv_path data/TV]
       lmbox call player add pvr:/$tv_path/!$tv_path/![lmbox config tv_pvrpages 2]
     }
     v4l {
       lmbox call player add v4l:/
       lmbox set tv mute 0
     }
     default {
       lmbox call player add [lmbox config tv_stream $tv_player]
       lmbox set player repeat 1
     }
    }
    # Do not trip player if the same url
    if { $tv_url != [lmbox get player file] } {
      lmbox call player stop
    }
    # Initialize tv object
    tvguide_settings $channel_id
    if { $play } {
      lmbox call player play
    } else {
      lmbox setpage videopage
    }
}

proc tvguide_favorite { id args } {

    set class ""
    set expires 0
    foreach { key val } $args { set $key $val }

    array set rec [lmbox dbget tvguide $id]
    if { ![info exists rec(channel_id)] } { return }
    set title "$rec(channel_id): $rec(station_label)"
    set description "$rec(channel_id): $rec(station_name)"

    favorites_add TV "script:tvguide_play {} 0 0 0 0 0 $rec(channel_id)" $title description $description expires $expires class $class
    lmbox set status caption "$rec(station_label) added to favorites"
}

proc tvguide_stop {} {

    switch -- [lmbox newpage] {
     tvinfopage -
     tvrecordpage -
     tvpage {
     }

     default {
       lmbox set tv mute 1
       lmbox call player stop
     }
    }
}

proc tvguide_schedule { id { type Record } } {

    set now [clock seconds]
    array set rec [lmbox dbget tvguide $id]
    array set rec [lmbox dbget tvguide_programs $rec(program_id)]
    switch -- $type {
     Cancel {
        # Unschedule exiting recording
        tvguide_unschedule $id
        lmbox set tvstatus caption "Recording has been cancelled"
     }

     default {
        # Schedule new recording, it assumes that duration check was performed
        set cal(timestamp) $rec(start_time)
        set cal(name) $rec(program_title)
        set cal(type) TVGuide
        foreach key { duration channel_id program_id station_name } {
          set cal($key) $rec($key)
        }
        set cal(file) [string map { { } _ & {} < {} > {} | {} } "[clock format $rec(start_time) -format "%b.%d.%Y.%H:%M"]_$rec(channel_id)_$cal(name).mpg"]
        lmbox dbput calendar $id [array get cal]
        # Schedule immediately in separate thread
        if { $rec(start_time) <= $now } {
          lmbox call player stop
          lmbox set tv mute 1
          lmbox run "tvguide_recorder $id"
          return 1
        } else {
          calendar_schedule $id
        }
        lmbox set tvstatus caption "Show has been scheduled for recording"
     }
    }
}

proc tvguide_unschedule { id } {

    array set rec [lmbox dbget calendar $id]
    if { ![info exists rec(file)] } { return }
    lmbox dbdel calendar $id
    catch { lmbox scheduleremove $id }
    # Remove recorder file as well
    set tv_path [lmbox config tv_path data/TV]
    catch { file delete -force -- $tv_path/$rec(file) }
}

# Perform show recording
proc tvguide_recorder { id } {

    array set rec [lmbox dbget calendar $id]
    if { ![info exists rec(file)] } { return }
    lmbox puts "tvguide_recorder: $id: started $rec(file)"
    # Mark recording
    lmbox varset tv_recorder $id
    # Initialize TV tuner and switch channel
    tvguide_settings $rec(channel_id)
    # Configuration
    set device [lmbox get tv device /dev/video0]
    set file [lmbox config tv_path data/TV]/$rec(file)
    switch -- [set type [string tolower [lmbox config tv_player pvr]]] {
     v4l {
       set norm [lmbox config tv_norm NTSC]
       set width [lmbox config tv_width 704]
       set height [lmbox config tv_height 480]
       set abitrate [lmbox config tv_abitrate 128]
       set vbitrate [lmbox config tv_vbitrate 2500]
       set chanlist [lmbox config tv_chanlist us-cable]
       append cmd "/usr/bin/mencoder tv:// "
       append cmd "-tv driver=v4l2:device=$device:norm=$norm:channel=$rec(channel_id):chanlist=$chanlist:width=$width:height=$height "
       append cmd "-quiet -oac lavc -ovc lavc -of mpeg -ofps 25 -vop pp=lb "
       append cmd "-lavcopts vcodec=mpeg2video:acodec=mp2:vbitrate=$vbitrate:abitrate=$abitrate:vhq:vqmin=2:vqmax=31 "
       append cmd "-endpos $rec(duration) -o $file"
       lmbox puts "tvguide_recorder: $cmd"
       if { [catch { eval exec $cmd } errmsg] } {
         lmbox puts "lmbox_recorder: $id: $errmsg"
       }
     }
     pvr {
       if { [catch {
         # Read MPEG directly from the device
         set wd [open $file w]
         fconfigure $wd -translation binary -encoding binary
         set fd [open $device r]
         fconfigure $fd -translation binary -encoding binary
         set now [clock seconds]
         while { [clock seconds] - $now <= $rec(duration) } {
           set buf [read $fd 2048]
           puts -nonewline $wd $buf
         }
       } errmsg] } {
         lmbox puts "lmbox_recorder: $id: $errmsg"
       }
       catch { close $wd }
       catch { close $fd }
     }
    }
    lmbox varset tv_recorder ""
}

proc tvguide_search { { filter "" } } {

    set search ""
    # Current filter
    if { $filter == "" } { set filter [lmbox var tvguide_filter] }
    foreach { name value } $filter {
      switch -- $name {
       "" {}
       Title {
         if { $value == "" } { continue }
         lappend search "(\[regexp -nocase {$value} \$rec(program_title)\$rec(program_subtitle)\$rec(description)\$rec(crew)\])"
       }
       All {
         if { $value != 1 && $value != "true" } { continue }
         lappend search "\$rec(start_time) >= \[clock seconds\] - \$rec(duration)"
       }
       default {
         if { $value != 1 && $value != "true" } { continue }
         lappend search "\[regexp -nocase {$name} \$rec(genre)\]"
       }
      }
    }
    # Save the filter, setup will use this filter until reset
    lmbox varset tv_filter [join $search " && "]
    lmbox setpage tvlistpage
}

# Checks if tvguide needs to be loaded
proc tvguide_checker { { days 7 } } {

    set max_time 0
    set now [clock seconds]
    set days [lmbox config tvguide_days $days]
    lmbox dbeval tvguide {
      if { $max_time < $rec(start_time) } { set max_time $rec(start_time) }
    } -array rec
    if { $max_time - $now >= 86400*$days } { return }
    lmbox puts "tvguide_schedule: [expr $max_time - $now] < [expr 86400*$days]"
    tvguide_parse $max_time
}

proc tvguide_parse { { start_time 0 } } {

    set login [split [lmbox config tvguide_login] :]
    if { [set user [lindex $login 0]] == "" || [set passwd [lindex $login 1]] == "" } {
      lmbox puts "tvguide_parse: user/password should be specified in tvguide_login config parameter as user:password"
      return
    }
    # Allow only one instance to do that
    if { [lmbox var tvguide_flag 0] == 1 } {
      lmbox puts "tvguide_parse: already running"
      return
    }
    lmbox varset tvguide_flag 1
    lmbox_puts "TV Guide database refresh started..."

    # SOAP settings
    set xmlfile /tmp/tvguide.xml
    set tmpfile /tmp/tvguide.soap
    set soapaction "SOAPAction: urn:TMSWebServices:xtvdWebService#download"
    set url http://datadirect.webservices.zap2it.com/tvlistings/xtvdService
    # Download and parse the XML file
    if { [catch {
      set now [clock seconds]
      if { $start_time <= 0 } { set start_time $now }
      set days [lmbox config tvguide_days 7]
      set start [clock format $start_time -format "%Y-%m-%dT%H:%M:%SZ" -gmt 1]
      set end [clock format [expr $start_time+86400*$days] -format "%Y-%m-%dT23:59:59Z" -gmt 1]
      lmbox puts "tvguide_parse: downloading from $start to $end"
      set fd [open $tmpfile w]
      puts $fd "<?xml version='1.0' encoding='utf-8'?><SOAP-ENV:Envelope xmlns:SOAP-ENV='http://schemas.xmlsoap.org/soap/envelope/' xmlns:xsd='http://www.w3.org/2001/XMLSchema' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:SOAP-ENC='http://schemas.xmlsoap.org/soap/encoding/'> <SOAP-ENV:Body><tms:download xmlns:tms='urn:TMSWebServices'><startTime xsi:type='tms:dateTime'>$start</startTime><endTime xsi:type='tms:dateTime'>$end</endTime></tms:download></SOAP-ENV:Body></SOAP-ENV:Envelope>"
      close $fd
      file delete -force -- $xmlfile
      exec wget -q -O - --http-user $user --http-passwd $passwd --post-file $tmpfile --header $soapaction --header "Accept-Encoding: gzip" $url | gzip -d > $xmlfile
      tvguide_reset
      tvguide_cleanup
      lmbox xmlparse -file $xmlfile -startelement {tvguide_xml start} -endelement {tvguide_xml end} -characters {tvguide_xml data}
    } errmsg] } {
      lmbox puts "tvguide_parse: $errmsg"
    }
    lmbox dbsync
    lmbox varunset tvguide:*
    lmbox varset tvguide_flag 0
    lmbox_puts "TV Guide database loaded"
}

proc tvguide_cleanup { { type "" } } {

    switch -- $type {
     full {
       lmbox dbdel tvguide
       lmbox dbdel tvguide_channels
       lmbox dbdel tvguide_programs
     }

     default {
       set now [clock scan [clock format [clock seconds] -format "%Y-%m-%d 23:59:59"]]
       lmbox dbeval tvguide {
         if { $rec(start_time) < $now-43200 } {
           return lmbox_delete
         }
       } -array rec
     }
    }
}

proc tvguide_reset {} {

    lmbox varset tvguide_top ""
    lmbox varset tvguide_name ""
    lmbox varset tvguide_data ""
}

proc tvguide_xml { tag { name "" } args } {

    switch -- $tag {
     start {
       switch -- [lmbox var tvguide_top]$name {
        lineups -
        stations -
        schedules -
        programs -
        productionCrew -
        genres {
          lmbox puts "tvguide_xml: parsing $name"
        }

        station {
          lmbox varset tvguide_top station
          foreach { name value } $args {
            switch -- $name {
             id { set name station_id }
            }
            lmbox varset tvguide_data "[lmbox var tvguide_data] {$name} {$value}"
          }
        }

        lineup {
          set columns { lineup_id lineup_name lineup_type lineup_device lineup_location lineup_zipcode }
          foreach name $columns { set $name "" }
          lmbox varset tvguide_top lineup
          foreach { name value } $args {
            switch -- $name {
             id {
               set name lineup_id
               lmbox varset tvguide_lineup_id $value
             }
             name - type - device - location { set name lineup_$name }
             postalCode { set name lineup_zipcode }
            }
            set $name $value
          }
          lmbox varset tvguide:lname:$lineup_id $lineup_name
        }

        lineupmap {
          set columns { channel_id lineup_id lineup_name station_id station_name station_label }
          foreach name $columns { set $name "" }
          set lineup_id [lmbox var tvguide_lineup_id]
          foreach { name value } $args {
            switch -- $name {
             station - channel { set ${name}_id $value }
             from { set start_date $value }
            }
          }
          lmbox varset tvguide:chan:$lineup_id:$station_id $channel_id
          set station_name [lmbox var tvguide:sname:$station_id]
          set station_label [lmbox var tvguide:slabel:$station_id]
	  lmbox dbput tvguide_channels $channel_id [list $station_name $station_label]
        }

        schedule {
          lmbox varset tvguide_top schedule
          foreach { name value } $args {
            switch -- $name {
             station - program { set name ${name}_id }
             time { set name start_time; set value [string map { T { } Z { UTC} } $value] }
             duration { set value [string map { T {} P {} H : M {} } $value] }
            }
            lmbox varset tvguide_data "[lmbox var tvguide_data] {$name} {$value}"
          }
        }

        schedulepart {
          foreach { key value } $args {
            lmbox varset tvguide_data "[lmbox var tvguide_data] {part_$key} {$value}"
          }
        }

        crew -
        program -
        programGenre {
          lmbox varset tvguide_top $name
          foreach { key value } $args {
            switch -- $key {
             id - program { set key program_id }
            }
            lmbox varset tvguide_data "[lmbox var tvguide_data] {$key} {$value}"
          }
        }

        crewrole -
        crewgivenname -
        crewsurname -
        programGenreclass -
        programGenrerelevance -
        programtitle -
        programsubtitle -
        programdescription -
        programshowType -
        programseries -
        programsyndicatedEpisodeNumber -
        programoriginalAirDate -
        programrunTime -
        programmpaaRating -
        programstarRating -
        programyear -
        stationname -
        stationcallSign -
        stationaffiliate -
        stationfccChannelNumber {
          switch -- $name {
           title { set name program_title }
           subtitle { set name program_subtitle }
           originalAirDate { set name program_date }
           syndicatedEpisodeNumber { set name episode_number }
           showType { set name program_type }
           runTime { set name runtime_id }
           series { set name series_id }
           year { set name program_year }
           class { set name genre }
           name { set name station_name }
           callSign { set name station_label }
           fccChannelNumber { set name channel_id }
          }
          lmbox varset tvguide_name $name
        }
       }
     }

     end {
       switch -- [lmbox var tvguide_top].$name {
        station.station {
          set columns { station_id station_name station_label affiliate }
          foreach key $columns { set $key "" }
          foreach { key value } [lmbox var tvguide_data] { append [string tolower $key] $value }
	  set lineup_id [lmbox var tvguide_lineup_id]
          lmbox varset tvguide:sname:$station_id $station_name
          lmbox varset tvguide:slabel:$station_id $station_label
          tvguide_reset
        }

        schedule.schedule {
          set columns { program_id station_name station_label channel_id lineup_id start_time duration tvrating closecaptioned stereo part_number part_total }
          foreach key $columns { set rec($key) "" }
          foreach { key value } [lmbox var tvguide_data] { append rec([string tolower $key]) $value }
          set rec(lineup_id) [lmbox var tvguide_lineup_id]
          set rec(station_name) [lmbox var tvguide:sname:$rec(station_id)]
          set rec(station_label) [lmbox var tvguide:slabel:$rec(station_id)]
          set rec(channel_id) [lmbox var tvguide:chan:$rec(lineup_id):$rec(station_id)]
          set rec(start_time) [clock scan $rec(start_time) -gmt 1]
          foreach { hours mins } [split $rec(duration) :] {
            set rec(duration) [expr [lmbox_trim0 $hours]*3600+[lmbox_trim0 $mins]*60]
          }
          set end_time [expr $rec(start_time) + $rec(duration)]
          while { $rec(start_time) < $end_time && $rec(duration) > 0 } {
            set id [lmbox dbnextid tvguide_sequence]
            lmbox dbput tvguide $id [array get rec]
            lmbox dbput tvguide_timestamp $rec(start_time) $id
            incr rec(start_time) 1800
            incr rec(duration) -1800
          }
          tvguide_reset
        }

        program.program {
          set columns { program_id program_title program_subtitle program_type program_date program_year episode_number series_id runtime_id mpaarating starrating description genre crew }
          foreach key $columns { set rec($key) "" }
          foreach { key value } [lmbox var tvguide_data] { append rec([string tolower $key]) $value }
          set rec(program_date) [clock scan $rec(program_date) -gmt 1]
          lmbox dbput tvguide_programs $rec(program_id) [array get rec]
          tvguide_reset
        }


        crew.member {
          set columns { program_id role givenname surname }
          foreach key $columns { set $key "" }
          foreach { key value } [lmbox var tvguide_data] { append [string tolower $key] $value }
          array set rec [lmbox dbget tvguide_programs $program_id]
          lappend rec(crew) $role [string trim "$givenname $surname"]
          lmbox dbput tvguide_programs $program_id [array get rec]
          lmbox varset tvguide_data "program_id $program_id"
        }

        programGenre.genre {
          set columns { program_id genre relevance }
          foreach key $columns { set $key "" }
          foreach { key value } [lmbox var tvguide_data] { append [string tolower $key] $value }
          array set rec [lmbox dbget tvguide_programs $program_id]
          lappend rec(genre) $genre
          lmbox dbput tvguide_programs $program_id [array get rec]
          lmbox varset tvguide_data "program_id $program_id"
        }

        crew.crew -
        programGenre.programGenre {
          tvguide_reset
        }

        lineup.lineup {
          tvguide_reset
        }

        station.name -
        station.callSign -
        station.affiliate -
        station.fccChannelNumber {
          lmbox varset tvguide_name ""
        }
       }
     }

     data {
       if { [set data [string trim $name "\n\r\t"]] == "" } { return }
       switch -glob -- [lmbox var tvguide_top] {
        programGenre -
        program -
        station -
        crew {
          lmbox varset tvguide_data "[lmbox var tvguide_data] {[lmbox var tvguide_name]} {$name}"
        }
       }
     }
    }
}

proc tvrec_setup {} {

    lmbox call tvlist clear
    lmbox dbeval calendar {
       if { $rec(type) == "TVGuide" } {
         set label "[clock format $rec(timestamp) -format "%m/%d/%Y %H:%M"]: $rec(channel_id): $rec(name)"
         lmbox call tvlist additemdata $label $lmbox_dbkey
       }
    } -array rec
    set tv_path [lmbox config tv_path data/TV]
    foreach file [glob -nocomplain $tv_path/*.vob] {
      lmbox call tvlist additemdata [file rootname [file tail $file]] $file
    }
}

proc tvrec_info { id } {

    if { $id == "" } { return }
    array set rec [lmbox dbget calendar $id]
    if { ![info exists rec(file)] } { return }
    lmbox set tvstatus caption [lmbox_date uptime $rec(duration)]
    set tv_path [lmbox config tv_path data/TV]
    lmbox set tvplay enabled [file exists $tv_path/$rec(file)]
}

proc tvrec_play { id } {

    array set rec [lmbox dbget calendar $id]
    if { ![info exists rec(file)] } { return }
    set tv_path [lmbox config tv_path data/TV]
    if { ![file exists $tv_path/$rec(file)] } { return }
    lmbox call player clear
    lmbox call player add $tv_path/$rec(file)
    lmbox setpage videopage
}

proc tvrec_delete { id } {

    tvguide_delete $id
    lmbox set tvstatus caption "Recording has been cancelled"
    lmbox call tvlist removeitem [lmbox get tvlist selectedindex]
}

