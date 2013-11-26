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

proc menu_setup { { category "" } } {

    if { ![lmbox get menulist count] } {
      lmbox set menulist flags !seekable
      # Sort menu items by sort field
      foreach { title data } [lmbox var lmbox:menu:list] {
        set sort [lindex $data 0]
        lappend sorted [list $sort $title $data]
      }
      set sorted [lsort -integer -index 0 $sorted]
      lmbox varset lmbox:menu:sorted $sorted
      # Setup main category menu
      set catlist ""
      foreach menu $sorted {
        set data [lindex $menu end]
        if { [lsearch -exact $catlist [lindex $data end]] == -1 } {
          lappend catlist [lindex $data end]
          lmbox call menulist additem [lindex $data end]
        }
      }
    }
}

# Returns currently selected menu icon
proc menu_selected {} {

    set name [lmbox get mainpage focusedwidget]
    if { [lmbox get $name type] != "picture" } {
      return [lmbox get menulist selectedtext]
    }
    set data [lmbox get $name data]
    return [lindex $data 0]
}

proc menu_mosaic { type { arg "" } } {

    set event [lmbox getevent]
    set object [lmbox get this name]
    set num1 [string index $object end-1]
    set num2 [string index $object end]
    set offset [lmbox var menu_offset 0]
    set ncols [lmbox config menu_ncols 4]
    set nrows [lmbox config menu_nrows 3]
    set count [llength [lmbox var menu_items]]

    switch -- $type {
     action {
        switch -glob -- $event {
         up -
         down -
         next -
         prev -
         prevpage -
         nextpage -
         accept {
           menu_mosaic $event
         }
         default {
           return lmbox_ignore
         }
        }
     }

     prevpage {
        if { $offset-$nrows*$ncols >= 0 } {
          incr offset -[expr $nrows*$ncols]
          lmbox varset menu_offset $offset
          menu_mosaic update
        }
     }

     nextpage {
        if { $offset+$nrows*$ncols < $count } {
          incr offset [expr $nrows*$ncols]
          lmbox varset menu_offset $offset
          menu_mosaic update
        }
     }

     up {
        if { $num1 > 0 } {
          lmbox call menu[incr num1 -1]$num2 setfocus
        } else {
          if { $offset > 1 } {
            incr offset -1
            lmbox varset menu_offset $offset
            menu_mosaic update
          }
        }
     }

     down {
        if { $num1 < $nrows-1 } {
          lmbox call menu[incr num1]$num2 setfocus
        } else {
          if { $offset < $count } {
            incr offset
            lmbox varset menu_offset $offset
            menu_mosaic update
          }
        }
     }

     next {
        if { $num2 < $ncols-1 } {
          lmbox call menu$num1[incr num2] setfocus
        } else {
          if { $num1 < $nrows-1 } {
            lmbox call menu[incr num1]0 setfocus
          } elseif { $offset < $count } {
            incr offset
            lmbox varset menu_offset $offset
            menu_mosaic update
          }
        }
     }

     prev {
        if { $num2 > 0 } {
          lmbox call menu$num1[incr num2 -1] setfocus
        } else {
          lmbox call menulist setfocus
        }
     }

     accept {
        sound_click
        set data [lmbox get this data]
        if { [set script [lindex $data 3]] != "" } {
          eval $script
        }
     }

     refresh {
        # Get menu icons for the current category
        set items ""
        foreach menu [lmbox var lmbox:menu:sorted] {
          set title [lindex $menu 1]
          set data [lindex $menu end]
          if { $arg == [lindex $data end] } {
            lappend items [list $title [lindex $data 1] [lindex $data 2] [lindex $data 3]]
          }
        }
        lmbox varset menu_items $items
        menu_mosaic update 0
     }

     update {
        # Draw the whole page of icons
        set menu_items [lmbox var menu_items]
        set offset [lmbox_nvl $arg $offset]
        if { $offset >= $count } { set offset [expr $count - $nrows*$ncols] }
        if { $offset < 0 } { set offset 0 }
        set focusedwidget [lmbox get focusedwidget name]
        lmbox varset menu_offset $offset
        lmbox set menuscroll value $offset
        for { set i 0 } { $i < $nrows } { incr i } {
          for { set j 0 } { $j < $ncols } { incr j } {
            set name menu$i$j
            if { $offset < $count } {
              set data [lindex $menu_items $offset]
              set bg menu/[lindex $data 2]
              set selectable 1
              set borderstyle normal
            } else {
              set data ""
              set bg none.gif
              set selectable 0
              set borderstyle none
            }
            set area [lmbox get $name origarea]
            if { [set x [lmbox get $name x]] % 10 != 0 } { incr x 4 }
            if { [set y [lmbox get $name y]] % 10 != 0 } { incr y 4 }
            if { $name == $focusedwidget } {
              if { $data == "" } {
                lmbox call [lmbox getpage] updatefocus -1
              }
            }
            lmbox set label$i$j caption [lindex $data 0]
            lmbox set $name x $x y $y selectable $selectable data $data borderstyle $borderstyle area $area
            lmbox call $name zoomimage $bg 0 0
            incr offset
          }
        }
        # Show scrollbar
        if { $count > $ncols*$nrows } {
          lmbox set menuscroll visible true maximum $count
        } else {
          lmbox set menuscroll visible false
        }
        # Update focused widget
        menu_mosaic info [lmbox get focusedwidget data]
     }

     info {
        # When selected, update status
        lmbox set status caption [lindex $arg 1]
        menu_mosaic focus
     }

     mouseover {
        set name [lmbox get this name]
        if { $name != [lmbox get focusedwidget name] } {
          lmbox set status caption [lindex [lmbox get this data] 1]
          lmbox set label[string range $name 4 end] fontcolor #FFFFFF
        }
     }

     mouseout {
        set name [lmbox get this name]
        if { $name != [lmbox get focusedwidget name] } {
          lmbox set label[string range $name 4 end] fontcolor #a6d1d9
        }
     }

     exit {
        # Called when focus lost on the icon
        if { [lmbox get focusedwidget type] == "picture" } {
          lmbox set focusedwidget area [lmbox get focusedwidget origarea]
          lmbox call focusedwidget reloadimage
          lmbox set label[string range [lmbox get focusedwidget name] 4 end] fontcolor #a6d1d9
        }
     }

     focus {
        # Called when focus received by the icon
        if { [lmbox get focusedwidget type] == "picture" } {
          set origarea [lmbox get focusedwidget origarea]
          if { $origarea == [lmbox get focusedwidget areaorig] } {
            sound_click
            lmbox set focusedwidget area $origarea
            lmbox set focusedwidget borderstyle none
            lmbox call focusedwidget transformscale 4 2 15
            lmbox set focusedwidget borderstyle normal
            lmbox set label[string range [lmbox get focusedwidget name] 4 end] fontcolor #c5cd4a
          }
        }
     }
    }
}
