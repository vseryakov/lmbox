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

lmbox_register_init live365_init

proc live365_init {} {

    lmbox dbopen live365_genres
    lmbox dbopen live365_stations
    lmbox dbopen live365_playlist
    # Initial data load
    lmbox_register_check Live365 live365_load
    # Main menu entry
    lmbox_register_menu Music "Live365.com" "Listen radio from live365.com" live365.jpg { lmbox setpage live365page } 670
    # Config page
    lmbox_register_config "Live365.com Account" "lmbox setpage live365configpage"
}

# Load the whole live365 database
proc live365_load { { refresh 0 } } {

    if { $refresh || [lmbox dbstat live365_stations count] == 0 } {
      lmbox_setconfig live365_timestamp 0
    }
    live365_login
    set now [clock seconds]
    set timestamp [lmbox config live365_timestamp 0]
    set timeout [lmbox config live365_timeout 604800]
    if { $now - $timestamp >= $timeout } {
      lmbox_puts "Reloading live365.com database..."
      live365_loadgenres
      lmbox varset live365_rows 0 live365_first 1
      lmbox dbdel live365_stations
      live365_loaddirectory
      # Load remaining pages
      for { set first 201 } { $first < [lmbox var live365_rows 0] } { incr first 200 } {
        live365_loaddirectory $first
      }
      lmbox_setconfig live365_timestamp $now
      lmbox_puts "live365.com database loaded"
    }
}

proc live365_setup {} {

    lmbox call live365genrelist clear
    lmbox call live365stationlist clear
    lmbox dbeval live365_genres {
      lmbox call live365genrelist additemdata $rec(display_name) $rec(name)
    } -array rec
    lmbox call live365genrelist sort
    if { [lmbox get player status] == "play" && [set status [lmbox var live365_status]] != "" } {
      lmbox set status caption "Now Playing: $status"
    }
}

proc live365_info { type { data "" } } {

    switch -- $type {
     stations {
        lmbox call live365stationlist clear
        lmbox dbeval live365_stations {
          if { [regexp -nocase $data $rec(station_genre)] } {
            lmbox call live365stationlist additemdata $rec(station_title) $lmbox_dbkey
          }
        } -array rec
     }

     info {
       array set rec [lmbox dbget live365_stations $data]
       if { [info exists rec(station_title)] } {
         set info ""
         foreach key { title location genre quality_level description } {
           set value [lmbox_var rec(station_$key)]
           if { $value == "" } { continue }
           set name [string totitle $key]
           switch -- $name {
            Quality_level {
              append info "Quality: $value kbps / Stream [lmbox_var rec(station_connection) 0] kbps\n"
            }
            default {
              append info "$name: $value\n"
            }
           }
         }
         lmbox set live365info caption $info
       }
     }

     status {
        # Check expiration of the playlist
        array set rec [lmbox dbget live365_playlist $data]
        if { [clock seconds] >= [lmbox_var rec(expires) 0] } {
          live365_loadplaylist $data
        }
        array set rec [lmbox dbget live365_playlist $data]
        set secs [lmbox_var rec(seconds) 0]
        set mins [expr $secs / 60]
        set secs [expr $secs % 60]
        set status "[lmbox_var rec(artist)]: [lmbox_var rec(title)], [lmbox_var rec(album)], \[$mins:$secs\]"
        lmbox set status caption "Now Playing: $status"
        lmbox varset live365_status $status
        # Schedule next timer when playlist will expire
        if { [set interval [expr ([lmbox_var rec(expires) 0] - [clock seconds])*1000]] <= 0 } {
          set interval 30000
        }
        lmbox call player settimer $interval "live365_info status $data"
     }
    }
}

proc live365_play { id } {

    live365_login
    set session_id [lmbox config live365_sessionid]
    array set rec [lmbox dbget live365_stations $id]
    if { ![info exists rec(station_address)] } { return }
    lmbox run "live365_info status $id"
    lmbox call player stop
    lmbox call player clear
    lmbox call player add $rec(station_address)?sessionid=$session_id
    player_audiofilter player
    lmbox run "lmbox call player play"
    # Add the link temporary to favorite links
    set expires [expr [clock seconds] + [lmbox config recent_lifetime 86400]]
    live365_favorite $id expires $expires class History
}

