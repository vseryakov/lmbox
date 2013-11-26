#!/usr/bin/tclsh

set hour [string trimleft [clock format [clock seconds] -format "%H"] 0]
if { $hour >= 8 } { exit 0 }
set count 0
foreach host { 192.168.0.9 192.168.0.5 } {
  if { [ catch { set ping [exec /bin/ping -q -c 2 -w 2 $host] }] } { continue }
  if { [regexp {2 received} $ping] } { incr count }
}
set ps [exec ps agx]
if { ![regexp {wget|sshd -i} $ps] && $count == 0 } {
  exec logger "no activity: $hour: shutting down the system"
  exec /sbin/halt
}

