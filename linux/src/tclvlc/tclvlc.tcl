#!/usr/bin/tclsh

load tclvlc.so

if { [set vlc [vlc create]] != "" } {
  vlc add $vlc /disk6/Movies/VKrugePervom.1.avi
  vlc play $vlc
  after 3000
  vlc stop $vlc
  vlc destroy $vlc
}