proc live365_favorite { id args } {

    set class ""
    set expires 0
    foreach { key val } $args { set $key $val }

    array set rec [lmbox dbget live365_stations $id]
    if { ![info exists rec(station_address)] } { return }
    favorites_add Music "script:live365_play $id" $rec(station_title) description $rec(station_genre) icon images/menu/live365.jpg expires $expires class $class
    lmbox set status caption "$rec(station_title) added to favorites"
}

proc live365_pause {} {

    lmbox call player pause
}

proc live365_stop {} {

    lmbox call player stop
}


# Login xml format:
#
# <LIVE365_API_LOGIN_CGI>
#  <Code>0</Code>
#  <Reason>Success</Reason>
#  <Session_ID>vseryakov:3443243s</Session_ID>
#  <Application_ID>live365:BROWSER</Application_ID>
#  <Device_ID>UNKNOWN</Device_ID>
#  <Member_Status>REGULAR</Member_Status>
#  <Acl_station_count>0</Acl_station_count>
# </LIVE365_API_LOGIN_CGI>
#
proc live365_login { { relogin 0 } } {

    if { $relogin } {
      lmbox_setconfig live365_sessionid ""
    }
    set now [clock seconds]
    if { [set session_id [lmbox config live365_sessionid]] != "" } {
      # Still valid
      if { $now - [lmbox config live365_sessiontime 0] < 86400 } {
        return 0
      }
      set session_id ""
    }
    set user [lmbox config live365_user]
    set passwd [lmbox config live365_passwd]
    if { $user != "" && $passwd != "" && $session_id == "" } {
      set url "http://www.live365.com/cgi-bin/api_login.cgi?action=login&remember=Y&org=live365&member_name=$user&password=$passwd"
      set data [lmbox_geturl $url]
      if { [regexp -nocase {\<Code\>0\<\/Code\>} $data d code] &&
           [regexp -nocase {\<Session_ID\>([^\<]+)\<\/Session_ID\>} $data d session_id] } {
        lmbox_setconfig live365_sessionid $session_id
        lmbox_setconfig live365_sessiontime $now
        lmbox puts "live365 logged in: $session_id"
        return 0
      }
    }
    return -1
}

proc live365_logout {} {

    set session_id [lmbox config live365_sessionid]
    set url "http://www.live365.com/cgi-bin/api_login.cgi?action=logout&sessionid=$session_id&org=live365"
    lmbox_geturl $url
    return
}

# Genres xml format:
# <LIVE365_API_GENRES_CGI>
#   <Genre>
#    <Display_Name>Alternative</Display_Name>
#    <Name>alternative</Name>
#    <Ad_Category>alternative</Ad_Category>
#    <ID>100</ID>
#    <Parent_ID>0</Parent_ID>
#    <Parent_Name>ROOT</Parent_Name>
#    <Related_Genres>1607,1606,1403,2006,2308</Related_Genres>
#    <Description_HTML><![CDATA[<b>Alternative</b> is one of those labels that means totally different things]]></Description_HTML>
#    <Community_Link_HTML><![CDATA[Talk about Alternative]]></Community_Link_HTML>
#    <Community_Link_URL><![CDATA[http://forums.live365.com/viewforum.php?f=15]]></Community_Link_URL>
#   </Genre>
#
proc live365_loadgenres {} {

    lmbox_puts "Loading live365.com genres..."
    set session_id [lmbox config live365_sessionid]
    set url "http://www.live365.com/cgi-bin/api_genres.cgi?format=xml&sessionid=$session_id"
    if { [catch {
      lmbox dbdel live365_genres
      set data [lmbox_geturl $url]
      lmbox xmlparse -data $data -startelement {live365_xml start} -endelement {live365_xml end} -characters {live365_xml data}
    } errmsg] } {
      lmbox puts "live365_loadgenres: $errmsg"
    }
    lmbox_puts "Live365.com genres loaded"
}

