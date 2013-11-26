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

lmbox_register_init sound_init

proc sound_init {} {

    lmbox varset sound:list [glob -nocomplain [lmbox config sound_path sounds/*.wav]]
    lmbox set sound volume [lmbox config sound_volume 96]
}

proc sound_play { { file "" } } {

    if { $file != "" } {
      lmbox call sound play $file
      return
    }
    # Otherwise play random sounds file
    if { [lmbox_true [lmbox config sound_effects 0]] &&
         [set sound [lmbox var sound:list]] != "" &&
         [set file [lindex $sound [expr round(rand()*100) % [llength $sound]]]] != "" } {
      lmbox call sound play $file
    }
}

proc sound_back {} {

   lmbox call sound play sounds/back.wav
}

proc sound_click {} {

   lmbox call sound play sounds/click.wav
}

proc sound_error {} {

   lmbox call sound play sounds/error.wav
}
