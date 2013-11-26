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

lmbox_register_init photo_init

proc photo_init {} {

    lmbox varset photo_loop 0
    lmbox varset photo_files ""
    lmbox varset photo_index -1

    lmbox_register_menu Personal "Home Photos" "Watch albums/photos on the screen" photo.jpg { lmbox setpage photopage } 400
}

proc photo_setup { { path "" } } {

    if { $path == "" } { set path [lmbox config photo_path data/Images]/ }
    if { ![file exists $path] } { return empty/ }
    return $path
}

proc photo_exit {} {

    set curpath [lmbox get photofilelist curpath]
    if { [lmbox get photofilelist rootpath] == $curpath } {
      lmbox setpage mainpage
      return
    }
    lmbox call photofilelist choose 0
    lmbox call photofilelist setfocus
}

proc photo_show { file } {

    if { [set count [lmbox get photofilelist count]] == "" } { set count 0 }
    if { [set index [lmbox get photofilelist selectedindex]] == "" } { set index 0 }
    if { $count > 0 } { incr index }
    lmbox set photocount caption "$index of $count"
    lmbox set photostatus caption [file tail [file dirname $file]]/[file tail $file]
    if { [file isdirectory $file] } { return }
    switch -- [lmbox config photo_effects] {
     Fade {
       lmbox call photoimage fadeimage $file
     }

     default {
       lmbox call photoimage zoomimage $file 0 0
     }
    }
}

proc photo_play { { file "" } { name "" } { width "" } { height "" } { loop 0 } } {

    if { $file != "" } {
      lmbox varset photo_index -1
      lmbox varset photo_loop $loop
      set files ""
      foreach file $file {
        lmbox set photostatus caption "Playing [file tail [file dirname $file]]"
        if { ![file isdirectory $file] } { set file [file dirname $file] }
        foreach file [lmbox_files $file] { lappend files $file }
      }
      lmbox varset photo_files [lmbox_shuffle $files]
    }
    lmbox varset photo_index [expr [lmbox var photo_index] + 1]
    set file [lindex [lmbox var photo_files] [lmbox var photo_index]]
    if { $file == "" } {
      if { [lmbox var photo_loop] } {
        lmbox varset photo_index 0
        lmbox varset photo_files [lmbox_shuffle [lmbox var photo_files]]
        set file [lindex [lmbox var photo_files] 0]
      }
    }
    if { $file == "" } { return [photo_stop] }
    if { $name == "" } { set name photoimage }
    if { $width == "" } { set width [lmbox config width] }
    if { $height == "" } { set height [lmbox config height] }
    lmbox call $name zoomimage $file $width $height
    lmbox set photostatus caption [file tail [file dirname $file]]/[file tail $file]
}

proc photo_stop {} {

    lmbox varset photo_loop 0
    lmbox varset photo_files ""
    lmbox varset photo_index -1
    lmbox set photostatus caption "Play stopped"
}
