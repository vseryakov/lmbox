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

# Example channels:
#   http://wwitv.com/television/174.htm
#   http://donmc.libsyn.com/rss
#

lmbox_register_init itv_init

proc itv_init {} {

    lmbox dbopen itv
    lmbox dbopen itv_guid
    lmbox dbopen itv_channels
    lmbox dbopen itv_queue

    # Initial data load
    lmbox_register_check ITV itv_check
    # Check for queued links
    lmbox schedule "itv_schedule" -interval 300 -thread 1
    # Main menu entry
    lmbox_register_menu TV "Internet TV" "Watch TV from the Internet" itv.jpg { lmbox setpage itvpage } 770
    # Catch directory should exist
    set rss_path [lmbox config rss_path [lmbox gethome]/rss]
    if { [catch { file mkdir $rss_path } errmsg] } { lmbox puts $errmsg }
}

proc itv_setup {} {

    if { [lmbox get itvchannels count] == 0 } {
      lmbox dbeval itv_channels {
        lmbox call itvchannels additemdata $rec(title) $lmbox_dbkey
      } -array rec
      lmbox call itvchannels sort
    }
}

proc itv_info { type id { data "" } } {

    switch -- $type {
     links {
       set index [lmbox get itvlinks selectedindex]
       lmbox call itvlinks clear
       lmbox dbeval itv {
         if { $id == $rec(channel) } {
           lmbox call itvlinks additemdata $rec(title) $lmbox_dbkey
         }
       } -array rec
       array set rec [lmbox dbget itv_channels $id]
       lmbox set itvlinks selectedindex $index
       lmbox set status caption "Channel: [lmbox get itvchannels selectedtext] [lmbox_var rec(genre)]"
     }

     info {
       array set rec [lmbox dbget itv $id]
       lmbox set itvinfo caption "$rec(title): $rec(genre) $rec(description)"
       set icon [itv_info icon $id]
       lmbox call itvicon zoomimage $icon 0 0
     }

     icon {
       array set rec [lmbox dbget itv $id]
       # Try by item id
       set icon [lmbox config rss_path [lmbox gethome]/rss]/$id.jpg
       # Try by item name
       if { ![file exists $icon] } {
         set itv_path [lmbox config itv_path data/iTV]
         set name [string tolower [string map { { } {_} } $rec(title)]]
         set icon $itv_path/$name.jpg
         if { ![file exists $icon] } {
           set icon ""
           foreach file [glob -nocomplain $itv_path/*.jpg] {
             if { [regexp "^[file rootname [file tail $file]]" $name] } { set icon $file }
           }
         }
       }
       # Try by channel id
       if { ![file exists $icon] } {
         set icon [lmbox config rss_path [lmbox gethome]/rss]/$rec(channel).jpg
       }
       if { ![file exists $icon] } {
         set icon none.gif
       }
       return $icon
     }

     status {
       if { [set title [lmbox get player title]] != "" &&
            [lmbox get player status] == "play" } {
         set title "Now Playing: $id: $title"
       }
       lmbox set status caption $title
     }
    }
}

proc itv_play { id } {

    array set rec [lmbox dbget itv $id]
    if { [set name [lmbox var itv_connecting]] != ""  } {
      lmbox_puts "Still connecting to $name"
      return
    }
    set url $rec(url)
    # Check for locally cached file first
    set file [lmbox config rss_path [lmbox gethome]/rss]/[itv_filename $url]
    if { [file exists $file] } {
      set url $file
    }
    lmbox varset itv_connecting $rec(title)
    lmbox call player stop
    lmbox call player clear
    lmbox call player add $url
    # Add the link temporary to favorite links
    set expires [expr [clock seconds] + [lmbox config recent_lifetime 86400]]
    itv_favorite $id expires $expires class History
    lmbox_puts "Connecting to $rec(title): $rec(url)"
    # For flash videos we have to use web browser
    if { [regexp -nocase {shockwave} [lmbox_var rec(mimetype)]] } {
      web_page $rec(url) 0
    } else {
      lmbox run "itv_connect {$rec(title)}"
      lmbox setpage itvpage
    }
}

proc itv_favorite { id args } {

    set class ""
    set expires 0
    foreach { key val } $args { set $key $val }

    array set rec [lmbox dbget itv $id]
    if { ![info exists rec(url)] } { return }
    favorites_add Video "script:itv_play $id" $rec(title) description $rec(description) icon [itv_info icon $id] expires $expires class $class
    lmbox set status caption "$rec(title) added to favorites"
}

proc itv_connect { name } {

    catch {
      itv_size normal
      if { [lmbox call player play] == 0 } {
        lmbox call player settimer 10000 "itv_info status {$name}"
        itv_info status $name
      }
    }
    lmbox varset itv_connecting ""
}

proc itv_pause {} {

    lmbox call player pause
}

proc itv_stop {} {

    lmbox call player stop
    lmbox call itvlinks setfocus
}

# Returns formatted file name from the given url
proc itv_filename { url } {

    set ext [file ext $url]
    set name [file rootname $url]
    return [string map { // _ / _ : {} & - ? - ; - = - + _ % {} . _ } $name]$ext
}

# Event handler for zoom button to process evbents for player during fullscreen
proc itv_event {} {

    if { [lmbox get player status] == "play" && [lmbox get player x] == 0 } {
      switch -- [lmbox getevent] {
       info { player_info }
       up { player_event up player_aspect -0.1 }
       down { player_event down player_aspect 0.1 }
       prev { player_event prev player_seek -900 }
       next { player_event next player_seek 900 }
       nextpage { player_seek 30 }
       prevpage { player_seek -30 }
       default { return lmbox_ignore }
      }
    }
    return lmbox_ignore
}

proc itv_size { { mode "" } } {

    # Size of output window
    foreach { x y w h } [lmbox get itvchannels area] {}

    switch -- $mode {
     toggle {
       array set modes { normal icon icon fs fs normal small normal }
       set mode [lmbox_var modes([lmbox var itv_size normal]) normal]
       itv_size $mode
     }

     fs {
       lmbox call player move 0 0 0 0
       lmbox varset itv_size $mode
       # Switch to zoom button so we can process player events
       lmbox call itvzoom setfocus
     }

     icon {
       set vwidth [lmbox get player videowidth]
       set vheight [lmbox get player videoheight]
       if { $vwidth < $w && $vheight < $h } {
         lmbox call player move [expr $x+($w-$vwidth)/2] [expr $y+($h-$vheight)/2] $vwidth $vheight
         lmbox varset itv_size $mode
         lmbox call itvlinks setfocus
       } else {
         itv_size fs
       }
     }

     small {
       foreach { x y w h } [lmbox get itvicon area] {}
       lmbox call player move $x $y $w $h
       lmbox varset itv_size small
     }

     default {
       lmbox call player move $x $y $w $h
       lmbox varset itv_size normal
       lmbox call itvlinks setfocus
     }
    }
}

# Perform downloading queue links
proc itv_schedule {} {

    # Already or still running
    if { ![lmbox var itv:schedule 0] } {
      lmbox varset itv:schedule 1
      catch {
        # Get the list of queued urls
        set queue ""
        lmbox dbeval itv_queue {
          lappend queue $lmbox_dbkey $lmbox_dbdata
        }
        # Check that link is still in the database and do downloading
        foreach { url id } $queue {
          if { [lmbox dbexists itv $id] } {
            set file [lmbox config rss_path [lmbox gethome]/rss]/[itv_filename $url]
            if { [file exists $file] } {
              lmbox puts "itv_schedule: $id: file already exists for $url"
              lmbox dbdel itv_queue $url
              continue
            }
            switch -glob -- $url {
             http://* - ftp://* {
               lmbox puts "itv_schedule: $id: download started for $url..."
               if { [catch { exec wget -q -c -O $file.tmp $url } errmsg] } { puts "wget: $errmsg" }
               if { [catch { file rename -force $file.tmp $file } errmsg] } { puts $errmsg }
               lmbox puts "itv_schedule: $id: downloaded."
             }

             default {
               lmbox puts "itv_schedule: $id: unsupported protocol: $url"
             }
            }
            # Delete from the queue now
            lmbox dbdel itv_queue $url
          }
        }
      }
      lmbox varset itv:schedule 0
    }
}

proc itv_check { { refresh 0 } } {

    # Wipe out the whole database
    if { $refresh } {
      lmbox dbdel itv
      lmbox dbdel itv_guid
      lmbox dbdel itv_channels
    }
    # Load locally saved channels on the first run
    if { [lmbox dbstat itv_channels count] == 0} {
      foreach file [glob -nocomplain data/iTV/*.htm] {
        itv_load_wwitv $file
      }
      # Local .rss files can contain urls to actual RSS feeds
      foreach file [glob -nocomplain data/iTV/*.rss] {
        foreach url [split [lmbox_getfile $file] "\n"] {
          switch -glob -- [set url [string trim $url]] {
           "" - "#*" {}
           default { itv_load_rss $url }
          }
        }
      }
    }
    set channels ""
    set now [clock seconds]
    # Go through channels and see who needs a refresh
    lmbox dbeval itv_channels {
      if { $now - $rec(timestamp) > $rec(ttl) } {
        lappend channels $rec(url) $rec(type)
      }
    } -array rec
    # Now do the real refresh
    foreach { url type } $channels {
      switch -- $type {
       wwitv {
         itv_load_wwitv $url
       }

       rss {
         itv_load_rss $url
       }
      }
    }
}

# Load url with Internet TV/video links
proc itv_load { url } {

    switch -regexp -- $url {
     {wwitv\.com/television} {
       itv_load_wwitv $url
     }

     default {
       itv_load_rss $url
     }
    }
}

# Parse file from wwitv.com
proc itv_load_wwitv { url } {

    if { [set url [string trim $url]] == "" } {
      return
    }
    if { [string match {http://*} $url] } {
      set data [lmbox_geturl $url]
    } else {
      set data [lmbox_getfile $url]
      set url http://wwitv.com/television/[file tail $url]
    }
    if { ![regexp -nocase {<title>(TV|Watch) .*from ([^<]+)</title>} $data d d country] &&
         ![regexp -nocase {<title>Local ([^ ]+) TV channels</title>} $data d country] } {
      lmbox_puts "Invalid iTV format $url"
      return
    }
    if { [set idx [string first "(" $country]] > 0 } {
      set country [string range $country 0 [incr idx -1]]
    }
    set now [clock seconds]
    set country [lmbox_title [string trim $country " ."]]
    # Creating new channel
    if { [set guid [lmbox dbget itv_guid $country]] == "" } {
      set guid [format "9990%7d" [lmbox dbnextid itv_sequence]]
      lmbox dbput itv_guid $country $guid
    }
    set rec(title) $country
    set rec(url) $url
    set rec(type) wwitv
    set rec(ttl) 86400
    set rec(genre) "Live TV"
    set rec(timestamp) $now
    lmbox dbput itv_channels $guid [array get rec]
    set count 0
    while {1} {
      # Look for the url
      if { [set idx [string first {<tr><td class=} $data]] == -1 } { break }
      set data [string range $data $idx end]
      if { [set idx [string first {</tr>} $data]] == -1 } { break }
      # Cut one row from the page
      set row [string map { \n {} \r {} } [string range $data 0 $idx]]
      # The rest of the page for the next iteration
      set data [string range $data $idx end]
      # Now parse the row
      if { [regexp -nocase {<a class="travel" href="([^"]+)" target="TV">([^<]+)</a>.+javascript:listen\('[^']+','([^']+)',0\)">.+>([^<]+)</td><$} $row d site name link descr] &&
           [string match http://* $link] } {
        set rec(channel) $guid
        set rec(title) [string trim $name]
        set rec(url) [string trim $link]
        set rec(type) wwitv
        set rec(genre) "Live TV"
        set rec(description) [string trim "$site $descr"]
        # See if we have this item already
        if { [set id [lmbox dbget itv_guid $guid.$rec(title)]] == "" } {
          set id [format "9880%7d" [lmbox dbnextid itv_sequence]]
          lmbox dbput itv_guid $guid.$rec(title) $id
        }
        lmbox dbput itv $id [array get rec]
        incr count
      }
    }
    lmbox_puts "Loaded iTV $url for $country, $count stations"
}

# Download and parse RSS feed
proc itv_load_rss { url } {

    if { [set url [string trim $url]] == "" } {
      return
    }
    lmbox_puts "Loading iTV $url ..."
    if { [string match {http://*} $url] } {
      set data [lmbox_geturl $url]
    } else {
      set data [lmbox_getfile $url]
    }
    lmbox varset rss_top "" rss_data "" rss_name "" rss_channel "" rss_url $url
    lmbox xmlparse -data $data -startelement {itv_parse_rss start} -endelement {itv_parse_rss end} -characters {itv_parse_rss data}
    lmbox varunset rss_*
    lmbox_puts "Loaded $url"
}

# XML callback for RSS feed parser
proc itv_parse_rss { tag { name "" } args } {

    set rss_top [lmbox var rss_top]
    set rss_data [lmbox var rss_data]
    switch -- $tag {
     start {
       switch -- $rss_top.$name {
        .channel {
          lmbox varset rss_name "" rss_data "" rss_channel "" rss_top $name
        }

        channel.item {
          # Save channel title for the following links
          if { [lmbox var rss_channel] == "" } {
            set id [itv_channel_new $rss_data]
            lmbox varset rss_channel $id
          }
          lmbox varset rss_name "" rss_data "" rss_top $rss_top.$name
        }

        channel.item.pubDate {
          lmbox varset rss_name timestamp
        }

        channel.item.category -
        channel.channel.category {
          lmbox varset rss_name genre
        }

        channel.item.link {
          lmbox varset rss_name url
        }

        channel.item.enclosure -
        channel.item.media:content {
          array set rec $rss_data
          foreach { key val } $args {
            switch -- $key {
             url {
                set rec(url) $val
             }
             type {
                set rec(mimetype) $val
             }
            }
          }
          lmbox varset rss_data [array get rec]
        }

        channel.item.media:thumbnail -
        channel.item.itunes:image {
          if { [lsearch -exact $rss_data image] == -1 } {
            foreach { key val } $args {
              switch -- $key {
               href - url { lappend rss_data image $val }
              }
            }
            lmbox varset rss_data $rss_data
          }
        }

        channel.image -
        channel.item.image {
          lmbox varappend rss_top .$name
        }

        channel.itunes:subtitle {
          lmbox varset rss_name description
        }

        channel.item.image.url -
        channel.image.url {
          if { [lsearch -exact $rss_data image] == -1 } {
            lmbox varset rss_name image
          }
        }

        channel.ttl -
        channel.title -
        channel.item.guid -
        channel.item.title -
        channel.item.image -
        channel.item.description {
          lmbox varset rss_name $name
        }
       }
     }

     end {
       switch -- $rss_top.$name {
        channel.item.item {
          array set rec $rss_data
          set rec(channel) [lmbox var rss_channel]
          set id [itv_link_new [array get rec]]
          lmbox varset rss_data "" rss_name "" rss_top channel
        }

        channel.image.image {
          lmbox varset rss_top channel rss_name ""
        }

        channel.item.image.image {
          lmbox varset rss_top channel.item rss_name ""
        }

        channel.channel {
          lmbox varset rss_name "" rss_data "" rss_top ""
        }

        default {
          lmbox varset rss_name ""
        }
       }
     }

     data {
       if { [set rss_name [lmbox var rss_name]] == "" ||
            [set name [string trim $name "\n\r\t "]] == "" } {
         return
       }
       array set rec $rss_data
       set key [string tolower $rss_name]
       switch -- $key {
        url {
          if { [set name [string trim $name]] != "" } {
            set rec($key) $name
          }
        }
        default {
          append rec($key) $name
        }
       }
       lmbox varset rss_data [array get rec]
     }
    }
}

# Create new channel from the array
proc itv_channel_new { data } {

    array set rec $data

    if { [set title [lmbox_var rec(title)]] != "" } {
      set rec(timestamp) [clock seconds]
      set rec(type) [lmbox_var rec(type) rss]
      set rec(ttl) [expr [lmbox_var rec(ttl) 0] + [lmbox config itv_ttl 86400]]
      set rec(genre) [lmbox_var rec(genre) "Video Channel"]
      set rec(image) [lmbox_striphtml [lmbox_var rec(image)]]
      set rec(description) [lmbox_striphtml [lmbox_var rec(description)]]
      set rec(url) [lmbox_striphtml [lmbox_var rec(url) [lmbox var rss_url]]]
      if { [set id [lmbox dbget itv_guid $title]] == "" } {
        set id [format "9980%07d" [lmbox dbnextid itv_sequence]]
        lmbox dbput itv_guid $title $id
      }
      lmbox dbput itv_channels $id [array get rec]
      # Retrieve channel image
      set imgfile [lmbox config rss_path [lmbox gethome]/rss]/$id.jpg
      if { $rec(image) != "" && ![file exists $imgfile] } {
        if { [set data [lmbox_geturl $rec(image)]] != "" } {
          lmbox_putfile $imgfile $data
        }
      }
      return $id
    }
    return
}

# Create new link from the given array
proc itv_link_new { data } {

    array set rec $data

    # Without channel ignore the record
    if { [lmbox_var rec(channel)] != "" } {
      # Strip all html tags from the text
      set rec(timestamp) [clock seconds]
      set rec(url) [lmbox_striphtml [lmbox_var rec(url)]]
      set rec(description) [lmbox_striphtml [lmbox_var rec(description)]]
      set rec(image) [lmbox_striphtml [lmbox_var rec(image)]]
      set rec(genre) [lmbox_var rec(genre) "Video Podcast"]
      set rec(type) [lmbox_var rec(type) rss]
      set rec(mimetype) [lmbox_var rec(mimetype) video]
      set guid [lmbox_var rec(guid) $rec(url)]
      # Check for existing link
      if { [set id [lmbox dbget itv_guid $guid]] == "" } {
        set id [format "9870%07d" [lmbox dbnextid rss_sequence]]
        lmbox dbput itv_guid $guid $id
      }
      lmbox dbput itv $id [array get rec]
      # Retrieve item image
      set imgfile [lmbox config rss_path [lmbox gethome]/rss]/$id.jpg
      if { $rec(image) != "" && ![file exists $imgfile] } {
        if { [set data [lmbox_geturl $rec(image)]] != "" } {
          lmbox_putfile $imgfile $data
        }
      }
      return $id
    }
    return
}

# Prepare options page and switch to it
proc itv_details { { new 0 } } {

    foreach key { itvurl itvtitle itvlurl itvltitle itvlgenre } {
      lmbox set $key text "" data ""
    }
    lmbox set itvdescr caption ""
    if { $new == 0 } {
      set info ""
      if { [set channel [lmbox get itvchannels selecteddata]] != "" } {
        array set rec [lmbox dbget itv_channels $channel]
        lmbox set itvurl text [lmbox_var rec(url)] data $channel
        lmbox set itvtitle text [lmbox_var rec(title)]
        append info [lmbox_var rec(description)] "\n\n"
      }
      if { [set link [lmbox get itvlinks selecteddata]] != "" } {
        array set lrec [lmbox dbget itv $link]
        lmbox set itvlurl text [lmbox_var lrec(url)] data $link
        lmbox set itvltitle text [lmbox_var lrec(title)]
        lmbox set itvlgenre text [lmbox_var lrec(genre)]
        append info [lmbox_var lrec(description)]
      }
      lmbox set itvdescr caption $info
    }
    lmbox setpage itvinfopage
}

# Save modified channel/link urls, if channel url is empty, take it from
# the currently selected, if link is empty, just add new channel
proc itv_save {} {

    set url [lmbox get itvurl text]
    set title [lmbox get itvtitle text]

    set lurl [lmbox get itvlurl text]
    set ltitle [lmbox get itvltitle text]
    set lgenre [lmbox get itvlgenre text]

    if { ($url != "" && $title == "") || ($url == "" && $title != "") } {
      lmbox set status caption "Please, specify Channel URL and Title"
      return
    }
    if { ($lurl != "" && $ltitle == "") || ($lurl == "" && $ltitle != "") } {
      lmbox set status caption "Please, specify Link URL and Title"
      return
    }
    # New channel requested
    if { [set channel [lmbox get itvurl data]] == "" } {
      if { $url == "" && $lurl != "" } {
        # Take channel from the list
        set channel [lmbox get itvchannels selecteddata]
      } else {
        # Create new
        set rec(title) $title
        set rec(url) $url
        set rec(genre) "Video Channel $lgenre"
        set channel [itv_channel_new [array get rec]]
        lmbox set itvurl data $channel
        lmbox set status caption "New Channel has been created"
      }
    } else {
      array set rec [lmbox dbget itv_channels $channel]
      set rec(title) $title
      set rec(url) $url
      lmbox dbput itv_channels $channel [array get rec]
      lmbox set status caption "Channel has been updated"
    }
    # New link requested
    if { [set link [lmbox get itvlurl data]] == "" } {
      set lrec(title) $ltitle
      set lrec(guid) $lurl
      set lrec(url) $lurl
      set lrec(genre) $lgenre
      set lrec(channel) $channel
      set link [itv_link_new [array get lrec]]
      lmbox set itvlurl data $link
      lmbox set status caption "New Link has been created"
    } else {
      array set lrec [lmbox dbget itv $link]
      set lrec(title) $ltitle
      set lrec(url) $lurl
      set lrec(genre) $lgenre
      set lrec(channel) $channel
      lmbox dbput itv $link [array get lrec]
      lmbox set status caption "Link has been updated"
    }
}

# Refresh channels
proc itv_refresh {} {

    if { [set link [lmbox get itvurl data]] == "" } {
      lmbox set status caption "This channel is not registered in the database, try saving it first"
      return
    }

    if { [set url [lmbox get itvurl text]] != "" } {
      itv_load $url
    } else {
      lmbox set status caption "Channel url should be specified"
    }
}

# Queue url for downloading
proc itv_queue {} {

    if { [set link [lmbox get itvlurl data]] == "" } {
      lmbox set status caption "This link is not registered in the database, try saving it first"
      return
    }
    if { [set url [lmbox get itvlurl text]] != "" } {
      lmbox dbput itv_queue $url $link
      lmbox_puts "$url has been queued for download"
    } else {
      lmbox set status caption "Link url and link title should be specified"
    }
}

# Deletes channel or link from the info page
proc itv_delete {} {

    # Delete link first
    if { [set link [lmbox get itvlurl data]] != "" } {
      array set rec [lmbox dbget itv $link]
      lmbox dbdel itv_guid $rec(guid)
      lmbox dbdel itv $link
      lmbox_puts "Link $link has been deleted"
    }
    # Delete channel if no links
    if { [set channel [lmbox get itvurl data]] != "" } {
      set count 0
      lmbox dbeval itv {
        if { $id == $rec(channel) } { incr count }
      } -array rec
      if { $count == 0 } {
        array set rec [lmbox dbget itv $link]
        lmbox dbdel itv_guid $rec(title)
        lmbox dbdel itv_channels $channel
        lmbox_puts "Channel $channel has been deleted"
      }
    }
}

