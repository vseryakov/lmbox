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

proc idle_setup {} {

    if { [lmbox var idle_flag 0] } { return }
    lmbox varset idle_flag 1
    photo_play "images/bg [lmbox config idle_path [lmbox config photo_path data/Images]]" idleimage [lmbox config width] [lmbox config height] 1
    lmbox varset idle_page [lmbox getpage]
    lmbox setpage idlepage
}

proc idle_exit {} {

    if { [set page [lmbox var idle_page]] == "" } { set page mainpage }
    lmbox setpage $page
}

proc idle_play {} {

    photo_play {} idleimage [lmbox config width] [lmbox config height]
}

proc idle_stop {} {

    lmbox varset idle_flag 0
    photo_stop
}
