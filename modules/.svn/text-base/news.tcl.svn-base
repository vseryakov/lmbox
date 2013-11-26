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

lmbox_register_init news_init

proc news_init {} {

    lmbox dbopen news
    lmbox dbopen news_guid
    # Update news hourly
    lmbox_register_check News news_schedule
    # Add menu item
    lmbox_register_menu Info "News" "World news" yahoonews.jpg { lmbox setpage newspage } 1010
    # Cache directory should exist
    set news_path [lmbox config news_path [lmbox gethome]/news]
    if { [catch { file mkdir $news_path } errmsg] } { lmbox puts $errmsg }
}

proc news_setup { { filter "" } } {

    lmbox call newslist clear
    lmbox dbeval news {
      if { $filter != "" && ![expr $filter] } { continue }
      lmbox call newslist additemdata "$category: $title" $lmbox_dbkey
    } -split
    lmbox call newslist sort
}

proc news_link { id } {

    array set rec [lmbox dbget news $id]
    if { ![info exists rec(link)] } { return }
    web_page $rec(link) 0
}

proc news_info { id } {

    array set rec [lmbox dbget news $id]
    if { ![info exists rec(title)] } { return }
    set news_path [lmbox config news_path [lmbox gethome]/news]
    set icon [lmbox_icon $id $news_path]
    lmbox set newstitle caption $rec(title)
    lmbox set newsdescr caption "$rec(timestamp)\n$rec(description)"
    lmbox set newscategory caption "$rec(category) NEWS"
    lmbox call newsicon zoomimage [lmbox_nvl $icon menu/yahoonews.jpg] 0 0
}

proc news_schedule {} {

    if { [lmbox dbstat news count] == 0 } {
      lmbox setconfig news_timestamp 0
    }
    # Update every hour
    set now [clock seconds]
    if { $now - [lmbox config news_timestamp 0] < 3600 } {
      return
    }
    # Allow only one instance to do that
    if { [lmbox var news:flag 0] == 1 } {
      lmbox puts "news_schedule: already running"
      return
    }
    lmbox varset news:flag 1
    lmbox puts "news_parse: started"
    # Delete old news entries
    lmbox dbdel news
    news_parser topstories GENERAL
    news_parser us REGION
    news_parser world WORLD
    news_parser business BUSINESS
    news_parser tech TECH
    news_parser science SCIENCE
    news_parser sports SPORTS
    news_parser entertaiment ENTERTAINMENT
    news_parser health HEALTH
    lmbox puts "news_parse: stopped"
    lmbox_setconfig news_timestamp $now
    # Cleanup old files
    set news_path [lmbox config news_path [lmbox gethome]/news]
    foreach file [glob -nocomplain $news_path/*.jpg] {
      if { [lmbox dbget news [file tail [file rootname $file]]] == "" } {
        file delete -force $file
      }
    }
    # Replace news list with new entries
    if { [lmbox getpage] == "newspage" } {
      news_setup
    }
    lmbox varset news:flag 0
}

proc news_parser { rss category } {

    lmbox varset news_category $category
    # Download and parse the XML file
    if { [catch {
      set data [lmbox_geturl http://rss.news.yahoo.com/rss/$rss]
      lmbox xmlparse -data $data -startelement {news_xml start} -endelement {news_xml end} -characters {news_xml data}
    } errmsg] } {
      lmbox puts "news_parse: $errmsg"
    }
    lmbox varunset news_*
}

proc news_xml { tag { name "" } args } {

    switch -- $tag {
     start {
       switch -- $name {
        item { lmbox varunset news_name news_data news_image }
        pubDate { lmbox varset news_name timestamp }
        media:content {
          foreach { key val } $args {
            if { $key == "url" } { lmbox varset news_image $val }
          }
        }
        default { lmbox varset news_name $name }
       }
     }

     end {
       switch -- $name {
        item {
          foreach { key value } [lmbox var news_data] { append rec([string tolower $key]) $value }
          set rec(category) [lmbox var news_category]
          # Strip all html tags from the text
          set rec(description) [lmbox_striphtml $rec(description)]
          # guid should exist
          if { [set guid [lmbox_var rec(guid)]] != "" } {
            if { [set id [lmbox dbget news_guid $guid]] == "" } {
              set id [format "%06d" [lmbox dbnextid news_sequence]]
              lmbox dbput news_guid $guid $id
            }
            lmbox dbput news $id [array get rec]
            # Retrieve news image
            set imgfile [lmbox config news_path [lmbox gethome]/news]/$id.jpg
            if { [set imgurl [lmbox var news_image]] != "" && ![file exists $imgfile] } {
              set imgurl [lindex [split $imgurl ?] 0]
              if { [set data [lmbox_geturl $imgurl]] != "" } {
                lmbox_putfile $imgfile $data
              }
            }
          }
        }
       }
     }

     data {
       if { [set name [string trim $name "\n\r\t"]] == "" } { return }
       lmbox varset news_data "[lmbox var news_data] {[lmbox var news_name]} {$name}"
     }
    }
}
