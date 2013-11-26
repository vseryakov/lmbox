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

lmbox_register_init dvdrip_init

proc dvdrip_init {} {
   
    lmbox dbopen dvdrip
    lmbox_register_menu Personal "DVD Rip" "Ripping DVD for personal collection" dvdrip.jpg { lmbox setpage dvdrippage } 1700
    # Cache directory should exist
    set dvdrip_path [lmbox config dvdrip_path [lmbox gethome]/dvdrip]
    if { [catch { file mkdir $dvdrip_path } errmsg] } { lmbox puts $errmsg }
}

proc dvdrip_setup {} {

    lmbox call dvdtitle clear
    lmbox dbeval dvdrip {
      lmbox call dvdtitle additemdata $lmbox_dbdata [string trimleft $lmbox_dbkey 0]
    }
    if { [lmbox get dvdaudio count] == 0 } {
      lmbox call dvdaudio additemdata "Track 1" 128
      lmbox call dvdaudio additemdata "Track 2" 129
      lmbox call dvdaudio additemdata "Track 3" 130
      lmbox call dvdaudio additemdata "Track 4" 131
      lmbox call dvdaudio additemdata "Track 5" 132

      lmbox call dvdsubtitle additem None
      lmbox call dvdsubtitle additemdata 0 0
      lmbox call dvdsubtitle additemdata 1 1
      lmbox call dvdsubtitle additemdata 2 2
      lmbox call dvdsubtitle additemdata 3 2
    }
    # Run scan automatically for the first time
    if { [lmbox get dvdtitle count] == 0 } {
      lmbox run dvdrip_scan
    }
    dvdrip_info progress
}

