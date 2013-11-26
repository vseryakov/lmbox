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

lmbox_register_init contact_init

proc contact_init {} {

    lmbox dbopen contacts
    lmbox_register_menu Personal Contacts "Personal contact database" contact.jpg { lmbox setpage contactpage } 1200
    lmbox_register_check Contacts contact_load
}

# Load all contacts on first call into the database
proc contact_load { args } {

    set contact_file [lmbox config contact_file contacts.txt]
    set contact_path [lmbox config contact_path data/Contacts]
    if { [file exists $contact_path/$contact_file] } {
      # Check last time we loaded contact file
      set contact_mtime [file mtime $contact_path/$contact_file]
      if { [lmbox dbstat contacts count] == 0 ||
           $contact_mtime > [lmbox config contact_mtime 0] } {
        set count 0
        lmbox dbdel contacts
        lmbox_puts "Loading contacts..."
        set fd [open $contact_path/$contact_file]
        while { ![eof $fd] } {
          set line [string trim [gets $fd]]
          if { [string index $line 0] == "#" } { continue }
          set line [split $line |]
          if { [set id [lindex $line 0]] == "" } { continue }
          if { [set rec(timestamp) [lindex $line 1]] == "" } { continue }
          if { [set rec(name) [lindex $line 2]] == "" } { continue }
          set rec(address) [lindex $line 3]
          set rec(home_phone) [lindex $line 4]
          set rec(work_phone) [lindex $line 5]
          set rec(cell_phone) [lindex $line 6]
          set rec(pager) [lindex $line 7]
          set rec(fax) [lindex $line 8]
          set rec(email) [lindex $line 9]
          set rec(email2) [lindex $line 10]
          set rec(icq) [lindex $line 11]
          set rec(company) [lindex $line 12]
          set rec(birthday) [lindex $line 13]
          set rec(notes) [lindex $line 14]
          lmbox dbput contacts $id [array get rec]
          incr count
        }
        close $fd
        lmbox call contactlist clear
        lmbox_puts "$count contacts loaded"
        # Mark the last time we loaded full contacts file
        lmbox_setconfig contact_mtime $contact_mtime
        lmbox dbsync
      }
    }
    contact_alarms
}

proc contact_alarms {} {

    # Check who has birthday today and show that on the main page
    set birthday ""
    set now [clock seconds]
    set today [clock format $now -format "%b %d"]
    lmbox dbeval contacts {
      if { [string match "$today *" $rec(birthday)] } {
        lappend birthday $lmbox_dbkey $rec(name) [lmbox_var rec(home_phone) [lmbox_var rec(cell_phone) [lmbox_var rec(work_phone)]]]
      }
    } -array rec
    if { $birthday != "" } {
      set msg ""
      set icon ""
      set today [clock format $now -format "%Y-%m-%d"]
      set contact_path [lmbox config contact_path data/Contacts]
      foreach { id name phone } $birthday {
        # See if we have already reported about that person
        array set rec [lmbox dbget system birthday:$id]
        if { [lmbox_var rec(today)] == $today } { continue }
        if { $icon == "" } {
          set icon [lmbox_icon $id $contact_path]
        }
        append msg "NAME: $name\n"
        if { $phone != "" } {
          append msg "Phone: $phone\n\n"
        }
        array set rec [list name $name today $today]
        lmbox dbput system birthday:$id [array get rec]
      }
      if { $msg != "" } {
        lmbox_log BIRTHDAY $birthday
        lmbox_msg "TODAY'S BIRTHDAY" $msg $icon
      }
    }
}

# Search contact records
proc contact_search { type value { count 1 } } {

    switch -- $type {
     birthday {
       if { $value == "" } {
         set value [clock format [clock seconds] -format "%b %d"]
       }
       set filter "\[string match {$value *} \[lmbox_var rec(birthday)\]\]"
     }

     email {
       set filter "\[string match {*$value*} \[lmbox_var rec(email)\]\] || \[string match {*$value*} \[lmbox_var rec(email2)\]\]"
     }

     name {
       set filter "\[string match {*$value*} \[lmbox_var rec(name)\]\]"
     }

     phone {
       set val [string map { - {} . {} { } {} } $value]
       set filter "\[lmbox_var rec(home_phone)\] == {$val} || \[lmbox_var rec(work_phone)\] == {$val} || \[lmbox_var rec(cell_phone)\] == {$val} || \[lmbox_var rec(pager)\] == {$val} || \[lmbox_var rec(fax)\] == {$val}"
     }

     default {
       return
     }
    }
    set result ""
    set contact_path [lmbox config contact_path data/Contacts]
    lmbox dbeval contacts {
      if { $filter != "" && [expr $filter] } {
        set rec(icon) [lmbox_icon $lmbox_dbkey $contact_path]
        lappend result [array get rec]
        incr count -1
        if { $count <= 0 } { break }
      }
    } -array rec
    return $result
}

proc contact_values {} {

    set values ""
    upvar rec rec
    foreach { key val } [array get rec] { lappend values $val }
    return $values
}

# Load contact list according to current filter
proc contact_setup { { filter "" } { clear 0 } } {

    if { $clear } {
      lmbox call contactlist clear
    }
    if { [lmbox get contactlist count] == 0 } {
      lmbox dbeval contacts {
        if { $filter == "" || [expr $filter] } {
          lmbox call contactlist additemdata $rec(name) $lmbox_dbkey
        }
      } -array rec
      lmbox call contactlist sort
    }
}

proc contact_info { type id } {

    if { $id == "" } { return }
    set data ""
    set contact_path [lmbox config contact_path data/Contacts]
    switch -- $type {
     status {
       set data ""
       array set rec [lmbox dbget contacts $id]
       foreach { key val } [array get rec] {
         if { $val == "" } { continue }
         switch -- $key {
          home_phone { set data "Home: $val " }
          cell_phone { append data "Cell: $val " }
          work_phone { append data "Work: $val " }
          birthday { append data "Birthday: $val" }
         }
       }
       lmbox set status caption [lmbox_nvl $data "No info"]
       set icon [lmbox_nvl [lmbox_icon $id $contact_path] none.gif]
       lmbox call contacticon zoomimage $icon 0 0
       set count [lmbox_nvl [lmbox get contactlist count] 0]
       set index [lmbox_nvl [lmbox get contactlist selectedindex] 0]
       if { $count > 0 } { incr index }
       lmbox set contactcount caption "$index of $count"
     }
     
     info {
       array set rec [lmbox dbget contacts $id]
       foreach { key val } [array get rec] {
         if { $key != "*" && $val != "" } {
           append data "[string totitle $key]: $val\n"
         }
       }
       set icon [lmbox_icon $id $contact_path]
       lmbox_alarm "Contact Details" $data $icon
     }
    }
}
