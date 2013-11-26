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

lmbox_register_init movie_init

proc movie_init {} {

    lmbox dbopen movies
    lmbox dbopen movies_genre
    lmbox_register_check Movies movie_load
    lmbox_register_menu Video "New Releases" "Show all new movies/cartoons" newrelease.jpg { movie_setup New30; lmbox setpage movie_mosaicpage } 100
    lmbox_register_menu Video "Movies" "Browse movies by cover picture in mosaic view" moviem.jpg { movie_setup Movie;lmbox setpage movie_mosaicpage } 110
    lmbox_register_menu Video "Cartoons" "Browse cartoons by cover picture in mosaic view" cartoonm.jpg { movie_setup Cartoon;lmbox setpage movie_mosaicpage } 120
    lmbox_register_menu Video "Movie List" "List of all movies" movie.jpg { movie_setup Movie; lmbox setpage moviepage } 130
    lmbox_register_menu Video "Cartoon List" "List of all cartoons" cartoon.jpg { movie_setup Cartoon; lmbox setpage moviepage } 140
    lmbox_register_menu Video "Search" "Search movies by genre or keywords" search.jpg { lmbox varset movie_type ""; lmbox setpage moviesearchpage } 150
    lmbox_register_menu Video "Browse" "Browse movie catalog by genre" browse.jpg { lmbox setpage moviebrowsepage } 160
    lmbox_register_menu Video "Recent" "Show recently watched movies" recent.jpg { movie_setup LastSeen; lmbox setpage movie_mosaicpage } 170

    lmbox_register_menu Personal "Home Videos" "Watch video from personal video catalog" video.jpg { movie_setup Clip; lmbox setpage moviepage } 300
    # Create local directories
    foreach dir { movies videos covers } {
      if { [catch { file mkdir [lmbox gethome]/$dir } errmsg] } { lmbox puts $errmsg }
    }
    lmbox setconfig movie_pattern {\.(avi|mpg|mpeg|mov|wma)$}
}