# Perform DVD disk scanning
#
# (dvd_reader.c) mpeg2 ntsc 16:9 only letterboxed U0 NTSC CC 1 720x480 video
# (dvd_reader.c) ac3 en drc 48kHz 2Ch 
# (dvd_reader.c) DVD title 3/14: 2 chapter(s), 1 angle(s), title set 2
# (dvd_reader.c) title playback time: 00:00:06.18  7 sec
# (dvd_reader.c) [Chapter 01] 00:00:00.000 , block from 3291 to 3295
# (dvd_reader.c) [Chapter 02] 00:00:00.000 , block from 16777218 to 17
# [tcprobe] summary for /dev/dvd, (*) = not default, 0 = not detected
# import frame size: -g 720x480 [720x576] (*)
#     aspect ratio: 4:3 (*)
#       frame rate: -f 23.976 [25.000] frc=1 (*)
#      audio track: -a 0 [0] -e 48000,16,2 [48000,16,2] -n 0x2000 [0x2000] 
#
proc dvdrip_scan {} {

    # Check DVD disk status
    switch -- [lmbox cdromstatus] {
     empty {
       lmbox set status caption "DVD disk is not present in the DVD tray"
       return
     }
     error -
     unknown {
       lmbox set status caption "DVD drive reports error, open/close DVD tray, check disk and try again"
       return
     }
    }
    dvdrip_puts "Scanning DVD ..."
    set dvd_dev [lmbox config dvd_device /dev/dvd]
    set dvdrip_path [lmbox config dvdrip_path [lmbox gethome]/dvdrip]
    # Clean up all leftover files
    foreach file [glob -nocomplain $dvdrip_path/*] {
      if { [catch { file delete -force -- $file } errmsg] } { lmbox puts $errmsg }
    }
    # Probe first title
    if { [catch { exec tcprobe -T 1 -d 0 -i $dvd_dev } data] } { lmbox puts $data }
    lmbox dbdel dvdrip
    lmbox call dvdtitle clear
    if { ![regexp {DVD title ([0-9]+)/([0-9]+)} $data d id count] } {
      lmbox set status caption "Unable to scan DVD titles, is DVD disk present"
      lmbox puts $data
      return
    }
    while {1} {
      set line "Title $id:"
      if { [regexp {title playback time: ([0-9\:]+)} $data d time] } { append line " $time" }
      if { [regexp {aspect ratio: ([^ ]+)} $data d aspect] } { append line " $aspect" }
      if { [regexp {frame rate: -f ([^ ]+)} $data d fps] } { append line " $fps fps" }
      lmbox call dvdtitle additemdata $line $id
      lmbox dbput dvdrip [format "%03d" $id] $line
      lmbox_puts "Found $line"
      if { [incr id] > $count } { break }
      if { [catch { exec tcprobe -T $id -d 0 -i $dvd_dev } data] } { lmbox puts $data }
      if { ![regexp "summary for $dvd_dev" $data] } { break }
    }
    # Make screenshot of the 1st title
    dvdrip_screenshot
    dvdrip_puts "DVD scan completed"
}

# Output message and save it in dvdrip progess status
proc dvdrip_puts { msg } {

    lmbox_puts $msg
    lmbox varset dvdrip_status $msg
}

# Show info about DVD title
proc dvdrip_info { type { id "" } } {

    if { $id == "" } {
      if { [set id [lmbox get dvdtitle selecteddata]] == "" } { return }
    }
    set dvd_dev [lmbox config dvd_device /dev/dvd]
    set dvdrip_path [lmbox config dvdrip_path [lmbox gethome]/dvdrip]
    switch -- $type {
     status {
       if { [file exists $dvdrip_path/$id.png] } {
         lmbox call dvdicon zoomimage $dvdrip_path/$id.png 0 0
       } else {
         lmbox set dvdicon image none.gif
       }
     }
     
     title {
       catch { exec tcprobe -T $id -d 0 -i $dvd_dev } data
       set data [string map { {(dvd_reader.c) } {} {[tcprobe] } {} } $data]
       lmbox_alarm "DVD Title $id Details" $data $dvdrip_path/$id.png
     }
     
     progress {
       if { [set status [lmbox var dvdrip_status]] != "" } {
         lmbox set status caption $status
       }
     }
    }
}

# Play selected DVD title
proc dvdrip_play { { id "" } } {

    if { $id == "" } {
      if { [set id [lmbox get dvdtitle selecteddata]] == "" } { return }
    }
    lmbox call player clear
    lmbox call player add dvd://$id
    lmbox setpage videopage
}

# Make a screenshot, perform random seeking
proc dvdrip_screenshot { { id "" } { file "" } } {

    if { $id == "" } {
      if { [set id [lmbox get dvdtitle selecteddata]] == "" } { return }
    }
    dvdrip_puts "DVD title $id: taking screenshot ..."
    if { $file == "" } {
      set dvdrip_path [lmbox config dvdrip_path [lmbox gethome]/dvdrip]
      set file $dvdrip_path/$id.png
    }
    lmbox call player clear
    lmbox call player add dvd://$id
    lmbox set player videodriver none
    lmbox set player audiodriver none
    if { [lmbox call player play] != 0 } {
      dvdrip_puts "Unable to play DVD title $id"
      return
    }
    # Seek to random point within the title, do 3 tries becaus eit may take 
    # some time to start DVD
    for { set i 0 } { $i < 3 } { incr i } {
      # Wait for a while for player to start
      after [expr [lmbox config dvdrip_delay 5]*1000]
      # Length of the DVD title
      set length [expr [lmbox get player length]/1000]
      set pos [lmbox random $length]
      lmbox call player seek $pos
      dvdrip_puts "DVD title $id: taking screenshot at position $pos out of $length"
      lmbox call player snapshot $file
      if { [file exists $file] } { break }
    }
    lmbox call player stop
    dvdrip_info status
}

# Perform DVD ripping into movie file
proc dvdrip_run { { id "" } { file "" } { mp3 0 } } {

    if { $id == "" } {
      if { [set id [lmbox get dvdtitle selecteddata]] == "" } {
        dvdrip_puts "No DVD title has been selected, please run Scan first"
        return
      }
    }
    set name [lmbox get dvdname text]
    if { $name == "" } {
      lmbox set status caption "Save Name should be specified"
      return
    }
    set now [clock seconds]
    set title [lmbox get dvdtitle selectedtext]
    set dvd_dev [lmbox config dvd_device /dev/dvd]
    set video_path [lmbox config camera_path [lmbox gethome]/videos]
    set dvdrip_path [lmbox config dvdrip_path [lmbox gethome]/dvdrip]
    # See if we have enough disk space
    array set fs [lmbox statfs $dvdrip_path]
    set vob_avail [expr ($fs(frsize)/1024*$fs(bavail))/1024]
    array set fs [lmbox statfs $video_path]
    set avi_avail [expr ($fs(frsize)/1024*$fs(bavail))/1024]
    if { $vob_avail < 8000 } {
      lmbox set status caption "Not enough disk space in $dvdrip_path, available [lmbox_size $vob_avail]"
      return
    }
    if { $avi_avail < 3000 } {
      lmbox set status caption "Not enough disk space in $video_path, available [lmbox_size $avi_avail]"
      return
    }
    # Build output file name if not specified
    if { $file == "" } {
      foreach word $name { lappend file [string totitle $word] }
      set file [join $file _].avi
    }
    # Make sure there is no extension
    set movie $video_path/$file
    # It is possible to run encoder directly from dvd drive instead of
    # from extracted VOB, in some cases it is the only way to properly rip DVD
    if { [lmbox config dvdrip_save 1] } {
      set vob $dvdrip_path/$id.vob
    } else {
      set vob dvd://$id
    }
    set icon $dvdrip_path/$id.png
    # Make screenshot of the title
    if { ![file exists $icon] } {
      dvdrip_screenshot $id $icon
    }
    # Copy screenshot to the video directory
    if { [catch { file copy -force -- $icon [file rootname $movie].png } errmsg] } {
      lmbox puts $errmsg
    }
    # Remove unnecessary files
    if { [catch { file delete -force -- divx2pass.log frameno.avi } errmsg] } {
      lmbox puts $errmsg
    }
    # Extract DVD title from DVD to the hard drive for faster processing
    if { ![string match dvd://* $vob] && ![file exists $vob] } {
      dvdrip_puts "Extracting DVD title $id ..."
      if { [catch { exec /bin/sh -c "tccat -i $dvd_dev -T $id,-1 > $vob" } errmsg] } {
        lmbox puts $errmsg
      }
      if { ![file exists $vob] } {
        dvdrip_puts "Unable to extract DVD title $id, DVD rip process cancelled"
        return
      }
    }
    dvdrip_puts "DVD rip process started ..."
    # Audio track, default is en, 1st audio track is 128, second 129,...
    set aid "-aid [lmbox get dvdaudio selecteddata]"
    # Optional arguments to mencoder
    if { [set params [lmbox config dvdrip_params]] != "" } {
      lmbox puts "DVDRip: adding custom parameters: $params"
    }
    append params " -vop pp=lb "
    # Video bit rate
    set vbr [lmbox config dvdrip_vbr 1450]
    # Autodetecting cropping
    dvdrip_puts "Autodetecting cropping..."
    catch { exec /bin/sh -c "mplayer $vob -quiet -ss 300 -frames 100 -nosound -vf cropdetect -geometry 1x1x0x0 2>/dev/null|grep crop|awk '{print \$8}'" } crop
    # Video crop and scale, example: -vf crop=720:352:0:62,scale=640:480
    set crop [lindex [split $crop "\n"] end]
    if { [regexp {crop=([0-9]+\:[0-9]+\:[0-9]+\:[0-9]+)\)} $crop d crop] } {
      append params " -vf crop=$crop "
      dvdrip_puts "Detected: $crop"
    }
    
    # Extract subtitles
    set subtitle [lmbox get dvdsubtitle selecteddata]
    if { $subtitle != "" } {
      #dvdrip_puts "Extracting subtitles..."
      #set subtitle [format 0x%x [expr $subtitle+0x20]]
      #if { [catch { eval exec bin/sh -c "tccat -i $vob -T $title -L | tcextract -x ps1 -t vob -a $subtitle > [file rootname $movie].sub" } errmsg] } { puts $errmsg }
    }

    # First pass, extract sound
    if { $mp3 } {
      dvdrip_puts "Encoding MP3 audio ..."
      if { [catch { eval exec mencoder $vob -quiet $aid $params -oac mp3lame -lameopts mode=2:cbr:br=192:vol=2 -ovc frameno -o frameno.avi } errmsg] } {
       lmbox puts $errmsg
      }
    } else {
      dvdrip_puts "Encoding AC3 audio ..."
      if { [catch { eval exec mencoder $vob -quiet $aid $params -oac copy -ovc frameno -o frameno.avi } errmsg] } {
        lmbox puts $errmsg
      }
    }
    dvdrip_puts "Encoding pass 1 ..."
    # Video encoding
    switch -- [lmbox config dvdrip_pass 2] {
     2 {
       # Two pass encoding, better quality
       if { [catch { eval exec mencoder $vob -quiet $aid $params -sws 2 -oac copy -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=$vbr:vhq:vqmin=2:vpass=1 -ffourcc XVID -o /dev/null } errmsg] } {
         lmbox puts $errmsg
       }
       dvdrip_puts "Encoding pass 2 ..."
       if { [catch { eval exec mencoder $vob -quiet $aid $params -sws 2 -oac copy -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=$vbr:vhq:vqmin=2:vpass=2 -ffourcc XVID -o $movie } errmsg] } {
         lmbox puts $errmsg
       }
     }
     
     1 {
       # One pass encoding
       if { [catch { eval exec mencoder $vob -quiet $aid $params -sws 2 -oac copy -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=$vbr:vhq:vqmin=2 -ffourcc XVID -o $movie } errmsg] } {
         lmbox puts $errmsg
       }
     }
    }
    # Remove unnecessary files
    if { [catch { file delete -force -- divx2pass.log frameno.avi } errmsg] } {
      lmbox puts $errmsg
    }
    dvdrip_puts "DVD rip process completed, time: [lmbox_date uptime [expr [clock seconds]-$now]]"
}

