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

# Initialize modem to produce caller id info
proc modem_init {} {

    global lmbox_chan

    set modem_init [lmbox config modem_init "AT+VCID=1"]
    # Modem port params: ex. 9600,8,n,1
    if { [set modem_mode [lmbox config modem_mode]] != "" } {
      fconfigure $lmbox_chan -mode $mode -buffering none -encoding binary -translation binary
    }
    puts $lmbox_chan "ATZ\r\n$modem_init\r"
    flush $lmbox_chan
}

# Check output from the modem and detect caller id
proc modem_read {} {

    global lmbox_data

    set line [string trim $lmbox_data]
    lmbox puts "modem_read: $line"

    switch -glob -- $line {
     NMBR* {
      switch -- [set phone [lindex $line 2]] {
       P { set phone Private }
       O { set phone Unknown }
      }
      # Save number and wait for the name
      lmbox varset modem_phone $phone
     }

     NAME* {
      switch -- [set name [join [lrange $line 2 end]]] {
       P { set name Private }
       O { set name Unknown }
      }
      # Show caller id info directly and send UDP packet
      if { [set phone [lmbox var modem_phone]] != "" } {
        http_exec "callerid:$phone:$name"
        # HTTP request for notification
        set url "GET callerid:$phone:[string map {" " +} $name] HTTP/1.0\r\n\r\n"
        # Send UDP packet if configured
        foreach ipaddr [lmbox config modem_ipaddr] {
          set ipaddr [split $ipaddr :]
          set type [lmbox_nvl [lindex $ipaddr 2] udp]
          set port [lmbox_nvl [lindex $ipaddr 1] 80]
          set ipaddr [lindex $ipaddr 0]
          if { [catch {
            switch -- $type {
             udp {
               if { [catch { lmbox udpsend $ipaddr $port $url -noreply 1 } errmsg] } {
                 lmbox puts "modem_read: $ipaddr:$port: $errmsg"
               }
             }
             tcp {
               if { [catch {
                 set fd [socket $ipaddr $port]
                 puts $fd $url
                 close $fd
               } errmsg] } {
                 lmbox puts "modem_read: $ipaddr:$port: $errmsg"
               }
             }
            }
          } errmsg] } {
            lmbox puts "modem_read: $ipaddr:$port: $errmsg"
          }
        }
      }
      lmbox varset modem_phone ""
     }
    }
}