# Stations xml format:
# <DIRECTORY_TOTAL_ROWS>600</DIRECTORY_TOTAL_ROWS>
# <LIVE365_STATION><STREAM_ID>1319062</STREAM_ID>
#  <STATION_ID>312920</STATION_ID>
#  <STATION_ADDR></STATION_ADDR>
#  <STATION_ADDRESS>http://www.live365.com/play/kkjz1</STATION_ADDRESS>
#  <STATION_BROADCASTER>kkjz1</STATION_BROADCASTER>
#  <STATION_BROADCASTER_URL><![CDATA[http://www.jazzandblues.org]]></STATION_BROADCASTER_URL>
#  <STATION_TITLE><![CDATA[KKJZ 88.1 FM]]></STATION_TITLE>
#  <STATION_DESCRIPTION><![CDATA[KKJZ-FM Jazz and Blues from Long Beach, CA]]></STATION_DESCRIPTION>
#  <STATION_KEYWORDS><![CDATA[Jazz Blues Latin K-JAZZ Long Beach B-Bop KKJZ KJAZZ]]></STATION_KEYWORDS>
#  <STATION_GENRE><![CDATA[jazz, blues]]></STATION_GENRE>
#  <STATION_CONNECTION>64</STATION_CONNECTION>
#  <STATION_CODEC>mp3PRO</STATION_CODEC>
#  <STATION_QUALITY_LEVEL>264</STATION_QUALITY_LEVEL>
#  <STATION_SOURCE>live365</STATION_SOURCE>
#  <STATION_RATING>7.85</STATION_RATING>
#  <STATION_LISTENERS_ACTIVE_REG>758</STATION_LISTENERS_ACTIVE_REG>
#  <STATION_LISTENERS_ACTIVE_PM>11</STATION_LISTENERS_ACTIVE_PM>
#  <STATION_LISTENERS_ACTIVE>769</STATION_LISTENERS_ACTIVE>
#  <STATION_LISTENERS_MAX>1200</STATION_LISTENERS_MAX>
#  <STATION_TLH_30_DAYS>436992</STATION_TLH_30_DAYS>
#  <LIVE365_ATTRIBUTES><STATION_ATTR>[Professional]</STATION_ATTR></LIVE365_ATTRIBUTES>
#  <LIVE365_ATTRIBUTES_CODES>LPRX</LIVE365_ATTRIBUTES_CODES>
#  <LISTENER_ACCESS>PUBLIC</LISTENER_ACCESS>
#  <STATION_STATUS>OK</STATION_STATUS>
#  <STATION_SERVER_MODE>LI</STATION_SERVER_MODE>
#  <STATION_SEARCH_SCORE>0.9998904</STATION_SEARCH_SCORE>
# </LIVE365_STATION>
#
proc live365_loaddirectory { { first 1 } } {

    lmbox_puts "Loading live365.com database ($first)..."
    set session_id [lmbox config live365_sessionid]
    set url "http://www.live365.com/cgi-bin/directory.cgi?site=xml&access=ALL&first=$first&rows=200&genre=All&searchgenre=All&searchfields=T:A:C&sort=L:D;R:D"
    if { [catch {
      set data [lmbox_geturl $url]
      lmbox xmlparse -data $data -startelement {live365_xml start} -endelement {live365_xml end} -characters {live365_xml data}
    } errmsg] } {
      lmbox puts "live365_loaddirectory: $errmsg"
    }
}

# Playlist xml format:
#  <Refresh>31</Refresh>
#  <Broadcaster>pieter_56</Broadcaster>
#  <PlaylistEntry>
#   <Title>Lot of love</Title>
#   <Artist>Melba Moore</Artist>
#   <Album>Falling</Album>
#   <ECommerceURL> </ECommerceURL>
#   <FileName> </FileName>
#   <trackType/>
#   <desc/>
#   <clickThruURL/>
#   <visualURL/>
#   <Seconds>0</Seconds>
#  </PlaylistEntry>
#
proc live365_loadplaylist { station_id } {

    set session_id [lmbox config live365_sessionid]
    set url "http://www.live365.com/pls/front?handler=playlist&cmd=view&handle=$station_id&viewType=xml&sessionid=$session_id"
    if { [catch {
      lmbox varset live365_station "" live365_refresh 0
      set data [lmbox_geturl $url]
      lmbox xmlparse -data $data -startelement {live365_xml start} -endelement {live365_xml end} -characters {live365_xml data}
    } errmsg] } {
      lmbox puts "live365_loadplaylist: $errmsg"
    }
}

proc live365_loadstation { station_id } {

    set session_id [lmbox config live365_sessionid]
    set url "http://www.live365.com/cgi-bin/station_info.cgi?format=xml&in=STATIONS&channel=$station_id&sessionid=$session_id"
}

