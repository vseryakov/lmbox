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

# Main menu items, various modules will register their own menu items with different
# sorting order, we just register main top and bottom menu items
# The following categories are predefined:
#  Video Music Fun Info Contacts System
#
proc lmbox_init_menu {} {

    lmbox_register_menu Video "Play DVD/CD" "Play DVD/CD/VCD/MP3 disk" dvd.jpg  { disk_play } 1
    lmbox_register_menu System "Msg Log" "Messages and alarms log" log.jpg { lmbox setpage logpage } 999999990
    lmbox_register_menu System "Settings" "Configuration settings" settings.jpg { config_setup } 999999991
    lmbox_register_menu System "Shutdown" "Restart/shutdown lmbox" shutdown.jpg { lmbox setpage shutdownpage } 999999999
}

# Automatically called on lmbox start
proc lmbox_init {} {

    # Load config into memory
    lmbox_initconfig

    # Setup video resolution
    lmbox_initvideo

    # lmbox checker, internal housekeeping
    lmbox schedule "lmbox_housekeeping" -interval 60

    # lmbox modules checker, runs checks for each registered module
    lmbox schedule "lmbox_check" -thread 1 -interval 1 -once 1
    lmbox schedule "lmbox_check" -thread 1 -interval [lmbox config check_interval 900]

    # Send alert message to the screen
    lmbox_register_callerid lmbox_msg

    # Setup calendar alerts as regular schedule items
    if { [catch { calendar_schedule } errmsg] } {
      lmbox puts $::errorInfo
    }

    # Run HTTP server, both TCP and UDP ports
    if { [set port [lmbox config http_port 80]] > 0 } {
      lmbox listen $port http_handler -thread 1 -onconnect {puts $lmbox_fd {LMBOX version [lmbox version]}}
      lmbox listen $port http_handler -udp 1
    }

    # Run modem CallerID detection listener
    if { [set modem [lmbox config modem_device]] != "" } {
      lmbox listen $modem modem_read -oninit modem_init
    }

    # Main menu setup
    lmbox_init_menu

    # Run registered init scripts
    foreach name [lmbox var lmbox:init:list] {
      set module [string map { _init {} } $name]
      if { [lmbox_true [lmbox config ${module}_disabled 0]] } {
        continue
      }
      if { [catch { eval $name } errmsg] } {
        lmbox puts "[lmbox_clock]: $name: $errmsg"
      }
    }

    # Show what is scheduled
    lmbox puts [lmbox listenlist]
    lmbox puts [lmbox schedulelist]
}

# Automatically called on lmbox exit
proc lmbox_close {} {

}

# Called on system object start
proc lmbox_onstart {} {

    # Start schedule thread
    lmbox scheduleinit

    # Start listener thread
    lmbox listeninit
}

# Called on system object stop
proc lmbox_onstop {} {

    if { [lmbox listenclose] } {
      lmbox puts "lmbox_onstop: timeout waiting listener thread exit"
    }
    if { [lmbox scheduleclose] } {
      lmbox puts "lmbox_onstop: timeout waiting scheduler thread exit"
    }
    if { [lmbox threadclose] } {
      if { [set threads [lmbox threadlist]] != "" } {
        lmbox puts "lmbox_onstop: still running threads: $threads"
      }
    }
    disk_umount
    lmbox_stop
}

# Adds menu items into main menu
proc lmbox_register_menu { category title descr icon script { sort 0 } } {

    set list [lmbox var lmbox:menu:list]
    # Check for duplicates
    foreach { key val } $list {
      if { $key == $title && [lindex $val end] == $category } { return }
    }
    lappend list $title [list $sort $descr $icon $script $category]
    lmbox varset lmbox:menu:list $list
}

# Generic callback registration helper
proc lmbox_register_callback { name args } {

    set list [lmbox var lmbox:$name:list]
    # Check for duplicates
    if { [lsearch -exact $list [lindex $args 0]] == -1 } {
      foreach arg $args { lappend list $arg }
    }
    lmbox varset lmbox:$name:list $list
}

# Registers stop callback which will be called by lmbox_stop.
# This is used to stop running media applications if necessary
# to run something which may conflict, like switching to listening music
# after watching the movie
proc lmbox_register_stop { name } {

    lmbox_register_callback stop $name
}

# Registers configuration page that will be called on the main settings page
proc lmbox_register_config { name script } {

    lmbox_register_callback config $name $script
}

# Registers check routine that will be called every once in a while, module can register
# its checker and it will be called every 30 mins or accordint to check_interval parameter
proc lmbox_register_check { name script } {

    lmbox_register_callback check $name $script
}

# Registers init routine that will be called only once on lmbox startup
proc lmbox_register_init { name } {

    lmbox_register_callback init $name
}

# Registers callerid notifier callback to be called on each incoming phone call
proc lmbox_register_callerid { name } {

    lmbox_register_callback callerid $name
}

# Registers database that will be saved periodically into text file
proc lmbox_register_dbsave { name } {

    lmbox_register_callback dbsave $name
}
