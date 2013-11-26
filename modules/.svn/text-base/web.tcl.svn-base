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

lmbox_register_init web_init

proc web_init {} {

    # Add menu item
    lmbox_register_menu Info "Web" "Browse the Internet" web.jpg { web_page } 1050
    lmbox_register_stop web_stop
}

proc web_stop {} {

    lmbox call webbrowser close
}

proc web_page { { url "" } { toolbar 1 } { x 0 } { y 0 } { w 0 } { h 0 } } {

    lmbox call webbrowser event toolbar $toolbar
    lmbox call webbrowser move $x $y $w $h
    lmbox set webbrowser location $url
    lmbox setpage webpage
}