# Load all movies on first call into the database
proc movie_load { args } {

    set count 0
    set movie_file [lmbox config movie_file data/Movies/movies.txt]
    if { ![file exists $movie_file] } { return }
    # check last time we loaded movie file
    set movie_mtime [file mtime $movie_file]
    if { [lmbox dbstat movies count] == 0 || $movie_mtime > [lmbox config movie_mtime 0] } {
      lmbox_puts "Loading movies..."
      # Movie meta file exists
      set fd [open $movie_file]
      while { ![eof $fd] } {
        set line [string trim [gets $fd]]
        if { [string index $line 0] == "#" } { continue }
        set line [split $line |]
        if { [set id [lindex $line 0]] == "" } { continue }
        # Initialize with required fields
        set rec(timestamp) 0
        if { [set rec(title) [lindex $line 1]] == "" } { continue }
        if { [set rec(files) [lindex $line 2]] == "" } { continue }
        set rec(create_time) [lindex $line 3]
        set rec(update_time) [lindex $line 4]
        set rec(genre) [lindex $line 5]
        set rec(description) ""
        foreach line [split [lmbox_wrap [string map { {\n} \n } [lindex $line 6]]] "\n"] {
          if { [regexp {^[[a-zA-Z ]+:} $line] && $rec(description) != "" } {
            append rec(description) "\n"
          }
          append rec(description) $line
        }
        # Preserve watching timestamp from existing record
        if { [set timestamp [lmbox dbget system movie:$id]] != "" } {
          set rec(timestamp) $timestamp
        }
        lmbox dbput movies $id [array get rec]
        incr count
      }
      close $fd
      # Mark the last time we loaded full movies file
      lmbox_setconfig movie_mtime $movie_mtime
      lmbox_puts "$count movies loaded"
      lmbox dbsync
      movie_genres init
      movie_snapshots
    }
}

# Generating move genre list
proc movie_genres { type { name moviegenrelist } } {

    switch -- $type {

     init {
       lmbox dbeval movies {
         foreach key $genre {
           if { ![regexp {^[0-9]+$} $key] } {
             set movies_genre($key) 1
           }
         }
       } -split
       lmbox dbdel movies_genre
       foreach genre [array names movies_genre] {
         lmbox dbput movies_genre $genre ""
       }
     }

     list {
       if { [lmbox get $name count] == 0 } {
         lmbox dbeval movies_genre {
           lmbox call $name additem $lmbox_dbkey
         }
       }
     }
    }
}

# Generating movie icons
proc movie_snapshots { { path "" } } {

    set movies ""
    set now [clock seconds]
    if { $path == "" } {
      lmbox dbeval movies {
        set icon [movie_icon $lmbox_dbkey]
        if { $icon == "none.gif" && $now - $update_time > 86400 } {
          lappend movies $lmbox_dbkey
        }
      } -split
    } else {
      foreach file [lmbox_files $path [lmbox config movie_pattern]] {
        set icon [movie_icon $file]
        if { $icon == "none.gif" } { lappend movies $file }
      }
    }
    foreach id $movies {
      movie_snapshot $id
    }
}

proc movie_setup { { type "" } { filter "" } } {

    set path ""
    set now [clock seconds]
    set movie_file [lmbox config movie_file data/Movies/movies.txt]
    set movie_path [list [lmbox gethome]/movies [lmbox config movie_path data/Movies]]

    # Start movie list setup
    if { $type != "" } {
      lmbox varset movie_type $type
    }
    set movie_type [lmbox var movie_type]
    set selected [lmbox get movie_list selectedtext]
    set offset [lmbox var movie_offset]

    lmbox varunset movie:*
    lmbox varset movie_offset 0

    lmbox call movie_list clear
    # Check main movie category
    switch -glob -- $movie_type {
     "/*" -
     Clip -
     Show {
        switch -glob -- $movie_type {
	 "/*" {
	   set path $movie_type
	   set genre "Media file"
	 }
	 Clip {
	   set path [list [lmbox gethome]/videos [lmbox config camera_path data/Videos]]
	   set genre "Home Videos"
	 }
	 Show {
           set path [list [lmbox gethome]/tv [lmbox config tv_path data/TV]]
	   set genre "Recorded Show"
	 }
	}
     }
     Lookup {
        set movie_type ""
        lmbox set movieplay caption Select
        if { ![file exists $movie_file] } {
          set path $movie_path
          set genre Movie
        }
     }
     default {
        if { ![file exists $movie_file] } {
          set path $movie_path
          set genre $movie_type
        }
     }
    }
    # Plain file list, no meta information
    if { $path != "" } {
      foreach file [lmbox_files $path [lmbox config movie_pattern]] {
        set title [file rootname [file tail $file]]
        lmbox call movie_list additemdata $title $file
      }
      # Run background icon checker
      lmbox run "movie_snapshots {$path}"
      return
    }
    set admin_mode [lmbox_admin_mode]
    set teen_mode [lmbox_teen_mode]
    set adult_mode [lmbox_adult_mode]
    set adult_filter [lmbox config movie_filter Adult]
    # Load movie list according to current filter
    set movie_list ""
    lmbox dbeval movies {
      # Adult restriction: password expires every minute
      if { !$adult_mode && [regexp $adult_filter $genre] } {
        continue
      }
      # Restriction: movies password expires every hour
      if { !$admin_mode } {
        if { $teen_mode } {
          if { ![regexp {Cartoon|Children|Teens} $genre] } {
            continue
          }
        } else {
          if { ![regexp {Cartoon|Children} $genre] } {
            continue
          }
        }
      }
      # Main category filter
      switch -glob -- $movie_type {
       New* {
         set days [string range $movie_type 3 end]
         if { !(($create_time > 0 && $now - $create_time < $days*86400) ||
                ($update_time > 0 && $now - $update_time < 10*86400)) } {
           continue
         }
       }
       LastSeen {
         if { $timestamp <= 0 || $now - $timestamp > 7*86400 } { continue }
       }
       default {
         if { $movie_type != "" && ![regexp $movie_type $genre] } { continue }
       }
      }
      # Additional filter
      if { $filter != "" && ![expr $filter] } { continue }
      # Mutual exclusion of in main categories
      switch -- $movie_type {
       Movie {
         if { [regexp {Cartoon} $genre] } { continue }
       }
      }
      lappend movie_list [list $title $lmbox_dbkey $timestamp $create_time]
    } -split
    # Sort according to page
    switch -glob -- $movie_type {
     New {
       set movie_list [lsort -index 3 -integer $movie_list]
     }
     LastSeen {
       set movie_list [lsort -index 2 -integer -decreasing $movie_list]
     }
     default {
       set movie_list [lsort -index 0 $movie_list]
     }
    }
    # Update listbox widget
    foreach item $movie_list {
      lmbox call movie_list additemdata [lindex $item 0] [lindex $item 1]
    }
    # Point to the same title
    lmbox set movie_list selectedindex [lmbox call movie_list searchitem $selected]
    if  { $offset > 0 && $offset < [lmbox get movie_list count] } {
      lmbox varset movie_offset $offset
    }
    if { $filter != "" } {
      lmbox puts "movie_search: $movie_type: $filter"
    }
}

proc movie_info { type id } {

    foreach key { create_time timestamp title genre files description } { set $key "" }
    if { [regexp {^[0-9]+$} $id] } {
      foreach { key val } [lmbox dbget movies $id] { set $key $val }
      if { [set nfiles [llength $files]] > 1 } { append genre " Parts #1-$nfiles" }
      set files [movie_files $files]
    } else {
      if { [file exists $id] } {
        set title $id
        set files $id
        set timestamp [file atime $id]
        set create_time [file mtime $id]
        set genre "[string toupper [string trim [file ext $id] "."]] File Size [lmbox_size [file size $id]]"
        set description ""
      }
    }
    if { $create_time > 0 } { append genre " Added [clock format $create_time -format "%m/%d/%y %H:%M"]" }
    if { $timestamp > 0 } { append genre " Watched [clock format $timestamp -format "%m/%d/%y %H:%M"]" }

    switch -- $type {
     info {
       lmbox puts "type $id"
       set fno 0
       set selected [lmbox get moviefilelist selectedtext]
       lmbox call moviefilelist clear
       foreach file $files {
         set fname "[incr fno]: [file rootname [file tail $file]]"
         # Custom chapter title
         if { [regexp {\#title:([^# ]+)} $file d title] } { set fname #$title }
         lmbox call moviefilelist additemdata $fname $file
       }
       lmbox set moviefilelist selectedindex [lmbox call moviefilelist searchitem $selected]
       set descr ""
       set hcolor "#c5cd4a"
       set tcolor [lmbox get moviedescr fontcolor]
       if { $create_time > 0 } { append descr "^${hcolor}ADDED: ^${tcolor}[lmbox_clock $create_time]\n" }
       if { $timestamp > 0 } { append descr "^${hcolor}WATCHED: ^${tcolor}[lmbox_clock $timestamp]\n" }
       foreach line [split $description "\n"] {
          set line [split $line :]
          append descr "^$hcolor[lindex $line 0]:^${tcolor}[join [lrange $line 1 end] :]\n"
       }
       lmbox set moviedescr caption "^${hcolor}TITLE:^${tcolor}$title\n$descr"
       lmbox call movieicon zoomimage [movie_icon $id] 0 0
     }

     file {
       if { [file exists $id] } {
         lmbox set status caption "TYPE: [string trim [file ext $id] .] SIZE: [lmbox_size [file size $id]]"
       }
     }

     status {
       if { [set count [lmbox get movie_list count]] == "" } { set count 0 }
       if { [set index [lmbox get movie_list selectedindex]] == "" } { set index 0 }
       if { $count > 0 } { incr index }
       lmbox set status caption "$title: $genre"
       lmbox set movie_count caption "$index of $count"
     }

     default {
       if { [set count [lmbox get movie_list count]] == "" } { set count 0 }
       if { [set index [lmbox get movie_list selectedindex]] == "" } { set index 0 }
       if { $count > 0 } { incr index }
       lmbox set status caption $genre
       lmbox set movie_count caption "$index of $count"
       lmbox call movieicon zoomimage [movie_icon $id] 0 0
     }
    }
}

# Search for movie cover picture
proc movie_icon { id { title "" } } {

    set icon ""
    if { ![regexp {^[0-9]+$} $id] && $id != "" } {
      if { [file exists $id] } {
        set icon [lmbox_icon [file root [file tail $id]] [file dirname $id]]
      } else {
        # Check movie database by file name
        set file [file tail $id]
        lmbox dbeval movies {
          if { [string match "*$file*" $rec(files)] } {
            set id $lmbox_dbkey
            break
          }
        } -array rec
      }
    }
    if { [regexp {^[0-9]+$} $id] } {
      set icon [lmbox_icon $id [lmbox gethome]/covers [lmbox config cover_path data/Movies]]
    }
    return [lmbox_nvl $icon none.gif]
}

proc movie_play { id { reset 0 } { files "" } } {

    # Special mode, movie selector
    switch -- [lmbox var movie_type] {
     Lookup {
       movie_select
       return
     }
    }
    set now [clock seconds]
    # Check for movie database
    if { [regexp {^[0-9]+$} $id] } {
      array set rec [lmbox dbget movies $id]
      set genre [lmbox_var rec(genre)]
      set adult_filter [lmbox config movie_filter Adult]
      set rc 0
      if { ![lmbox_admin_mode] } { 
        if { [lmbox_teen_mode] } {
          if { ![regexp {Cartoon|Children|Teens} $genre] } {
            set rc -1
          }
        } else {
          if { ![regexp {Cartoon|Children} $genre] } {
            set rc -1
          }
        }
      }
      # Movie filter, no access without password
      if { ![lmbox_adult_mode] && [regexp $adult_filter $genre] } {
        set rc -1
      }
      if { $rc == -1 } {
        lmbox set status caption "Access to that movie has been denied: $genre"
        return
      }
      # Play all files or just given ones only
      set files [movie_files [lmbox_nvl $files [lmbox_var rec(files)]]]
      lmbox puts "$files: [lmbox_var rec(files)]"
      set rec(timestamp) [clock seconds]
      lmbox dbput movies $id [array get rec]
      lmbox dbput system movie:$id $rec(timestamp)
      # Reset movie cache if requested
      if { $reset } {
        movie_reset $id $files
      }
    } else {
      set files $id
    }
    if { $files == "" } {
      lmbox set status caption "No movie files found for #$id"
      return
    }
    lmbox_stop
    # Run the player
    player_exec $files $id
    # Add the link temporary to favorite links
    set expires [expr [clock seconds] + [lmbox config recent_lifetime 86400]]
    movie_favorite $id expires $expires class History
}

proc movie_stop {} {

    player_stop
}

# Clears movie files position cache
proc movie_reset { id { files "" } } {

    if { $files == "" } {
      array set rec [lmbox dbget movies $id]
      set files [movie_files [lmbox_var rec(files)]]
    }
    foreach file $files {
      lmbox dbdel system position:$file
    }
}

proc movie_favorite { id args } {

    set class ""
    set expires 0
    foreach { key val } $args { set $key $val }

    if { [regexp {^[0-9]+$} $id] } {
      array set rec [lmbox dbget movies $id]
      if { ![info exists rec(title)] } { return }
      set files [movie_files $rec(files)]
    } else {
      array set rec [list title [file rootname [file tail $id]] genre Movie]
      set files $id
    }
    favorites_add Video "script:movie_play $id" $rec(title) description $rec(genre) icon [movie_icon $id] expires $expires class $class
    lmbox set status caption "$rec(title) added to favorites"
}

# Sort and returns movie files in the order they will be played
proc movie_files { files { key "" } } {

    set part ""
    set parts ""
    set pcount 0
    set mfiles ""
    set movie_path [list [lmbox gethome]/movies [lmbox config movie_path data/Movies]]
    # Save full paths
    foreach file $files {
      set fname [file tail $file]
      if { [string index $file 0] != "/" } {
        set mpaths($fname) [lmbox getcwd]/$file
      } else {
        set mpaths($fname) $file
      }
    }
    # Sort according to sequence or number
    foreach file [lsort [array names mpaths]] {
      if { (![regexp -nocase {\#pos\:([0-9]+)} $file d num] &&
            ![regexp -nocase {([0-9]+)\.[^0-9]+$} $file d num]) ||
           [string trimleft $num 0] == "" } {
        set num 0
      }
      lappend parts [list $file [lmbox_trim0 $num] [incr pcount]]
    }
    # Build final list with full paths
    foreach file [lsort -integer -index 1 $parts] {
      if { $key != "" && $key != [lindex $file 2] } { continue }
      set fname $mpaths([lindex $file 0])
      set fpath [lindex [split $fname "#"] 0]
      if { [string index $fname 0] != "/" } {
        # Go through all configured directories
        foreach path $movie_path {
          if { [file exists $path/$fname] } {
            lappend mfiles $path/[lindex $file 0]
            break
          }
        }
      } else {
        # Full path given already
        if { [file exists $fpath] } {
          lappend mfiles $fname
        }
      }
    }
    return $mfiles
}

proc movie_select {} {

    if { [set name [lmbox var movie_select]] != "" } {
      if { [set id [lmbox get movie_list selecteddata]] != "" } {
        lmbox set $name caption $id
      }
    }
    lmbox setpage [lmbox prevpage]
}

proc movie_search { { movie_type "" } { filter "" } } {

    set search ""
    if { $filter == "" } {
      set filter [lmbox var movie_filter]
    }
    foreach { name value } $filter {
      switch -glob -- $name {
       "" {}
       Keyword {
         if { $value == "" } { continue }
         lappend search "\[regexp -nocase {$value} \$title\$genre\$description\]"
       }
       Title {
         if { $value == "" } { continue }
         lappend search "\[regexp -nocase {$value} \$title\]"
       }
       Genre {
         if { $value == "" } { continue }
         lappend search "\[regexp -nocase {$value} \$genre\]"
       }
       Actor {
         if { $value == "" } { continue }
         lappend search "\[regexp -nocase {$value} \$description\]"
       }
       Description {
         if { $value == "" } { continue }
         lappend search "\[regexp -nocase {$value} \$description\]"
       }
       Updated {
         if { $value != 1 && $value != "true" } { continue }
         lappend search "(\$update_time > \$create_time)"
       }
       New {
         if { $value != 1 && $value != "true" } { continue }
         lappend search "\$timestamp <= 0"
         movie_load
       }
       New1* - New2* - New3* - New4* - New5* - New6* - New7* - New8* - New9* {
         if { $value != 1 && $value != "true" } { continue }
         lappend search "(\$create_time > 0 && \$now - \$create_time <= [string range $name 3 end]*86400)"
       }
       LastSeen {
         if { $value != 1 && $value != "true" } { continue }
         lappend search "(\$timestamp > 0 && \$now - \$timestamp <= 7*86400)"
       }
       Genres {
         foreach item $value {
           lappend search "\[regexp -nocase {$item} \$genre]"
         }
       }
       default {
         if { $value != 1 && $value != "true" } { continue }
         lappend search "\[regexp -nocase $name \$genre\]"
       }
      }
    }
    movie_setup $movie_type [join $search &&]
    # If we came from outside of movies, go back to main movie list
    set prevpage [lmbox prevpage]
    if { ![string match *movie* $prevpage] } {
      set prevpage movie_mosaicpage
    }
    lmbox setpage $prevpage
}

proc movie_genre_filter { type  { data "" } } {

    switch -- $type {
     set {
       set filter ""
       set items [lmbox get moviefilter caption]
       foreach item $items {
         if { $item == $data } {
           set data ""
         } else {
           if { $item != "None" } {
             lappend filter $item
           }
         }
       }
       if { $data != "" } { lappend filter $data }
       lmbox set moviefilter caption [lmbox_nvl $filter None]
     }

     clear {
       lmbox set movieseek text Filter
       lmbox set moviefilter caption None
     }

     filter {
       set filter ""
       set items [lmbox get moviefilter caption]
       if { $items != "" && $items != "None" } {
         foreach item $items {
           lappend filter "\[regexp -nocase {$item} \$genre]"
         }
       }
       set seek [lmbox get movieseek text]
       if { $seek != "" && $seek != "Filter" } {
         lappend filter "\[regexp -nocase {$seek} \$title\]"
       }
       return [join $filter &&]
     }
    }
}

# Create movie snapshot image
proc movie_snapshot { id } {

    lmbox_puts "Snapshot: $id"
    lmbox call player clear
    array set rec [lmbox dbget movies $id]
    set file $id
    if { [info exists rec(files)] } { set file [movie_files [lindex $rec(files) 0]] }
    lmbox call player add $file
    lmbox set player videodriver none
    lmbox set player audiodriver none
    if { [lmbox call player play] != 0 } {
      lmbox_puts "Unable to play movie $id/[lmbox get player playlist]"
      lmbox set player videodriver ""
      lmbox set player audiodriver ""
      return
    }
    for { set i 0 } { $i < 3 } { incr i } {
      set length [expr [lmbox get player length]/1000]
      set pos [lmbox random $length]
      lmbox call player seek $pos
      lmbox_puts "movie $id: taking screenshot at position $pos out of $length"
      player_snapshot $id
      if { [file exists [movie_icon $id]] } { break }
    }
    lmbox call player stop
    lmbox call player clear
    lmbox set player videodriver ""
    lmbox set player audiodriver ""
}

