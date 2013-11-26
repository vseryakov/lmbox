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

# Returns to previous page, takes care about page loops
proc lmbox_cancel {} {

    if { [set page [lmbox prevpage]] == "" || [lmbox getpage] == $page } {
      set page mainpage
    }
    lmbox setpage $page
}

# Exits the lmbox
proc lmbox_quit {} {

    lmbox_dbsave
    exit
}

# Performs syncing with remote file server
proc lmbox_sync {} {

    set sync_cmd [lmbox config sync_cmd /etc/rc.sync]
    if { $sync_cmd != "" } { catch { exec /bin/sh -c "$sync_cmd" } }
}

# Runs checker for each registered module
proc lmbox_check { args } {

    lmbox_dbsave
    foreach { title script } [lmbox var lmbox:check:list] {
      if { [catch { eval $script } errmsg] } {
        lmbox puts "$title: $errmsg: $::errorInfo"
      }
      lmbox dbsync
    }
}

# Routine checks, idle, screensaver, shutdown
proc lmbox_housekeeping { args } {

    lmbox dbsync
    set idle [lmbox get system idletime]
    # Can stay in those pages as long as needed
    switch -glob -- [lmbox getpage] {
     infopage -
     videopage -
     config* {
       lmbox set system idletime 0
       return
     }
    }
    # TV guide refreshing, keep running
    if { [lmbox var tvguide_flag 0] } {
      return
    }
    # Run screensaver in music mode
    if { [lmbox get player status] == "play" &&
         ![lmbox get player hasvideo] &&
         [lmbox config sound_effects] == 1 } {
      set idle_timeout [lmbox config idle_timeout 30]
      if { $idle_timeout > 0 && $idle > $idle_timeout } {
        lmbox set system idletime 0
        idle_setup
      }
      return
    }
    # Check when we can shutdown the lmbox
    set idle_shutdown [lmbox config idle_shutdown 0]
    if { $idle_shutdown == "" ||
         $idle_shutdown <= 0 ||
         $idle_shutdown*60 > $idle } {
      return
    }
    # Otherwise shutdown the box
    lmbox_shutdown
}

# Called whenever play button pressed
proc lmbox_play {} {

    set name [string trim [menu_selected]]
    lmbox puts "Playing for menu $name..."
    switch -glob -- $name {
     *Music* {
       music_shuffle 1
       music_play [music_setup]
     }

     *Video* -
     *Movie* -
     *Cartoon* -
     *Release* {
       if { [lmbox get player playlist] == "" } {
         switch -- $name {
          *Cartoon* { movie_setup Cartoon }
          *Home* { movie_setup Clip }
          default { movie_setup Movie }
         }
         if { [set count [lmbox get movielist count]] > 0 } {
           set idx [expr round(rand()*100) % $count]
           movie_play [lmbox call movielist data $idx]
           return
         }
       }
       lmbox setpage videopage
     }

     *Photo* {
       idle_setup
     }

     default {
       disk_play
     }
    }
}

# Called whenever stop button pressed, executes all registered stop handlers
proc lmbox_stop {} {

    # Stop all players
    foreach name [lmbox get system objects] {
      if { [regexp -nocase {^player|^radio} $name] } {
        lmbox call $name stop
      }
    }
    # Registered handlers
    foreach name [lmbox var lmbox:stop:list] {
      if { [catch { eval $name } errmsg] } {
        lmbox puts "lmbox_stop: $errmsg"
      }
    }
}

proc lmbox_eject {} {

    disk_open
}

proc lmbox_next {} {

    music_next
}

proc lmbox_pause {} {

    music_pause
}

proc lmbox_mute {} {

    music_mute
}

proc lmbox_title { str } {

    set title ""
    foreach word $str { lappend title [string totitle $word] }
    return [join $title]
}

proc lmbox_shutdown { { type "" } } {

    lmbox_dbsave
    lmbox puts "lmbox shutdown, idle=[lmbox get system idletime]"
    switch -- $type {
     reboot {
       catch { exec /sbin/shutdown -r now }
     }
     default {
       catch { exec /sbin/shutdown -h now }
     }
    }
}

