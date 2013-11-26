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

lmbox_register_init weather_init

proc weather_init {} {

    lmbox dbopen weather
    lmbox dbopen weather_config
    # Initial data load
    lmbox_register_check Weather weather_load
    # Add menu item
    lmbox_register_menu Info "Weather" "Weather information" weather.jpg { lmbox setpage weatherpage } 900
    # Config
    lmbox_register_config "Weather Settings" weather_config
}

proc weather_config {} {

    if { [lmbox get weather_station count] == 0 } {
      lmbox dbopen weather_config
      lmbox dbeval weather_config {
        switch -- [lindex $lmbox_dbdata 0] {
         S {
           lmbox call weather_station additemdata $lmbox_dbkey [lindex $lmbox_dbdata 1]
         }
         Z {
           lmbox call weather_zone additemdata $lmbox_dbkey [lindex $lmbox_dbdata 1]
         }
        }
      }
    }
    lmbox setpage weatherconfigpage
}

proc weather_load {} {

    set path [lmbox config weather_path data/Weather]
    if { [file exists $path/weather.txt] && [lmbox dbstat weather_config count] <= 0 } {
      lmbox_puts "Loading weather config..."
      set fd [open $path/weather.txt]
      while { ![eof $fd] } {
        set line [split [gets $fd] |]
        if { [set id [string trim [lindex $line 0]]] == "" ||
             [set type [string trim [lindex $line 1]]] == "" ||
             [set name [string trim [lindex $line 2]]] == "" } {
          continue
        }
        lmbox dbput weather_config $name [list $type $id]
      }
      close $fd
      lmbox_puts "Weather config loaded"
    }
    # Refresh current weather
    weather_schedule
}

