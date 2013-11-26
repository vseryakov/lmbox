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

lmbox_register_init calendar_init

proc calendar_init {} {

    lmbox_register_menu Personal "Calendar" "Calendar and alarms" calendar.jpg { lmbox setpage calendarpage } 1300
}

proc calendar_setup { { date "" } } {

    switch -- $date {
     prevMonth -
     nextMonth {
       if { [set olddate [lmbox var calendar:date]] == "" } { return }
       set date [lmbox_date $date $olddate]
     }
     "" {
       set date [clock seconds]
     }
    }
    # Save current date
    lmbox varset calendar:date $date
    lmbox set calendartitle caption [clock format $date -format "%B, %Y"]
    set today [clock format [clock seconds] -format "%Y-%m-%d"]
    set month [clock format $date -format "%Y-%m"]
    set days [eval lmbox_date monthArray [clock format $date -format "%m %Y"]]
    for { set i 0 } { $i < [llength $days] } { incr i } {
      set day [lindex $days $i]
      if { $day == "" } {
        lmbox set calendar$i enabled 0
        lmbox set calendar$i caption $day
        lmbox set calendar$i background calendar_d.png
        continue
      }
      set alarms 0
      set start [clock scan "$month-$day 0:0:0"]
      set end [clock scan "$month-$day 23:59:59"]
      lmbox set calendar$i background calendar.png
      # Show all alarms for this day
      lmbox dbeval calendar {
        array set rec $lmbox_dbdata
        if { $rec(timestamp) >= $start && $rec(timestamp) <= $end } {
          incr alarms
        }
      }
      if { $alarms > 0 } {
        set alarms "\n\n# $alarms"
        lmbox set calendar$i background calendar_a.png
      } else {
        set alarms ""
      }
      if { $today == "$month-$day" } {
        lmbox set calendar$i background calendar_t.png
        lmbox call calendar$i setfocus
      }
      lmbox set calendar$i enabled 1
      lmbox set calendar$i caption "$day$alarms"
      lmbox set calendar$i data "$month-$day"
    }
}

proc calendar_day { { date "" } } {

    if { $date == "" } {
      set date [lmbox get this data]
    }
    set date [clock scan "$date 0:0:0"]
    lmbox varset calendar:date $date
    lmbox set alarmclock caption [clock format $date -format {%a %b %d, %Y}]
    # Initialize time select boxes
    if { [lmbox get hourslist count] == 0 } {
      for { set i 0 } { $i <= 23 } { incr i } { lmbox call hourslist additem [format "%02d" $i] }
    }
    if { [lmbox get minuteslist count] == 0 } {
      for { set i 0 } { $i <= 59 } { incr i } { lmbox call minuteslist additem [format "%02d" $i] }
    }
    if { [lmbox get typelist count] == 0 } {
      lmbox call typelist additem Popup
      lmbox call typelist additem Movie
      lmbox call typelist additem Cartoon
      lmbox call typelist additem Music
      lmbox call typelist additem Stop
      lmbox call typelist additem Shutdown
    }
    if { [lmbox get repeatlist count] == 0 } {
      lmbox call repeatlist additem None
      lmbox call repeatlist additem Hourly
      lmbox call repeatlist additem Daily
      lmbox call repeatlist additem Weekly
      lmbox call repeatlist additem Monthly
      lmbox call repeatlist additem Yearly
    }
    # Populate alarms listbox
    lmbox call alarmlist clear
    lmbox dbeval calendar {
      array set rec $lmbox_dbdata
      set time [clock format $rec(timestamp) -format "%H:%M"]
      lmbox call alarmlist additemdata "$time | $rec(name)" $lmbox_dbkey
    }
    lmbox setpage calendardaypage
}