proc live365_loadpresets {} {

    set session_id [lmbox config live365_sessionid]
    set url "http://www.live365.com/cgi-bin/api_presets.cgi?action=get&sessionid=$session_id&device_id=UNKNOWN&app_id=live365:BROWSER&first=1&rows=200&access=ALL&format=xml"
}

proc live365_xml { tag { name "" } args } {

    switch -- $tag {
     start {
       switch -- [lmbox var live365_top]$name {
        Genre -
        Playlist -
        PlaylistEntry -
        LIVE365_STATION {
          lmbox varset live365_top $name live365_key ""
        }

        GenreName -
        GenreDisplay_Name -
        GenreParent_Name -
        PlaylistEntryTitle -
        PlaylistEntryArtist -
        PlaylistEntryAlbum -
        PlaylistEntrySeconds -
        PlaylistBroadcaster -
        PlaylistRefresh -
        LIVE365_STATIONSTATION_ID -
        LIVE365_STATIONSTATION_TITLE -
        LIVE365_STATIONSTATION_ADDRESS -
        LIVE365_STATIONSTATION_BROADCASTER -
        LIVE365_STATIONSTATION_BROADCASTER_URL -
        LIVE365_STATIONSTATION_DESCRIPTION -
        LIVE365_STATIONSTATION_QUALITY_LEVEL -
        LIVE365_STATIONSTATION_CONNECTION -
        LIVE365_STATIONSTATION_LOCATION -
        LIVE365_STATIONSTATION_GENRE -
        LIVE365_STATIONSTATION_CODEC {
          lmbox varset live365_key $name
        }

        DIRECTORY_TOTAL_ROWS {
          lmbox varset live365_top $name live365_key $name
        }

        default {
          # Ignore other fields
          lmbox varset live365_key ""
        }
       }
     }

     end {
       # Put all colected key:value pairs into array
       foreach { key value } [lmbox var live365_data] {
         set rec([string tolower $key]) [live365_strip $value]
       }
       switch -- [lmbox var live365_top].$name {
        Genre.Genre {
          set name [lmbox_var rec(name)]
          set parent [lmbox_var rec(parent_name)]
          if { $name != "" } {
            if { $parent != "ROOT" } {
              set rec(display_name) "[string totitle $parent] $rec(display_name)"
            }
            lmbox dbput live365_genres $name [array get rec]
          }
          live365_reset
        }

        Playlist.Refresh {
          lmbox varset live365_refresh [lmbox_var rec(refresh) 0]
        }

        Playlist.Broadcaster {
          set station [lmbox_var rec(broadcaster)]
          if { $station != "" } {
            lmbox varset live365_plscount 0
            lmbox varset live365_station $station
          }
          live365_reset
        }

        PlaylistEntry.PlaylistEntry {
          set station [lmbox var live365_station]
          # Keep only current and previuos entries
          if { $station != "" } {
            # Timestamp when this playlist expires
            set rec(expires) [expr [clock seconds] + [lmbox var live365_refresh 0]]
            switch -- [set plscount [lmbox var live365_plscount 0]] {
             0 { lmbox dbput live365_playlist $station [array get rec] }
             1 { lmbox dbput live365_playlist $station:previous [array get rec] }
            }
            lmbox varset live365_plscount [incr plscount]
          }
          live365_reset
        }

        LIVE365_STATION.LIVE365_STATION {
          set station [lmbox_var rec(station_broadcaster)]
          if { $station != "" } {
            lmbox dbput live365_stations $station [array get rec]
          }
          live365_reset
        }

        DIRECTORY_TOTAL_ROWS.DIRECTORY_TOTAL_ROWS {
          lmbox varset live365_rows [lmbox_var rec(directory_total_rows)]
          live365_reset
        }
       }
     }

     data {
       # Collect data between tags
       if { [set data [string trim $name "\n\r\t"]] == "" ||
            [set key [lmbox var live365_key]] == "" ||
            [set top [lmbox var live365_top]] == "" } {
         return
       }
       set key_list [lmbox var live365_data]
       lappend key_list $key $data
       lmbox varset live365_data $key_list
     }
    }
}

proc live365_reset {} {

    lmbox varset live365_top "" live365_key "" live365_data ""
}

proc live365_strip { data } {

    return [string map { {![CDATA[} {} {]]} {} } $data]
}