# Schedule weather retrieval in the background
proc weather_schedule { { timeout "" } } {

    set now [clock seconds]
    set timestamp [lmbox config weather_timestamp 0]
    set timeout [lmbox_nvl $timeout [lmbox config weather_timeout 7200]]
    if { $now - $timestamp >= $timeout } {
      array set weather { Icon clear.gif Date "" Temperature "" Wind_Cill "" Heat_Index ""
                         Humidity "" Dew_Point "" Wind "" Pressure "" Sky "" Weather ""
                         Visibility "" Precipitation "" City "" State "" Station ""
                         forecast:rowcount 0 forecast:warning "" }
      # Zone Forecasts:
      # Finding a location's Zone ID:
      # http://iwin.nws.noaa.gov/iwin/??/zone.html
      # (replace ?? with your lowercase 2 character state code).
      # Then scroll down until you find the city/area you want
      # listed. Just above that you will find the zone code, in
      # the format of ??Z??? (first 2 chars are state code,
      # followed by "Z", followed by 3 digit zone code).
      set weather_zone [string tolower [lmbox config weather_zone dcz001]]
      set state [string range $weather_zone 0 1]
      set data [lmbox_geturl http://weather.noaa.gov/pub/data/forecasts/zone/$state/$weather_zone.txt]
      if { [string first "WEATHER SERVICE" $data] != -1 } {
        set data [string tolower $data]
        # Position of the forecast data
        set forecast [string first $data "\n."]
        set flag 0
        set count 0
        set buffer ""
        set lines [split $data "\r\n"]
        for { set i 0 } { $i < [llength $lines] } { incr i } {
          set line [lindex $lines $i]
          switch $flag {
           0 {
             # Parse header
             switch -regexp -- $line {
              {.+zone forecasts} {
                 set weather(forecast:area) $line
              }
              {weather service} {
                 set weather(forecast:center) $line
              }
              {^[a-z][a-z]z[0-9][0-9][0-9][>-]} {
                 set weather(forecast:zone) $line
                 set flag 1
              }
             }
           }
           1 {
             # Collecting cities
             if { [string is integer -strict [lindex $line 0]] } {
               set flag 2
               set weather(forecast:date) [string toupper $line]
             } else {
               append weather(forecast:city) " " [string map { ... " " } $line]
             }
           }
           2 {
             # Warning message
             if { [string index $line 0] == "." && [string index $line 1] != "." } {
               incr i -1
               set flag 3
               continue
             }
             append weather(forecast:warning) " " [string map { "..." {} } $line]
           }
           3 {
             # Parse forecast days
             set line2 [lindex $lines [expr $i+1]]
             if { $line2 != "" && ![string match ".*...*" $line2] } {
               append buffer $line " "
               continue
             }
             set line "$buffer $line"
             set buffer ""
             if { ![string match ".*...*" $line] } { 
               # Forecast separator
               if { [string match "*\$\$*" $line] } { break }
               continue
             }
             set data [split $line "."]
             incr count
             set weather(forecast:$count:low) ""
             set weather(forecast:$count:high) ""
             set weather(forecast:$count:temp) ""
             set weather(forecast:$count:wind) ""
             set weather(forecast:$count:day) [lindex $data 1]
             set weather(forecast:$count:weather) [lindex $data 4]
             foreach item [lrange $data 5 end] {
               switch -glob -- $item {
                "*lows*" { append weather(forecast:$count:temp) $item }
                "*highs*" { append weather(forecast:$count:temp) $item }
                "*winds*" { append weather(forecast:$count:wind) $item }
                default { append weather(forecast:$count:weather) $item " " }
               }
             }
             # Assign weather icon
             set weather(forecast:$count:icon) [weather_icon "$weather(forecast:$count:day) $weather(forecast:$count:weather)"]
             # Extract temperature numbers
             set temp $weather(forecast:$count:temp)
             if { [regexp -nocase {Lows[^0-9]* ([0-9]+)} $temp d low] } {
             set weather(forecast:$count:low) "$low F"
             }
             if { [regexp -nocase {Highs[^0-9]* ([0-9]+)} $temp d high] } {
               set weather(forecast:$count:high) "$high F"
             }
           } 
          }
        }
        set weather(forecast:rowcount) $count
      } else {
        lmbox puts "weather_schedule: $weather_zone: ERROR: $data"
      }
      # The National Oceanic and Atmospheric Administration (NOAA,
      # www.noaa.gov) provides easy access to the weather reports
      # generated by a large number of weather stations (mostly at
      # airports) worldwide. To get the station code go to
      # http://www.nws.noaa.gov/tg/siteloc.shtml.
      # Those reports are called METAR reports and
      # are delivered as plain text files that look like this:
      # Station list format: http://www.nws.noaa.gov/oso/site.shtml
      set weather_station [string toupper [lmbox config weather_station KIAD]]
      set data [lmbox_geturl http://weather.noaa.gov/pub/data/observations/metar/decoded/$weather_station.TXT]
      if { [string first "Temperature:" $data] != -1 || [string first "Weather:" $data] != -1 } {
        set count 0
        foreach line [split $data "\r\n"] {
          switch -glob -- $line {
           "Wind:*" { set weather(Wind) [string range $line 6 end] }
           "Visibility:*" { set weather(Visibility) [string range $line 12 end] }  
           "Sky conditions:*" { set weather(Sky) [string range $line 16 end] }
           "Weather:*" { set weather(Weather) [string range $line 9 end] }
           "Precipitation last hour:*" { set weather(Precipitation) [string range $line 25 end] }
           "Temperature:*" { set weather(Temperature) [string range $line 13 end] }
           "Dew Point:*" { set weather(Dew_Point) [string range $line 12 end] }
           "Relative Humidity:*" { set weather(Humidity) [string range $line 20 end] }
           "Pressure (altimeter):*" { set weather(Pressure) [string range $line 22 end] }
           default {
             switch $count {
              0 { set weather(City) $line }
              1 { set weather(Date) $line }
              default { continue }
             }
           }
          }
          incr count
        }
      } else {
        lmbox puts "weather_schedule: $weather_station: ERROR: $data"
      }
      set date [string map { - {} } [lindex [split $weather(Date) /] 0]]
      if { ![catch { set time [clock scan $date] }] } {
        if { [set time [clock format $time -format "%H%M"]] > 2100 || $time < 0700 } {
          set date Night
        }
      }
      set weather(Icon) [weather_icon "$date $weather(Weather) $weather(Sky)"]
      if { $weather(forecast:rowcount) > 0 || ($weather(Weather) != "" || $weather(Temperature) != "") } {
        lmbox dbdel weather
        set name "$weather(Weather) $weather(Sky) $weather(Temperature) $weather(Wind)"
        while { [regsub -all {  } $name { } name] } {}
        lmbox dbput weather 0 [list timestamp $now day Now name $name temphigh $weather(Temperature) templow "" icon $weather(Icon) warning $weather(forecast:warning)]
        for { set i 1 } { $i <= $weather(forecast:rowcount) } { incr i } {
          set rec(day) $weather(forecast:$i:day)
          set rec(temphigh) $weather(forecast:$i:high)
          set rec(templow) $weather(forecast:$i:low)
          set rec(name) "$weather(forecast:$i:weather) $weather(forecast:$i:wind) $weather(forecast:$i:temp)"
          set rec(icon) $weather(forecast:$i:icon)
          set rec(warning) ""
          set rec(timestamp) $now
          while { [regsub -all {  } $name { } name] } {}
          lmbox dbput weather $i [array get rec]
        }
      }
    }
    # See if we have any alerts
    set msgs ""
    set alerts [lmbox config weather_alerts "THUNDERSTORM|TORNADO|SEVERE|HEAT|COLD|FREEZE|ADVISORY"]
    lmbox dbeval weather {
      if { [set warning [lmbox_var rec(warning)]] != "" && [regexp -nocase $alerts $warning] } {
        lappend msgs [string trim $warning]
      }
    } -array rec
    # Show the alert
    if { $msgs != "" } {
      set msgs [join $msgs ", "]
      lmbox set status caption "WEATHER ALERT: $msgs"
      lmbox_log WEATHER $msgs
    }
}

proc weather_setup {} {

   lmbox dbeval weather {
      set id $lmbox_dbkey
      if { $id == 0 } {
        lmbox call iweather$id zoomimage images/weather/$rec(icon) 0 0
        set title [string totitle "$rec(day) $rec(name)"]
        if { [set warning [lmbox_var rec(warning)]] != "" } {
          append title "\n\n^#f82a2f" [string totitle [string trim $warning]]
        }
        lmbox set weather$id caption $title
      } else {
        lmbox set weather$id caption [string totitle $rec(day)]
        lmbox varset weather$id [string totitle $rec(day)]
        lmbox set iweather$id enabled 1
        lmbox set iweather$id borderstyle normal
        lmbox set iweather$id background images/weather/$rec(icon)
        lmbox set iweather$id pressedbackground images/weather/$rec(icon)
        lmbox set iweather$id focusedbackground images/weather/$rec(icon)
        lmbox varset iweather$id $rec(name)
        set temp $rec(temphigh)
        if { $rec(templow) != "" } {
          if { $temp != "" } { append temp " / " }
          append temp $rec(templow)
        }
        lmbox set tweather$id caption $temp
      }
    } -array rec
    incr id
    while { $id <= 6 } {
        lmbox set iweather$id borderstyle none
        lmbox set iweather$id enabled 0
        lmbox set iweather$id background none.gif
        incr id
    }
    lmbox call iweather1 setfocus
    weather_info iweather1
}

# How to setup radar config:
# Go to www.seather.com, find your city/zipcode, click on radar screen. Large radar web page will be
# shown. If using  Mozilla, right click on radar image and choose 'View Image'. Image url will look like
# http://image.weather.com/web/radar/us_lax_closeradar_large_usen.jpg.
# You need lax part of it. Then go to Settings, click on Weather button and enter lax in the Radar:
# textfield.
proc weather_radarsetup { { refresh 0 } } {

    set now [clock seconds]
    set radar [lmbox config weather_radar dca]
    set file /tmp/$radar.jpg
    if { $refresh || ![file exists $file] || $now - [file mtime $file] > 300 } {
      set data [lmbox_geturl http://image.weather.com/web/radar/us_${radar}_closeradar_large_usen.jpg]
      if { $data != "" } {
        lmbox_putfile $file $data
      }
    }
    lmbox call weatherradar zoomimage $file 0 0
}

# Refresh weather data
proc weather_refresh {} {

    weather_schedule -1
    weather_setup
}

proc weather_info { { name "" } } {

    if { $name == "" } { set name [lmbox get this name] }
    set day [lmbox var [string range $name 1 end]]
    set text [lmbox var $name]
    lmbox set status caption "$day $text"
    for { set i 1 } { $i <= 6 } { incr i } {
      lmbox set weather$i caption [lmbox var weather${i}]
    }
    lmbox set [string range $name 1 end] caption [string toupper $day]
}

proc weather_icon { name } {

    if { [regexp {([0-9]+) Percent} $name d percent] && $percent >= 50 } {
      switch -regexp -- $name {
       "Snow|snow" { return snow.gif }
       "Thunder|thunder" { return tstorm.gif }
       "Rain|rain" { return rain.gif }
       "Showers|showers" { return showers.gif }
      }
    }

    switch -regexp -- $name {
     "Mostly.*Cloudy|mostly.*cloudy" {
        if { [regexp -nocase Night $name] } { return ncloudy.gif }
        return mcloudy.gif
     }
     "Partly.*Cloudy|partly.*cloudy" {
        if { [regexp -nocase Night $name] } { return ncloudy.gif }
        return pcloudy.gif
     }
     "Cloudy|cloudy" {
        if { [regexp -nocase Night $name] } { return ncloudy.gif }
        return cloudy.gif
     }
     "Fog|fog" { return fog.gif }
     "Mist|mist" { return misty.gif }
     "Snow.*Rain|snow.*rain" { return snowshowers.gif }
     "Snow.*Flurr|snow.*flurr" { return snowshowers.gif }
     "Snow|snow" { return snow.gif }
     "Showers|showers" { return showers.gif }
     "Rain|rain" { return rain.gif }
     "Thunder|thunder" { return tstorm.gif }
     "Haze|haze" { return haze.gif }
     "Sun|sun" { return sunny.gif }
     "Freeze.*Drizzle|freeze.*drizzle" { return freezingdrizzle.gif }
     "Drizzle|drizzle" { return drizzle.gif }
     "Freeze.*Grain|freeze.*grain" { return freezinggrain.gif }
     "Wind|wind" { return wind.gif }
     "Cold|cold" { return cold.gif }
     "Hot|hot" { return hot.gif }
     "Sleet|sleet" { return sleet.gif }
     "Dust|dust" { return dust.gif }
     "Fair|fair" { return fair.gif }
     default {
        if { [regexp -nocase Night $name] } { return nclear.gif }
        return clear.gif
     }
    }
}
