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

lmbox_register_init shoutcast_init

proc shoutcast_init {} {

    lmbox dbopen shoutcast
    lmbox_register_check Shoutcast shoutcast_check
    lmbox_register_menu Music "Shoutcast.com" "Listen music from shoutcast.com" shoutcast.jpg { lmbox setpage shoutcastpage } 800
}

proc shoutcast_check { { force 0 } } {

    set now [clock seconds]
    set timeout [lmbox config shoutcast_timeout 86400]
    set timestamp [lmbox config shoutcast_mtime 0]
    if { $force || $now - $timestamp >= $timeout } {
      set count 0
      set host http://yp.shoutcast.com
      lmbox_puts "Downloading shoutcast playlist..."
      for { set i 0 } { $i < 100 } { incr i 20 } {
        set data [lmbox_geturl $host/directory/index.phtml?startat=$i]
        while {1} {
          # Look for the url
          if { [set idx [string first {/sbin/shoutcast-playlist.pls?rn=} $data]] == -1 } { break }
          set data [string range $data $idx end]
          if { [set idx [string first > $data]] == -1 } { break }
          set url $host/[string trim [string range $data 0 $idx] { <>""}]
          set data [string range $data $idx end]
          # Look for the genre
          if { [set idx [string first {[} $data]] == -1 } { break }
          set data [string range $data $idx end]
          if { [set idx [string first {]} $data]] == -1 } { break }
          set genre [string trim [string range $data 0 $idx] { []}]
          # Look for the title
          if { [set idx [string first "<a " $data]] == -1 } { break }
          if { [set idx [string first > $data [incr idx]]] == -1 } { break }
          set data [string range $data [incr idx] end]
          if { [set idx [string first < $data]] == -1 } { break }
          if { [set title [string trim [string range $data 0 $idx] { <>""}]] == "" } { continue }
          set shoutcast([incr count]) [list url $url title $title genre $genre]
        }
      }
      if { [array size shoutcast] > 0 } {
        lmbox dbdel shoutcast
        foreach { id value } [array get shoutcast] {
          array set rec $value
          set rec(timestamp) $now
          set rec(title) [string map { \{ {} \} {} ' {} } $rec(title)]
          lmbox dbput shoutcast [format "%05d" $id] [array get rec]
        }
      }
      lmbox_setconfig shoutcast_mtime $now
      lmbox_puts "Shoutcast playlist downloaded, [array size shoutcast] stations"
      lmbox dbsync
      # Refresh GUI with new items
      shoutcast_setup
    }
}

proc shoutcast_setup {} {

    lmbox call shoutcastlist clear
    lmbox dbeval shoutcast {
      lmbox call shoutcastlist additemdata $title $lmbox_dbkey
    } -split
}

proc shoutcast_info { type { id "" } } {

    switch -- $type {
     info {
        array set rec [lmbox dbget shoutcast $id]
        if { [info exists rec(genre)] } {
          lmbox set status caption $rec(genre)
        }
     }

     status {
        if { [lmbox get player status] == "play" } {
          lmbox set status caption [lmbox get player title]
        }
     }
    }
}

proc shoutcast_play { id { url "" } } {

    lmbox_stop
    if { $url == "" } {
      array set rec [lmbox dbget shoutcast $id]
      if { [set url [lmbox_var rec(url)]] == "" } { return }
      if { [set name [lmbox var shoutcast_connecting]] != "" } {
        lmbox set status caption "Still connecting to $name"
        return
      }
      lmbox varset shoutcast_connecting $rec(title)
    }
    set data [lmbox_putfile /tmp/shoutcast.pls [lmbox_geturl $url]]
    lmbox call player clear
    lmbox call player add /tmp/shoutcast.pls
    player_audiofilter player
    lmbox run "shoutcast_connect $id"
    # Add the link temporary to favorite links
    set expires [expr [clock seconds] + [lmbox config recent_lifetime 86400]]
    shoutcast_favorite $id expires $expires class History
}

proc shoutcast_connect { { id "" } } {

    lmbox call player play
    lmbox call player settimer 10000 "shoutcast_info status $id"
    lmbox varset shoutcast_connecting ""
}

proc shoutcast_favorite { id args } {

    set class ""
    set expires 0
    foreach { key val } $args { set $key $val }
    array set rec [lmbox dbget shoutcast $id]
    if { ![info exists rec(url)] } { return }
    favorites_add Music "script:shoutcast_play {} $rec(url)" $rec(title) description $rec(genre) icon images/menu/shoutcast.jpg expires $expires class $class
    lmbox set status caption "$rec(title) added to favorites"
}

proc shoutcast_pause {} {

    lmbox call player pause
}

proc shoutcast_stop {} {

    lmbox call player stop
}

proc shoutcast_next {} {

    lmbox call player next
}