proc lmbox_files { path { exp "" } } {

    set files ""
    foreach dir $path {
      foreach file [glob -nocomplain -- $dir/*] {
        if { [file isdirectory $file] } {
          foreach file [lmbox_files [list $file] $exp] { lappend files $file }
        } elseif { [file isfile $file] } {
          if { $exp != "" && ![regexp $exp $file] } { continue }
          lappend files $file
        }
      }
    }
    return $files
}

# From Christoph Bauer, http://wiki.tcl.tk/941
proc lmbox_shuffle { list } {

    set n 1
    set slist {}
    foreach item $list {
      set index [expr {int(rand()*$n)}]
      set slist [linsert $slist $index $item]
      incr n
    }
    return $slist
}

proc lmbox_true { x } {

    expr [lsearch -exact { 0 f false n no off } [string tolower $x]] == -1
}

proc lmbox_keys { { key "" } } {

    # Reset keys and exit
    if { $key == "" } {
      lmbox varset lmbox_keys ""
      return
    }
    # Convert event names into actual keys
    switch -regexp -- $key {
     {^number} { set key [string index $key 6] }
     {^key} { set key [string index $key 3] }
     default { return }
    }

    lmbox varappend lmbox_keys $key
    set key [lmbox var lmbox_keys]
    switch -- [lmbox getpage] {
     mainpage {
        switch -- $key {
         0101 {
            exit
         }
         0707 {
            set  xterm_cmd [lmbox config xterm_cmd xterm]
            if { $xterm_cmd != "" } {
              catch { exec /bin/sh -c "$xterm_cmd" }
            }
         }
         default {
            set admin_password [lmbox config admin_password]
            if { $admin_password != "" && $admin_password == $key } {
              if { ![lmbox_admin_mode] } {
                lmbox varset admin_time [clock seconds]
                lmbox set status caption "Admin password accepted"
              } else {
                lmbox varset admin_time 0
                lmbox set status caption "Admin mode disabled"
              }
              lmbox varset lmbox_keys ""
            }

            set teen_password [lmbox config teen_password]
            if { $teen_password != "" && $teen_password == $key } {
              if { ![lmbox_admin_mode] && ![lmbox_teen_mode] } {
                lmbox varset teen_time [clock seconds]
                lmbox set status caption "Teen password accepted"
              } else {
                lmbox varset teen_time 0
                lmbox set status caption "Teen mode disabled"
              }
              lmbox varset lmbox_keys ""
            }
         }
        }
     }
    }
}

# Returns 1 if box in the admin mode
proc lmbox_admin_mode {} {

    set now [clock seconds]
    set admin_time [lmbox var admin_time 0]
    if { [lmbox config admin_password] == "" } {
      set admin_time $now
    }
    set admin_timeout [lmbox config admin_timeout 3600]
    if { $now - $admin_time > $admin_timeout } {
      return 0
    }
    return 1
}

# Returns 1 if box in the teen mode
proc lmbox_teen_mode {} {

    set now [clock seconds]
    set teen_time [lmbox var teen_time 0]
    if { [lmbox config teen_password] == "" } {
      set teen_time $now
    }
    set teen_timeout [lmbox config teen_timeout 3600]
    if { $now - $teen_time > $teen_timeout } {
      return 0
    }
    return 1
}

# Returns 1 if box in the adult mode
proc lmbox_adult_mode {} {

    set now [clock seconds]
    set admin_time [lmbox var admin_time 0]
    if { [lmbox config admin_password] == "" } {
      set admin_time $now
    }
    set adult_timeout [lmbox config adult_timeout 60]
    if { $now - $admin_time > $adult_timeout } {
      return 0
    }
    return 1
}

proc lmbox_clock { { time "" } } {

    if { $time == "" } { set time [clock seconds] }
    return [clock format $time -format {%a %b %d, %Y %H:%M}]
}

proc lmbox_wrap { text { size 50 } } {

    set lines [split $text "\n"]
    set result ""
    foreach line $lines {
      while { [string length $line] > $size } {
       for { set i $size } \
           { $i > 0 && [string first [string index $line $i] " ,;-:.|!?/"] == -1 } \
           { incr i -1 } {}
       if { $i == 0 } { set i $size }
       append result [string range $line 0 $i] "\n"
       set line [string range $line [incr i] end]
      }
      append result $line "\n"
    }
    return $result
}

proc lmbox_striphtml { text } {

    set map { \n {} \r {} \t {} &quot; \" &amp; & &lt; < &gt; > &nbsp; { } &copy; C &trade; TM &reg; R }
    for { set i 32 } { $i < 128 } { incr i } {
      lappend map "&#$i;" [format {%c} $i]
      if { $i < 100 } { lappend map "&#0$i;" [format {%c} $i] }
    }
    return [string map $map [lmbox striphtml $text]]
}

proc lmbox_pad0 { string size } {

    if { ![string is integer -strict $string] } { return $string }
    set val [string repeat "0" [expr $size - [string length $string]]]
    append val $string
    return $val
}

proc lmbox_nvl { value { default "" } } {

    if { $value != "" } { return $value }
    return $default
}

proc lmbox_var { name { default "" } } {

    upvar 1 $name value
    if { ![info exists value] || $value == "" } { return $default }
    return $value
}

proc lmbox_trim0 { value } {

    set val [string trimleft $value 0]
    if { $value != "" && $val == "" } { return 0 }
    return $val
}

# Returns image name if found in the specified path
# Check all icons and return the last modified one
proc lmbox_icon { name args } {

    set icon ""
    set mtime 0
    foreach path $args {
      # Use common icons path if not specified
      if { $path == "" } {
        set path [lmbox gethome]/icons
      }
      foreach ext { png jpg gif } {
        if { [file exists $path/$name.$ext] &&
             [set mtime0 [file mtime $path/$name.$ext]] > $mtime } {
          set icon $path/$name.$ext
          set mtime $mtime0
        }
      }
    }
    return $icon
}

# Reset current page widgets
proc lmbox_reset { { filter "" } } {

    foreach name [lmbox get [lmbox getpage] widgetlist] {
      switch -- [lmbox get $name type] {
       togglebutton {
         lmbox set $name checked 0
       }
       textfield {
         lmbox set $name text ""
       }
       listbox {
         lmbox set ${name}_filter caption None
       }
      }
    }
    if { $filter != "" } {
      lmbox varset $filter ""
    }
}

# Construct search filter
proc lmbox_filter { filter { name "" } } {

    set name [lmbox_nvl $name [lmbox get this name]]
    array set search [lmbox var $filter]
    switch -- [lmbox get $name type] {
     togglebutton {
        set search($name) [lmbox get $name checked]
     }
     textfield {
        set search($name) [lmbox get $name text]
     }
     listbox {
        set items ""
        set data [lmbox get $name selectedtext]
        foreach item [lmbox_var search($name)] {
          if { $item == $data } {
            set data ""
          } else {
            if { $item != "None" } {
              lappend items $item
            }
          }
        }
        if { $data != "" } { lappend items $data }
        set search($name) $items
        lmbox set ${name}_filter caption [lmbox_nvl $items None]
     }
    }
    lmbox varset $filter [array get search]
}

# Load config settings
proc lmbox_initconfig {} {

    lmbox dbeval config {
      lmbox setconfig $lmbox_dbkey $lmbox_dbdata
      # Per object configuration
      switch -glob -- $lmbox_dbkey {
       tv* -
       radio* -
       sound* -
       player* {
          set obj [lindex [split $lmbox_dbkey _] 0]
          set key [lindex [split $lmbox_dbkey _] 1]
          if { [lmbox exists $obj $key] } { lmbox set $obj $key $lmbox_dbdata }
       }
      }
    }
}

proc lmbox_initvideo {} {

    set tv_out [lmbox config tv_out]
    lmbox puts "Setting TV-OUT to $tv_out ..."
    switch -- $tv_out {
     S-Video {
       set cmd "/usr/bin/nvtv -r 640,480 -s Normal -t -S NTSC --set colorfix:on --set flicker:98"
       if { [catch { exec $cmd } errmsg] } {
         lmbox puts $errmsg
       }
     }
    }
}

# Set config entry and save in the database
proc lmbox_setconfig { name value } {

    lmbox setconfig $name $value
    lmbox dbput config $name $value
}

# Formats given size in bytes into more user-friendly size text
proc lmbox_size { size } {

    if { $size > 1073741824 } {
      set size "[format "%.1f" [expr $size / 1073741824.0]]G"
    } elseif { $size > 1048576 } {
      set size "[format "%.1f" [expr $size / 1048576.0]]M"
    } elseif { $size > 1024 } {
      set size "[format "%.1f" [expr $size/1024.0]]K"
    }
    return $size
}

proc lmbox_date { name args } {

    switch -- $name {
     nextMonth {
        foreach { mon year } [clock format [lindex $args 0] -format "%m %Y"] {}
        set date [clock scan "$mon/[lmbox_date daysInMonth $mon $year]/$year 12:0"]
        return [expr $date+86400]
     }
     prevMonth {
        foreach { mon year } [clock format [lindex $args 0] -format "%m %Y"] {}
        set date [clock scan "$mon/01/$year 12:0"]
        return [expr $date-86400]
     }
     dayOfWeek {
        set secs [clock scan "[lindex $args 1]/[lindex $args 0]/[lindex $args 2]"]
        return [clock format $secs -format "%w"]
     }
     leapYear {
        set year [lindex $args 0]
        if { ![string is integer -strict $year] } { return 0 }
        if { (![expr $year % 4] && [expr $year % 100]) || ![expr $year % 400] } { return 1 }
        return 0
     }
     daysInMonth {
        set monthDays { 0 31 28 31 30 31 30 31 31 30 31 30 31 }
        set month [string trimleft [lindex $args 0] 0]
        if { ![string is integer -strict $month] } { return 0 }
        set days [lindex $monthDays $month]
        if { $month == 2 && [lmbox_date leapYear [lindex $args 1]] } { incr days }
        return $days
     }
     monthArray {
        set month [lindex $args 0]
        set year [lindex $args 1]
        set dm [lmbox_date daysInMonth $month $year]
        set dw [expr ([lmbox_date dayOfWeek 1 $month $year] + 7) % 7]
        set day 1
        for { set i 0 } { $i < 43 } { incr i } { lappend days "" }
        while { $dm > 0 } {
          set days [lreplace $days $dw $dw [lmbox_pad0 $day 2]]
          incr dm -1
          incr day
          incr dw
        }
        return $days
     }
     weekArray {
        set secs [clock scan "[lindex $args 1]/[lindex $args 0]/[lindex $args 2] 12:0"]
        set dow [expr $secs-(86400*[clock format $secs -format "%w"])]
        for { set i 0 } { $i < 7 } { incr i } {
          lappend days [clock format $dow -format "%m/%d/%Y"]
          incr dow 86400
        }
        return $days
     }
     uptime {
        set uptime [lindex $args 0]
        if { $uptime == "" } { return }
        set days [expr $uptime / 86400]
        set uptime [expr $uptime % 86400]
        set hrs [expr $uptime / 3600]
        set uptime [expr $uptime % 3600]
        set mins [expr $uptime / 60]
        set secs [expr [lindex $args 0] - (($days * 86400) + ($hrs * 3600) + ($mins * 60))]
        set result ""
        if { $days > 0 } {
          append result " $days day"
          if { $days > 1 } { append result s }
        }
        if { $hrs > 0 } {
          append result " $hrs hour"
          if { $hrs > 1 } { append result s }
        }
        if { $mins > 0 } {
          append result " $mins minute"
          if { $mins > 1 } { append result s }
        }
        if { $secs > 0 } {
          append result " $secs second"
          if { $secs > 1 } { append result s }
        }
        return $result
     }
    }
}

# Creates a text file.
#  path The absolute path to the file
#  data to be saved
proc lmbox_putfile { file data } {

    if { [catch {
      set fd [open $file w]
      fconfigure $fd -translation binary -encoding binary
      puts -nonewline $fd $data
      close $fd
    } errmsg] } {
      lmbox puts "lmbox_putfile: $errmsg"
      catch { close $fd }
      return -1
    }
    return 0
}

# Returns contents of the file
proc lmbox_getfile { file } {

    set data ""
    if { [catch {
      set fd [open $file]
      set data [read $fd]
      close $fd
    } errmsg] } {
      lmbox puts "lmbox_getfile: $errmsg"
      catch { close $fd }
    }
    return $data
}

proc lmbox_geturl { url args } {

    set data ""
    set headers ""
    set timeout [lmbox config http_timeout 30]
    foreach { key val } $args {
      switch -- $key {
       -timeout { set timeout $val }
       -headers { set headers $val }
       -outheaders {
         upvar 1 $val outheaders
         set outheaders ""
       }
      }
    }
    for { set i 0 } { $i < 5 } { incr i } {
      # Keep host in case of relative redirects
      if { ![regexp {^([^:]+://[^/]+)/} $url d host] } {
        lmbox_puts "lmbox_geturl: bad url: $url"
        return
      }
      set data [lmbox geturl $url -headers $headers -timeout $timeout -outheaders outheaders -status status]
      switch -- $status {
       301 - 302 {
         foreach { key val } $outheaders {
          if { $key == "location" && $val != "" } {
            set url $val
            # Relative url, use previous host
            if { ![string match http://* $url] } {
              set url $host/[string trimleft $url /]
            }
            break
          }
         }
       }
       default { break }
      }
    }
    return $data
}

# Called when Tcl interpreter are about to be returned to interp pool for reuse
proc lmbox_tclcleanup {} {

    foreach f [file channels] {
      if { ![regexp {^std} $f] } { catch { close $f } }
    }
    foreach g [info vars] {
      if { ![regexp {^auto_|^tcl_|^env} $g] } { catch { unset $g } }
    }
}

proc lmbox_logsetup {} {

    set log ""
    set count 0
    set now [expr [clock seconds] - 86400*2]
    lmbox dbeval log {
      lappend $log "[clock format $lmbox_dbkey]: $lmbox_dbdata"
      incr count
      if { $count >= 500 } { break }
    } -last -desc
    lmbox set logtext caption [join $log "\n--\n"]
}

# Logging of events
proc lmbox_log { type data } {

    set timestamp [clock seconds]
    lmbox dbput log $timestamp "$type $data"
    lmbox puts "$type: $data"
}

# Output the message to the log and status label
proc lmbox_puts { msg } {

    lmbox puts $msg
    lmbox set status caption $msg
}

# Display info page with specified text and optionally play sound
proc lmbox_alarm { title text { icon "" } { sound "" } } {

    lmbox set infotitle caption $title
    lmbox set infotext caption $text
    set icon [lmbox_nvl $icon none.gif]
    lmbox call infoicon zoomimage $icon 0 0
    lmbox setpage infopage
    # Check if we can play specified sound file
    if { $sound != "" } {
      if { ![file exists $sound] } {
        set sound [lmbox config sound_path sounds]/$sound
      }
      if { [file exists $sound] } {
        lmbox call sound play $sound
      }
    }
}

# Display urgent message
proc lmbox_msg { title text { icon "" } { sound "" } } {

    switch -- [lmbox getpage] {
     videopage {
       player_osdtext 10 10 "$title\n$text" 10000
     }
     default {
       lmbox_alarm $title $text $icon $sound
     }
    }
}

proc lmbox_dblist { dbname } {

    lmbox dbeval $dbname {
      lmbox puts "$lmbox_dbkey: $lmbox_dbdata"
    }
}

# Delete all keys from specified database that starts with key
proc lmbox_dbdel { db key } {

    lmbox dbeval $db { return lmbox_delete } -start $key -beginwith $key
}

proc lmbox_dbdump { dbname { file "" } } {

    set result ""
    lmbox dbeval $dbname {
      lappend result $lmbox_dbkey $lmbox_dbdata
    }
    if { $file != "" && $result != "" } {
      set fd [open $file w]
      foreach { key data } $result {
        if { $key != "" } { puts $fd "$key|$data" }
      }
      close $fd
      set result ""
    }
    return $result
}

proc lmbox_dbload { dbname file { lines "" } } {

    if { $file != "" && [file exists $file] } {
      if { [catch {
        set lines ""
        set fd [open $file]
        foreach line [split [read $fd] "\n"] {
          set line [split $line |]
          lappend lines [lindex $line 0] [join [lrange $line 1 end] |]
        }
        close $fd
      } errmsg] } {
        lmbox puts "lmbox_dbload: $file: $errmsg"
        return
      }
    }
    foreach { key data } $lines {
      lmbox dbput $dbname $key $data
    }
    return
}

# Save databases into text files
proc lmbox_dbsave {} {

    if { ![file isdirectory ~/.lmbox] } {
      file mkdir -force ~/.lmbox
      return
    }
    foreach dbname "config system calendar favorites [lmbox var lmbox:dbsave:list]" {
      lmbox_dbdump $dbname ~/.lmbox/$dbname.txt
    }
}

proc lmbox_mosaic { prefix type args } {

    set event [lmbox getevent]
    set this [lmbox get this name]
    set focused [lmbox get focusedwidget name]
    set num1 [string index $this end-1]
    set num2 [string index $this end]
    set offset [lmbox var ${prefix}_offset 0]
    set ncols [lmbox config ${prefix}_ncols 4]
    set nrows [lmbox config ${prefix}_nrows 2]
    set count [lmbox get ${prefix}_list count]
    set plen [expr [string length $prefix] + 1]

    switch -- $type {
     action {
        switch -glob -- $event {
         up -
         down -
         next -
         prev -
         prevpage -
         nextpage -
         accept {
           return [lmbox_mosaic $prefix $event]
         }
         default {
           return lmbox_ignore
         }
        }
     }

     prevpage {
        if { $offset-$nrows*$ncols >= 0 } {
          incr offset -[expr $nrows*$ncols]
          lmbox varset ${prefix}_offset $offset
          lmbox_mosaic $prefix update
        }
     }

     nextpage {
        if { $offset+$nrows*$ncols < $count } {
          incr offset [expr $nrows*$ncols]
          lmbox varset ${prefix}_offset $offset
          lmbox_mosaic $prefix update
        }
     }

     up {
        if { $num1 > 0 } {
          lmbox call ${prefix}_[incr num1 -1]$num2 setfocus
        } else {
          if { $offset > 0 } {
            incr offset -1
            lmbox varset ${prefix}_offset $offset
            lmbox_mosaic $prefix update
          }
        }
     }

     down {
        if { $num1 < $nrows-1 } {
          lmbox call ${prefix}_[incr num1]$num2 setfocus
        } else {
          if { $offset < $count } {
            incr offset
            lmbox varset ${prefix}_offset $offset
            lmbox_mosaic $prefix update
          }
        }
     }

     next {
        # Jump to the next widget on the page
        if { $offset+$ncols*$nrows > $count } {
          return lmbox_ignore
        }
        if { $num2 < $ncols-1 } {
          lmbox call ${prefix}_$num1[incr num2] setfocus
        } else {
          if { $num1 < $nrows-1 } {
            lmbox call ${prefix}_[incr num1]0 setfocus
          } elseif { $offset < $count } {
            incr offset
            lmbox varset ${prefix}_offset $offset
            lmbox_mosaic $prefix update
          }
        }
     }

     prev {
        # Jump to the next widget on the page
        if { $offset == 0 && $num1+$num2 == 0 } {
          return lmbox_ignore
        }
        if { $num2 > 0 } {
          lmbox call ${prefix}_$num1[incr num2 -1] setfocus
        } else {
          if { $num1 > 0 } {
            lmbox call ${prefix}_[incr num1 -1][expr $ncols-1] setfocus
          } elseif { $offset > 0 } {
            incr offset -1
            lmbox varset ${prefix}_offset $offset
            lmbox_mosaic $prefix update
          }
        }
     }

     accept {
        sound_click
        ${prefix}_play [lmbox get this data]
     }

     update {
        set offset [lmbox_nvl [lindex $args 0] $offset]
        if { $offset >= $count } {
          set offset [expr $count - $nrows*$ncols]
        }
        if { $offset < 0 } {
          set offset 0
        }
        lmbox varset ${prefix}_offset $offset
        lmbox set ${prefix}_scroll value $offset
        lmbox set ${prefix}_list selectedindex $offset
        for { set i 0 } { $i < $nrows } { incr i } {
          for { set j 0 } { $j < $ncols } { incr j } {
            set name ${prefix}_$i$j
            if { $offset < $count } {
              set id [lmbox call ${prefix}_list itemdata $offset]
              set label [lmbox call ${prefix}_list itemtext $offset]
              set bg [lmbox_nvl [${prefix}_icon $id $label] menu/music.jpg]
              set selectable 1
              set borderstyle normal
            } else {
              set id ""
              set label ""
              set bg none.gif
              set selectable 0
              set borderstyle none
            }
            set area [lmbox get $name origarea]
            if { $name == $focused } {
              if { $id == "" } {
                lmbox call [lmbox getpage] updatefocus -1
              }
            }
            lmbox set label_$i$j caption $label
            lmbox set $name selectable $selectable data $id borderstyle $borderstyle area $area
            lmbox call $name zoomimage $bg 0 0
            incr offset
          }
        }
        if { [lmbox get focusedwidget type] == "picture" && $count > 0 } {
          lmbox_mosaic $prefix ongotfocus
        }
     }

     ongotfocus {
        set id [lmbox get focusedwidget data]
        lmbox varset ${prefix}_id $id
        ${prefix}_info status $id
        lmbox_mosaic $prefix focus
     }

     onlostfocus {
        if { [lmbox get this type] == "picture" } {
          lmbox set this area [lmbox get focusedwidget origarea]
          lmbox call this reloadimage
          lmbox set label_[string range $this $plen end] fontcolor #a6d1d9
        }
     }

     onmouseover {
        if { $this != $focused } {
          lmbox set label_[string range $this $plen end] fontcolor #FFFFFF
        }
     }

     onmouseout {
        if { $this != $focused } {
          lmbox set label_[string range $this $plen end] fontcolor #a6d1d9
        }
     }

     focus {
        if { [lmbox get focusedwidget type] == "picture" } {
          set step [lindex $args 0]
          set origarea [lmbox get focusedwidget origarea]
          if { $origarea == [lmbox get focusedwidget areaorig] } {
            sound_click
            lmbox set focusedwidget area $origarea
            lmbox set focusedwidget borderstyle none
            lmbox call focusedwidget transformscale 1 2 1
            lmbox set focusedwidget borderstyle normal
            lmbox set label_[string range $focused $plen end] fontcolor #c5cd4a
          }
        }
     }

     init {
        # Initialization, called in OnGotFocus of the page
        # Filter textfield name and current value
        set filter [lindex $args 0]
        set refresh [lindex $args 1]
        if { $filter != "" } {
          set filter [lmbox get $filter text]
        }
        lmbox_mosaic $prefix setup "" $filter $refresh
        lmbox_mosaic $prefix update
        # Scrollbar setup
        lmbox set ${prefix}_scroll maximum [lmbox get ${prefix}_list count]
        lmbox set ${prefix}_scroll trackingupdate 0
        # Set focus on the first button
        if { [lmbox get focusedwidget type] != "picture" } {
          lmbox call ${prefix}_00 setfocus
        }
     }

     setup {
        # Refresh list according to type and/or filter
        set filter [lindex $args 1]
        set refresh [lindex $args 2]
        switch -- $filter {
         "" - Filter {
           # If not empty, use existing list
           if { [lmbox get ${prefix}_list count] > 0 && $refresh != "-refresh" } {
             return
           }
           set filter ""
         }

         default {
           set filter "\[regexp -nocase {$filter} \$title\]"
         }
        }
        ${prefix}_setup [lindex $args 0] $filter
     }
    }
}
