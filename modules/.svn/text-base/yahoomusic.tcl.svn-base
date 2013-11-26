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

lmbox_register_init yahoomusic_init

proc yahoomusic_init {} {

    lmbox dbopen yahoomusic_genres
    lmbox dbopen yahoomusic_videos
    # Initial data load
    lmbox_register_check YahooMusic yahoomusic_load
    # Main menu entry
    lmbox_register_menu Music "Yahoo Music" "Listen and watch music videos from Yahoo" yahoomusic.jpg { lmbox setpage yahoomusicpage } 690
    # Config page
    lmbox_register_config "Yahoo Music Settings" "lmbox setpage yahoomusicconfigpage"
}

proc yahoomusic_load { { refresh 0 } } {

}