proc calendar_add {} {

    set date [lmbox var calendar:date]
    set rec(name) [lmbox get alarmtext text]
    set rec(type) [lmbox get typelist selectedtext]
    set rec(file) [lmbox get alarmfile caption]
    set rec(repeat) [lmbox get repeatlist selectedtext]
    set time "[lmbox get hourslist selectedtext]:[lmbox get minuteslist selectedtext]"
    set rec(timestamp) [clock scan "[clock format $date -format "%Y-%m-%d"] $time"]

    if { $rec(name) == "" } {
      lmbox set alarmstatus caption "Please, enter alarm text"
      lmbox call alarmtext setfocus
      return
    }

    switch -- $rec(type) {
     Movie - Cartoon - Music {
       if { $rec(file) == "" || $rec(file) == "Choose..." } {
         lmbox set alarmstatus caption "Please, choose media file to run on alarm"
         lmbox call alarmfile setfocus
         return
       }
     }
     default {
       set rec(file) ""
     }
    }
    if { $rec(repeat) == "None" } { set rec(repeat) "" }
    set id [lmbox dbnextid]
    lmbox dbput calendar $id [array get rec]
    lmbox call alarmlist additemdata "$time | $rec(name)" $id
    calendar_schedule $id
    calendar_reset
}

proc calendar_remove {} {

    set id [lmbox get alarmlist selecteddata]
    lmbox dbdel calendar $id
    lmbox call alarmlist removeitem [lmbox get alarmlist selectedindex]
    catch { lmbox scheduleremove $id }
}

proc calendar_info {} {

    set id [lmbox get alarmlist selecteddata]
    array set rec [lmbox dbget calendar $id]
    if { [info exists rec(type)] } {
      lmbox set alarmstatus caption "$rec(type) [lmbox_var rec(repeat)] [file tail [lmbox_var rec(file)]]"
    }
}

proc calendar_reset {} {

    lmbox set alarmtext text ""
    lmbox set hourslist selectedindex 0
    lmbox set minuteslist selectedindex 0
    lmbox set typelist selectedindex 0
}

proc calendar_media {} {

    switch -- [lmbox get typelist selectedtext] {
     Cartoon {
       lmbox varset movie_select alarmfile
       movie_search Lookup { Cartoon 1 }
     }
     Movie {
       lmbox varset movie_select alarmfile
       movie_search Lookup { Movie 1 }
     }
     Music {
       lmbox varset music_select alarmfile
       lmbox setpage musicselectpage
     }
    }
}

proc calendar_alarm { id } {

    array set rec [lmbox dbget calendar $id]
    if{ ![info exists rec(type)] || ![info exists rec(name)] } { return }
    lmbox puts "calendar::alarm: $id: $rec(type)/[lmbox_var rec(repeat)]: $rec(name)/[lmbox_var rec(file)]"
    photo_stop
    movie_stop
    sound_play
    switch -- $rec(type) {
     Movie - Cartoon {
       movie_play $rec(file)
     }
     Music {
       music_play $rec(file)
     }
     Shutdown {
       lmbox_shutdown
     }
     TVGuide {
       tvguide_recorder $id
     }
    }
    lmbox_alarm $rec(name)
}

proc calendar_schedule { { idlist "" } } {

    lmbox dbeval calendar {
      switch -- [lmbox_var rec(repeat)] {
       Hourly - Daily - Weekly - Monthly - Yearly {}
       default { continue }
      }
      # Schedule only specified calendar ids
      if { $idlist != "" && [lsearch -exact $idlist $lmbox_dbkey] == -1 } { continue }
      switch -- [lmbox_var rec(repeat)] {
       Hourly {
         set args [clock format $rec(timestamp) -format "-min %M"]
       }
       Daily {
         set args [clock format $rec(timestamp) -format "-min %M -hour %H"]
       }
       Weekly {
         set args [clock format $rec(timestamp) -format "-min %M -hour %H -dow %w"]
       }
       Monthly {
         set args [clock format $rec(timestamp) -format "-min %M -hour %H -day %d"]
       }
       Yearly {
         set args [clock format $rec(timestamp) -format "-min %M -hour %H -mon %m -day %d"]
       }
       default {
         set args [clock format $rec(timestamp) -format "-min %M -hour %H -mon %m -day %d -dow %w -year %Y"]
       }
      }
      switch -- [lmbox_var rec(type)] {
       TVGuide {
         eval lmbox schedule "{tvguide_recorder $lmbox_dbkey} -id $lmbox_dbkey -thread 1 -once 1 $args"
       }
       default {
         eval lmbox schedule "{calendar_alarm $lmbox_dbkey} -id $lmbox_dbkey -thread 1 $args"
       }
      }
    } -array rec
}
