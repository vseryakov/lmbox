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


lmbox_register_init favorites_init

# Top menu item
proc favorites_init {} {

    lmbox_register_menu Video "Favorites" "Favorite video/movie links" favorite.jpg { lmbox varset favorites_class Video; lmbox setpage favorites_mosaicpage } 1
    lmbox_register_menu Music "Favorites" "Favorite music links" favorite.jpg { lmbox varset favorites_class Music; lmbox setpage favorites_mosaicpage } 3
    lmbox_register_menu TV "Favorites" "Favorite TV links" favorite.jpg { lmbox varset favorites_class TV; lmbox setpage favorites_mosaicpage } 5
    lmbox_register_menu Info "Favorites" "Favorite Web links" favorite.jpg { lmbox varset favorites_class Web; lmbox setpage favorites_mosaicpage } 7
    lmbox_register_menu Info History "Recently played multimedia links" recent.jpg { lmbox varset favorites_class History; lmbox setpage favorites_mosaicpage } 99999
}

proc favorites_setup { { fclass "" } { filter "" } } {

    if { $fclass == "" } {
      set fclass [lmbox var favorites_class]
    }
    lmbox varset favorites_class $fclass

    if { [lmbox get favorites_type count] == 0 } {
      lmbox call favorites_type additem Music
      lmbox call favorites_type additem Video
      lmbox call favorites_type additem Web
      lmbox call favorites_type additem TV

      lmbox call favorites_expires additemdata "Choose new..." 0
      lmbox call favorites_expires additemdata Never 0
      lmbox call favorites_expires additemdata "1 Hour" 3600
      lmbox call favorites_expires additemdata "3 Hours" 10800
      lmbox call favorites_expires additemdata "12 Hours" 43200
      lmbox call favorites_expires additemdata "1 Day" 86400
      lmbox call favorites_expires additemdata "2 Days" 172800
      lmbox call favorites_expires additemdata "3 Days" 259200
      lmbox call favorites_expires additemdata "7 Days" 604800
      lmbox call favorites_expires additemdata "14 Days" 1209600
      lmbox call favorites_expires additemdata "1 Month" 2592000
      lmbox call favorites_expires additemdata "2 Months" 5184000
      lmbox call favorites_expires additemdata "3 Months" 7776000
    }
    set now [clock seconds]
    set selected [lmbox get favorites_list selectedtext]
    lmbox call favorites_list clear
    lmbox dbeval favorites {
      if { [lsearch -exact $fclass [lmbox_var class]] == -1 } {
        continue
      }
      # Additional filter
      if { $filter != "" && ![expr $filter] } { continue }
      # Should not expire
      if { $expires == 0 || $expires > $now } {
        lmbox call favorites_list additemdata $title $lmbox_dbkey
      } else {
        switch -- $class {
         History {}
         default {
            lmbox puts "favorite_setup: $lmbox_dbkey: expires $type: $title, $expires"
            return lmbox_delete
         }
        }
      }
    } -split
    lmbox call favorites_list sort
    lmbox set favorites_list selectedindex [lmbox call favorites_list searchitem $selected]
    # Show details for existing items only
    if { [lmbox get favorites_list count] == 0 } {
      lmbox set favorites_details enabled 0 visible 0
      lmbox set favorites_delete enabled 0 visible 0
      lmbox set favorites_filter enabled 0 visible 0
      set x 200
      set on 0
    } else {
      lmbox set favorites_details enabled 1 visible 1
      lmbox set favorites_delete enabled 1 visible 1
      lmbox set favorites_filter enabled 1 visible 1
      set x 350
      set on 1
    }
    switch -- $class {
     History {
       lmbox set favorites_move enabled $on visible $on x $x
     }

     default {
       lmbox set favorites_move enabled 0 visible 0
     }
    }
    if { $filter != "" } {
      lmbox puts "favorites_setup: $fclass: $filter"
    }
}

proc favorites_play { id } {

    array set rec [lmbox dbget favorites $id]
    if { ![info exists rec(type)] || ![info exists rec(url)] } { return }
    lmbox_stop
    lmbox_puts "Playing $rec(url)..."
    lmbox refresh

    switch -- $rec(type) {
     Web {
        # Start web browser
        web_page $rec(url) 0
     }

     TV -
     Video -
     Radio -
     Music {
        lmbox call player clear
        if { [string match script:* $rec(url)] } {
          # If script is given, let it handle the whole playback
          lmbox run [string range $rec(url) 7 end]
        } else {
          # Otherwise setup player and run it
          foreach url $rec(url) {
            lmbox call player add $url
          }
          switch -- $rec(type) {
           TV -
           Video {
             lmbox setpage videopage
           }
           default {
             lmbox run "lmbox call player play"
           }
          }
        }
     }

     default {
        lmbox set status caption "Unknown type $rec(type): $rec(url)"
     }
    }
}

