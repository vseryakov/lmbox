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

lmbox_register_init config_init

# Load last saved config snapshot if database got re-created
proc config_init {} {

    set count [lmbox dbstat config count]
    if { $count <= 0 } {
      lmbox puts "restoring databases..."
      lmbox_dbload config ~/.lmbox/config.txt
      lmbox_dbload system ~/.lmbox/system.txt
      lmbox_dbload calendar ~/.lmbox/calendar.txt
      lmbox_dbload favorites ~/.lmbox/favorites.txt
      lmbox_initconfig
    }
    # Common icons directory
    catch { file mkdir [lmbox gethome]/icons }
    # Register config pages
    lmbox_register_config "General Settings" "lmbox setpage mainconfigpage"
    lmbox_register_config "Path Settings" "lmbox setpage pathconfigpage"
    lmbox_register_config "Reload lmbox config" "lmbox_initconfig;lmbox_puts {Config file has been reloaded}"
}

proc config_setup {} {

    set now [clock seconds]
    if { [lmbox getpage] != "mainpage" ||
         ([lmbox config admin_password] != "" && $now - [lmbox var admin_time 0] > 60) } {
      lmbox set status caption "Please, enter the admin password"
      return
    }
    if { [lmbox get idle_shutdown count] == 0 } {
      for { set i 0 } { $i <= 60 } { incr i } { lmbox call idle_shutdown additem $i }
    }
    if { [lmbox get music_effects count] == 0 } {
      lmbox call music_effects additemdata None None
      lmbox call music_effects additemdata "Display Photos" photos
      lmbox call music_effects additemdata "Goom Effects" goom
    }
    if { [lmbox get photo_effects count] == 0 } {
      lmbox call photo_effects additemdata None None
      lmbox call photo_effects additemdata Fade Fade
    }
    foreach { name value } [lmbox listconfig] {
      switch -- [lmbox get $name type] {
       textfield { lmbox set $name text $value }
       togglebutton { lmbox set $name checked $value }
       listbox {
         if { [lmbox call $name itemdata 0] != "" } {
           lmbox set $name selectedindex [lmbox call $name searchitemdata $value]
         } else {
           lmbox set $name selectedindex [lmbox call $name searchitem $value]
         }
       }
      }
    }
    lmbox call configlist clear
    foreach { title page } [lmbox var lmbox:config:list] {
      lmbox call configlist additemdata $title $page
    }
    lmbox setpage configpage
}

# Saves all fields from the current config page
proc config_save {} {

    foreach name [lmbox get [lmbox getpage] widgetlist] {
      switch -- [lmbox get $name type] {
       textfield {
         set value [lmbox get $name text]
       }
       togglebutton {
         switch -- [lmbox get $name checked] {
          true { set value 1 }
          default { set value 0 }
         }
       }
       listbox {
         if { [set value [lmbox get $name selecteddata]] == "" } {
           set value [lmbox get $name selectedtext]
         }
       }
       default { continue }
      }
      lmbox setconfig $name $value
      lmbox dbput config $name $value
    }
    lmbox dbsync
    # Save config snapshot
    lmbox_dbdump config ~/.lmbox/config.txt
}
