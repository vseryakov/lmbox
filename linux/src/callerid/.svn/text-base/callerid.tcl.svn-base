#!/usr/bin/tclsh                                                                                          

load /usr/local/lib/tclsys.so

set name ""
set phone ""
set address { 192.168.0.255:80 }
set modeminit "AT+VCID=1"

set fd [open /dev/modem {RDWR NOCTTY}]                                                          
fconfigure $fd -mode 9600,n,8,1 -buffering none -encoding binary -translation binary

puts $fd "ATZ\r\n$modeminit\r"
flush $fd

while {1} {
   if { [set line [string trim [gets $fd]]] == "" } { continue }
   ns_syslog -ident callerid: info $line
   switch -glob -- $line {
    NMBR* {
      switch -- [set phone [lindex $line 2]] {
       P { set phone Private }
       O { set phone Unknown }
      }
    }
    NAME* {
      switch -- [set name [join [lrange $line 2 end]]] {
       P { set name Private }
       O { set name Unknown }
      }
      ns_syslog -ident callerid: info "Caller ID: $phone $name"
      # Send HTTP broadcast
      set url "callerid:$phone:[string map {" " +} $name]"
      foreach ipaddr $address {
        foreach { ipaddr port } [split $ipaddr :] {}
        ns_sysudp $ipaddr $port "GET $url HTTP/1.0\r\n\r\n" -noreply 1
      }
      set name ""
      set phone ""
    }
   }
}
