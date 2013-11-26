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


lmbox_register_init im_init

proc im_init {} {

    lmbox dbopen im
    lmbox_register_menu Personal "ICQ Status" "Instant messaging status" news.jpg { lmbox setpage impage } 1400
    # Config page
    lmbox_register_config "ICQ Settings" "lmbox setpage icqconfigpage"
    # Register caller id notifier
    lmbox_register_callerid im_callerid
    # Register backup saver
    lmbox_register_dbsave im
    # Main ICQ thread
    lmbox schedule im_thread -once 1 -thread 1 -interval 1
    lmbox objectcreate icq
}

# Send messages about incoming call
proc im_callerid { title text args } {

    foreach uin [lmbox config icq_callerid] {
      im_send $uin "$title $text"
    }
}

proc im_setup { { refresh 0 } } {

    if { $refresh } {
      lmbox call imlist clear
    }
    if { [lmbox get imlist count] == 0 } {
      lmbox dbeval im {
        lmbox call imlist additemdata "$lmbox_dbkey: [lmbox_var rec(alias)] $rec(status)" $lmbox_dbkey
      } -array rec
    }
    # Show details for existing items only
    if { [lmbox get imlist count] == 0 } {
      lmbox set imdetails enabled 0 visible 0
      lmbox set imchat enabled 0 visible 0
      set x 200
      set on 0
    } else {
      lmbox set imdetails enabled 1 visible 1
      lmbox set imchat enabled 1 visible 1
      set x 320
      set on 1
    }
}

proc im_info { uin { icon imstatus } } {

    array set rec [lmbox dbget im $uin]
    if { [set status [lmbox_var rec(status)]] == "" } { return }
    switch -- $status {
     online - away - na {
       set image icq/$status.gif
     }
     default  {
       set image icq/offline.gif
     }
    }
    lmbox call $icon zoomimage $image 0 0
}

proc im_save {} {

    set uin [lmbox get im_edituin text]
    set alias [lmbox get im_editalias text]
    if { $uin == "" } {
      lmbox set status caption "ICQ UIN should be specified"
      return
    }
    lmbox dbput im $uin [list status offline alias $alias]
    # Request alias from the server
    if { $alias == "" } { im_getinfo }
    lmbox setpage impage
}

proc im_getinfo {} {

    set uin [lmbox get im_edituin text]
    if { $uin != "" } {
      lmbox call icq message send "GetInfo $uin"
      lmbox set status caption "Info request has been sent to the server"
    }
}

proc im_editpage { { new 0 } { uin "" } } {

    array set rec { status offline alias "" }

    if { $new == 0 } {
      set uin [lmbox get imlist selecteddata]
      array set rec [lmbox dbget im $uin]
      if { ![info exists rec(status)] } { return }
    }
    lmbox set im_edituin text $uin
    lmbox set im_editalias text $rec(alias)
    set im_editinfo ""
    foreach { key val } [array get rec] { append im_editinfo "$key: $val " }
    lmbox set im_editinfo caption $im_editinfo
    # Get status image from the main page
    im_info $uin im_editstatus
    lmbox setpage imeditpage
}

proc im_delete {} {

    set uin [lmbox get im_edituin text]
    if { $uin != "" } {
      lmbox dbdel im $uin
    }
    lmbox setpage impage
}

proc im_chatpage {} {

    set uin [lmbox get imlist selecteddata]
    if { $uin == "" } {
      lmbox setpage impage
      return
    }
    array set rec [lmbox dbget im $uin]
    lmbox set im_chatuin caption "$uin / [lmbox_var rec(alias)]"
    # Get status image from the main page
    im_info $uin im_chatstatus
    lmbox set im_chattext focus 1
    lmbox setpage imchatpage
}

proc im_sendmsg {} {

    set uin [lindex [lmbox get im_chatuin caption] 0]
    set text [lmbox get im_chattext text]
    if { $uin != "" && $text != "" } {
      im_send $uin $text
      lmbox set im_chattext text ""
      lmbox set status caption "Message has been sent"
    }
    return lmbox_ignore
}

# Send ICQ message to specified  UIN
proc im_send { uin msg { type text } } {

    lmbox call icq message send [list SendMsg $type $uin $msg]
}

# ICQ message handler
proc im_event { event args } {

    if { $event != "Log" && [lmbox config icq_logevents 0] > 0 } {
      lmbox puts "im: <$event> <$args>"
    }
    switch -- $event {
     MyStatus {
       set status [lindex $args 0]
     }
 
     Incoming {
       lmbox puts "im: $event $args"
       foreach { type uin time text } $args {}
       switch -- $type {
        text {
           array set rec [lmbox dbget im $uin]
           # Do no accept msgs outside of contact list
           if { ![info exists rec(alias)] && [lmbox config icq_unknown] == "" } {
             return
           }
           set title "[clock format $time -format "%m/%d/%y %H:%M"]: ICQ Message From [lmbox_var rec(alias) $uin]"
           # Add message to the chat text
           if { [lmbox getpage] == "imchatpage" } {
             set text "$title\n$text\n\n[lmbox get im_chatlog caption]"
             lmbox set im_chatlog caption $text lineoffset 1
           } else {
             lmbox_msg $title $text
           }
        }
       }
     }

     Status {
       set uin [lindex $args 0]
       array set rec [lmbox dbget im $uin]
       set rec(status) [lindex $args 1]
       lmbox dbput im $uin [array get rec]
       lmbox puts "im: $event $uin [lmbox_var rec(alias)] $rec(status)"
     }
     
     Info {
       set ref [lindex $args 0]
       set uin [lmbox var im:mapping:$ref]
       if { $uin != "" } {
         array set rec [lmbox dbget im $uin]
         foreach { key val } [lindex $args 1] {
           if { $val == "" } { continue }
           switch -- $key {
            Nick { set key alias }
           }
           set rec($key) $val
         }
         lmbox dbput im $uin [array get rec]
         lmbox varunset im:mapping:$ref
         lmbox puts "im: $event: [array get rec]"
       }
     }
     
     SendMsg {
       foreach { type uin msg } $args {}
       [lmbox get icq data] send $type $uin $msg
     }
     
     GetInfo {
       set uin [lindex $args 0]
       set ref [lmbox varincr im:refcount]
       lmbox varset im:mapping:$ref $uin
       [lmbox get icq data] info $uin $ref
       if { $ref >= 250 } {
         lmbox varset im:refcount 0
       }
     }
    }
}

# Main ICQ thread
proc im_thread {} {

    set uin [lmbox config icq_user]
    set passwd [lmbox config icq_passwd]
    if { $uin == "" || $passwd == "" } { return }
    # Login to ICQ and populate internal contacts list
    set icq [icq::icq $uin $passwd -event im_event -reconnect 1]
    lmbox dbeval im { $icq contacts all $lmbox_dbkey }
    $icq status online
    lmbox set icq data $icq
    lmbox puts "ICQ thread started: $uin"
    if { [catch {
      while { ![lmbox isquit] } {
        set msg [lmbox call icq message read 2]
        if { $msg != "" && [catch { eval im_event $msg } errmsg] } {
          lmbox puts im_thread: $errmsg
        }
        if { [lmbox get system idletime] > [lmbox config im_away 3600] } {
          $icq status away
        } else {
          if { [$icq status] != "online" } {
            $icq status online
          }
        }
        update
      }
      $icq delete
    } errmsg] } {
      lmbox puts "ICQ error: $errmsg"
    }
    lmbox puts "ICQ thread stopped"
}
