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

lmbox_register_init music_init

proc music_init {} {

    lmbox_register_menu Music "All Music" "Play your music collection" music.jpg { lmbox setpage musicpage } 200
    lmbox_register_stop music_stop
}

proc music_setup { { path "" } } {

    if { $path == "" } { set path [lmbox config music_path data/Music]/ }
    return $path
}

proc music_volume { level } {

    set volume_type [lmbox config volume_type amplevel]

    lmbox set player $volume_type [expr [lmbox get player $volume_type] + $level]
    set level [lmbox get player $volume_type]
    lmbox set status caption "Volume: $level"
}

proc music_shuffle { { shuffle "" } } {

    if { $shuffle == "" } { set shuffle [expr 1-[lmbox get player shuffle]] }
    lmbox set player shuffle $shuffle
    if { $shuffle } {
      lmbox set shuffle caption Unshuffle
    } else {
      lmbox set shuffle caption Shuffle
    }
}

proc music_info { type { file "" } } {

    switch -- $type {
     count {
        set count [lmbox get musicfilelist count]
        lmbox set musiccount caption "$count entries"
     }

     status {
        if { [lmbox get player status] == "play" } {
          lmbox set status caption [player_status]
        }
     }

     scroll {
        set lineoffset [expr [lmbox get musicinfo lineoffset]+3]
        if { $lineoffset > [lmbox get musicinfo linecount] } { set lineoffset 0 }
        lmbox set musicinfo lineoffset $lineoffset
        lmbox call musicicon reloadimage
     }

     info {
        if { [regexp [music_info filetypes] $file] } { return }
        set info ""
        if { [file isdirectory $file] } {
          if { [set data [lmbox var music:size:$file]] == "" } {
            set files [lmbox_files [list $file]]
            set size [llength $files]
            set data "$size {[lindex $files 0]}"
            lmbox varset music:size:$file $data
          }
          set file [lindex $data 1]
          append info "Music files: [lindex $data 0]\n"
        }
        if { [set data [lmbox var music:info:$file]] == "" } {
          set data [lmbox call player info $file]
          lmbox varset music:info:$file $data
        }
        append info "File: [file tail $file]\nUpdated: [clock format [file mtime $file]]\n$data"
        return $info
     }

     icon {
        # Check file icon and then directory icon
        set icon [lmbox_icon [file tail $file] $file]
        if { $icon == "" } {
          set icon [lmbox_icon [file tail [file dirname $file]] [file dirname $file]]
        }
        return $icon
     }

     file {
        if { [regexp [music_info filetypes] $file] } { return }
        set icon [music_info icon $file]
        set info [music_info info $file]
        set data ""
        foreach line [split $info "\n"] {
          if { [set line [split $line :]] == "" } { continue }
          append data "^#61BF5F[lindex $line 0]:^#c5cd4a [join [lrange $line 1 end] :]\n"
        }
        lmbox set musicinfo caption $data
        lmbox set musicicon enabled 0
        if { $icon != "" } {
          lmbox call musicicon zoomimage $icon 0 0
          lmbox set musicicon enabled 1
        }
     }

     filetypes {
        return {\.mp3$|\.wav$|\.wma$|\.ogg$}
     }
    }
}

proc music_play { { file "" } } {

    set file [lmbox_nvl $file [lmbox get musicfilelist filename]]
    if { $file == "" } { return }
    lmbox call player stop
    lmbox call player clear
    lmbox call player addfiles $file [lmbox config music_files [music_info filetypes]]
    lmbox call player settimer 10000 "music_info status"
    player_audiofilter
    lmbox call player play
    # Add the link temporary to favorite links
    set expires [expr [clock seconds] + [lmbox config recent_lifetime 86400]]
    music_favorite $file expires $expires class History
    # In case of video files switch to player page
    if { [lmbox get player hasvideo] == 1 } {
      lmbox setpage videopage
    }
}

proc music_favorite { file args } {

    set class ""
    set expires 0
    foreach { key val } $args { set $key $val }

    set description ""
    set title [file tail $file]
    set info [music_info info $file]
    foreach line [split $info "\n"] {
      set line [string trim $line]
      switch -regexp -- $line {
       {:$} {}
       {^Music|^Album|^Artist} { append description $line " " }
      }
    }
    favorites_add Music $file $title description $description icon [music_info icon $file] expires $expires class $class
    lmbox set status caption "$title added to favorites"
}

proc music_stop {} {

    lmbox call player stop
}

proc music_next {} {

    lmbox call player next
}

proc music_prev {} {

    lmbox call player prev
}

proc music_pause {} {

    lmbox call player pause
}

proc music_mute {} {

    lmbox set player mute [expr 1 - [lmbox get player mute]]
}

proc music_select {} {

    if { [set name [lmbox var music_select]] != "" } {
      lmbox set $name caption [lmbox get musicfilelist filename]
    }
    lmbox setpage [lmbox prevpage]
}


