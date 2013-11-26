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

lmbox_register_init fmtuner_init

proc fmtuner_init {} {

    lmbox_register_menu Music "FM Radio" "Play radio tuner" fmradio.jpg { lmbox setpage fmtunerpage } 900
    lmbox_register_stop fmtuner_stop
}

proc fmtuner_info {} {

    lmbox set fmtunerfreq caption [lmbox get radio frequency]
    if { [lmbox get radio stereo] == 1 } {
      lmbox set fmtunerstereo caption Stereo
    } else {
      lmbox set fmtunerstereo caption Mono
    }
    lmbox set fmtunerpreset caption [lmbox var fmtuner_preset]
}

proc fmtuner_preset { id } {

    if { [lmbox var fmtuner_flag] == 1 } {
      lmbox varset fmtuner_flag 0
      set freq [lmbox get radio frequency]
      lmbox_setconfig fmtuner_preset$id $freq
    }
    lmbox varset fmtuner_preset $id
    lmbox set radio frequency [lmbox config fmtuner_preset$id]
    lmbox call radio play
    fmtuner_info
}

proc fmtuner_save {} {

    lmbox varset fmtuner_flag 1
    fmtuner_info
}

proc fmtuner_play {} {

    lmbox_stop
    lmbox call tv mixer unmute 1 ""
    lmbox call radio play
    fmtuner_info
}

proc fmtuner_stop {} {

    lmbox call tv mixer unmute 0 ""
    lmbox call radio stop
    fmtuner_info
}

proc fmtuner_next { step } {

    lmbox set radio frequency [expr [lmbox get radio frequency] + $step]
    lmbox varset fmtuner_preset ""
    fmtuner_info
}

proc fmtuner_seek { step } {

    lmbox call radio seek $step
    lmbox varset fmtuner_preset ""
    fmtuner_info
}
