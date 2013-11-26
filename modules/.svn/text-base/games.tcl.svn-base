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

lmbox_register_init games_init

proc games_init {} {

    # Open games database
    lmbox dbopen games
    # Add main menu item
    lmbox_register_menu Fun "Games" "Play flash games" game.jpg { lmbox setpage gamespage } 900
    # Check for new games
    lmbox_register_check Games games_load
}

proc games_play { file } {

    set games_path [file normalize [lmbox config games_path data/Games]]
    if { [set rec [lmbox dbget games $file]] != "" } {
      array set game $rec
      incr game(counter)
      lmbox dbput games $file [array get game]
    }
    lmbox_putfile /tmp/game.html "<EMBED SRC=$games_path/$file AUTOSTART=TRUE WIDTH=100% HEIGHT=100% LOOP=TRUE>"
    lmbox puts "games: playing $file"
    # If embedded browser is not compiled in, check for external flash player
    switch -- [lmbox config games_player embed] {
     embed -
     lmboxweb {
       if { [lmbox get webbrowser version] == "" } {
         lmbox setconfig games_player glashplayer
         lmbox puts "webbrowser is not available, using gflashplayer"
       }
     }
    }
    switch -- [set player [lmbox config games_player embed]] {
     embed {
        web_page file:///tmp/game.html 1
     }

     default {
        lmbox exec "$player $games_path/$file"
     }
    }
}

proc games_cancel {} {

    switch -- [set player [lmbox config games_player embed]] {
     embed {
        lmbox call webbrowser close
     }

     default {
        catch { exec killall -9 $player }
     }
    }
    lmbox setpage mainpage
}

proc games_setup { { filter "" } } {

    lmbox call gameslist clear
    lmbox dbeval games {
      lmbox call gameslist additemdata "$counter. $title" $lmbox_dbkey
    } -split 1
}

proc games_info { file } {

    set games_path [lmbox config games_path data/Games]
    set icon [lmbox_icon [file root $file] $games_path]
    lmbox call gamesicon zoomimage [lmbox_nvl $icon none.gif] 0 0
    set count [lmbox get gameslist count 0]
    set index [lmbox get gameslist selectedindex 0]
    if { $count > 0 } { incr index }
    lmbox set gamescount caption "$index of $count"
}

proc games_load { args } {

    set games_path [lmbox config games_path data/Games]
    set files [lmbox_files $games_path {\.(swf)$}]
    set games_count [lmbox dbstat games count]
    if { [llength $files] == $games_count } {
      return
    }
    set count 0
    lmbox_puts "Loading games..."
    foreach file $files {
      set title [string totitle [string map { - { } _ { } } [file rootname [file tail $file]]]]
      if { [string is integer $title] } {
        set title "Game-$title"
      }
      set file [file tail $file]
      if { ![lmbox dbexists games $file] } {
        set game(title) $title
        set game(counter) 0
        lmbox dbput games $file [array get game]
        incr count
      }
    }
    lmbox_puts "$count games loaded"
    lmbox dbsync
}