proc favorites_info { type id } {

    array set rec [lmbox dbget favorites $id]
    if { [info exists rec(type)] } {
      set info "$rec(type): [lmbox_nvl $rec(description) $rec(url)]"
      if { [lmbox_var rec(expires) 0] > 0 } { append info ". Expires [lmbox_clock $rec(expires)]" }
      if { [lmbox_var rec(timestamp) 0] > 0 } { append info ". Watched [lmbox_clock $rec(timestamp)]" }
      if { [lmbox_var rec(icon)] == "" } {
        set rec(icon) [favorites_icon $id $rec(title)]
      }
    }
    set rec(icon) [lmbox_var rec(icon) menu/music.jpg]
    lmbox set status caption [lmbox_var info "No favorite links"]
    switch -- $type {
     info {
       lmbox call favorites_image zoomimage $rec(icon) 0 0
     }
    }
}

proc favorites_icon { id { title "" } } {

    return [lmbox_icon $title [lmbox gethome]/covers [lmbox config itv_path data/iTV] [lmbox config cover_path data/Movies] ""]
}

proc favorites_add { type url title args } {

    foreach { key val } { id "" expires 0 description "" icon "" class "" } { set rec($key) $val }
    foreach { key val } $args { set rec($key) $val }
    # Check for existing record by url
    if { $rec(id) == "" } {
      lmbox dbeval favorites {
        if { [lmbox_var rec2(url)] == $url && [lmbox_var rec2(class)] == $rec(class) } {
          set rec(id) $lmbox_dbkey
          break
        }
      } -array rec2
    }
    if { $rec(id) == "" } {
      set rec(id) [lmbox dbnextid favorites_sequence]
    } else {
      # Read existing record
      array set rec [lmbox dbget favorites $rec(id)]
      # Update fields new values only
      foreach { key val } $args { set rec($key) $val }
    }
    set rec(url) $url
    set rec(type) $type
    set rec(title) $title
    set rec(timestamp) [clock seconds]
    set rec(class) [lmbox_nvl $rec(class) $rec(type)]
    lmbox dbput favorites $rec(id) [array get rec]
    lmbox puts "favorites_add: [array get rec]"
    return $rec(id)
}

proc favorites_save { id } {

    set type [lmbox get favorites_type selectedtext]
    set class [lmbox get favorites_class caption]
    set expires [lmbox get favorites_expires selecteddata]
    set url [lmbox get favorites_url text]
    set title [lmbox get favorites_title text]
    set descr [lmbox get favorites_description text]
    if { $type == "" || $url == "" || $title == "" } {
      lmbox set status caption "All fields should be filled"
      return
    }
    # Set expiration date in the future
    if { $expires > 0 } { incr expires [clock seconds] }
    favorites_add $type $url $title id $id description $descr expires $expires class $class
    lmbox setpage favorites_mosaicpage
}

proc favorites_edit { id } {

    array set rec { url "" title "" description "" type Web expire 0 expires 0 icon "" class "" }

    if { $id != "" } {
      array set rec [lmbox dbget favorites $id]
      if { ![info exists rec(type)] } { return }
    } else {
      set rec(type) [lmbox var favorites_class Web]
      set rec(class) [lmbox var favorites_class Web]
    }
    lmbox set favorites_url text $rec(url)
    lmbox set favorites_title text $rec(title)
    lmbox set favorites_description text $rec(description)
    lmbox set favorites_type selectedindex [lmbox call favorites_type searchitem $rec(type)]
    lmbox set favorites_class caption $rec(class)
    lmbox set favorites_expires selectedindex 0
    lmbox set favorites_expires2 caption ""
    if { $rec(expires) > 0 } {
      lmbox set favorites_expires2 caption "Set: [lmbox_clock $rec(expires)]"
    }
    if { $rec(icon) == "" } {
      set rec(icon) [favorites_icon $id $rec(title)]
    }
    lmbox call favorites_icon zoomimage [lmbox_var rec(icon) none.gif] 0 0
    lmbox set favorites_editpage data $id
    lmbox setpage favorites_editpage
}

proc favorites_delete { id } {

    if { $id != "" } {
      lmbox dbdel favorites $id
    }
    lmbox setpage favorites_mosaicpage
}

proc favorites_move { id } {

    array set rec [lmbox dbget favorites $id]
    if { ![info exists rec(type)] } { return }
    set rec(class) $rec(type)
    lmbox dbput favorites $id [array get rec]
    lmbox set status caption "$rec(title) moved to favorites"
    favorites_setup
}

