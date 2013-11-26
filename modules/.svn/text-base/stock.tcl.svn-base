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

lmbox_register_init stock_init

proc stock_init {} {

    # Open stocks database
    lmbox dbopen stock
    lmbox dbopen stock_pricelog
    # Add main menu item
    lmbox_register_menu Info "Stocks" "Stock Quotes" stock.jpg { lmbox setpage stockpage } 950
    # Check for new quotes, they are 5 mins delayed
    lmbox schedule "stock_schedule 1" -interval 300
}

# Format: sl1d1t1c1ohgv
# Reference: http://www.gummy-stuff.org/Yahoo-data.htm
# s  - Symbol
# l1 - Last Trade (Price Only)
# d1 - Last Trade Date
# t1 - Last Trade Time
# c1 - Change
# o  - Open 
# h  - Day's High
# g  - Day's Low
# v  - Volume
# j1 - Market Cap
# n  - Name
#
proc stock_schedule { { cleanup 0 } } {

    set symbols [string toupper [lmbox config stock_symbols]]
    if { $symbols == "" } { return }
    set query "http://quote.yahoo.com/d/quotes.csv?s=[join $symbols +]&f=sl1d1t1c1ohgvj1n&e=.csv"
    set data [lmbox_geturl $query]
    regsub -all {"} $data {} data; # remove " quotes
    foreach rec [split $data "\r\n"] {
      if { $rec == "" } { continue }
      set rec [split $rec ,]
      set symbol [lindex $rec 0]
      set quote(price) \$[lindex $rec 1]
      set quote(date) "[lindex $rec 2] [lindex $rec 3]"
      set quote(change) [lindex $rec 4]
      set quote(open) \$[lindex $rec 5]
      set quote(high) \$[lindex $rec 6]
      set quote(low) \$[lindex $rec 7]
      set quote(volume) [lindex $rec 8]
      set quote(marketcap) [lindex $rec 9]
      set quote(name) [lindex $rec 10]
      if { $symbol == "" || $quote(volume) == "" } { continue }
      lmbox dbput stock $symbol [array get quote]
      lmbox dbput stock_pricelog $symbol:[clock scan $quote(date)] [lindex $rec 1]
    }
    # Clean up history, keep only last 3 years
    if { $cleanup } {
      set dlist ""
      set now [clock seconds]
      set history [lmbox config stock_history 94608000]
      lmbox dbeval stock_pricelog {
        if { $now - [lindex [split $lmbox_dbkey :] 1] > $history } {
          lappend dlist $lmbox_dbkey
        }
      }
      foreach key $dlist {
        lmbox dbdel stock_pricelog $key
      }
    }
}

proc stock_setup {} {

    lmbox call stocklist clear
    lmbox dbeval stock {
      lmbox call stocklist additemdata "$lmbox_dbkey: $price" $lmbox_dbkey
    } -split
}

proc stock_info { id { scale 0 } } {

    array set rec [lmbox dbget stock $id]
    if { ![info exists rec(price)] } { return }
    set info ""
    foreach { key val } [array get rec] {
      append info "^#61BF5F[string totitle $key]: ^#c5cd4a$val\n"
    }
    # History scale: 3 years = 0, 2 years = 1, 1 year = 2, 6 months = 3, 1 month = 4, 1 day = 5
    set history [lmbox var stock_history 0]
    if { $scale } {
      incr history $scale
      if { $history > 5 } { set history 5 }
      if { $history < 0 } { set history 0 }
      lmbox varset stock_history $history
      switch $history {
       1 { set history [expr 86400*365*2] }
       2 { set history [expr 86400*365] }
       3 { set history [expr 86400*31*6] }
       4 { set history [expr 86400*31] }
       5 { set history [expr 86400] }
      }
    }
    lmbox set stockinfo caption $info
    set prices ""
    set prices2 ""
    set wmin 99999999999
    set wmax 0
    set wscale 0
    set hmin 99999999999
    set hmax 0
    set hscale 0
    set now [clock seconds]
    set len [string length $id:]
    set width [lmbox get stockchart width]
    set height [lmbox get stockchart height]
    lmbox dbeval stock_pricelog {
      set date [string range $lmbox_dbkey $len end]
      if { $lmbox_dbdata != "" && $history == 0 || $now - $date < $history } {
        set price [expr round($lmbox_dbdata*100)]
        if { $date > $wmax } { set wmax $date }
        if { $date < $wmin } { set wmin $date }
        if { $price > $hmax } { set hmax $price }
        if { $price < $hmin } { set hmin $price }
        lappend prices $date $price
      }
    } -start $id: -beginwith $id:
    lmbox call stockchart reset
    lmbox call stockchart fill #FFFFFF
    set wscale [expr ($wmax - $wmin)/($width - 4)]
    set hscale [expr $hmax/($height - 2)]
    if { $wscale > 0 && $hscale > 0 } {
      foreach { date price } $prices {
        if { $price != "" } {
          lappend prices2 [expr ($date - $wmin)/$wscale + 1] [expr $height - ($price/$hscale)/2]
        }
      }
      lmbox call stockchart polygon $prices2 #0000FF
      lmbox call stockchart text 3 3 "[clock format [lindex $prices 0] -format "%m/%y"] - [clock format [lindex $prices end-1] -format "%m/%y"]" #0000FF
      lmbox call stockchart text 2 [expr [lindex $prices2 1]-9] [format "%.2f" [expr [lindex $prices 1].0/100.0]] #000000
      lmbox call stockchart text [expr $width-[string length [lindex $prices end]]*10] [expr [lindex $prices2 end]-9] [format "%.2f" [expr [lindex $prices end].0/100.0]] #000000
    }
    lmbox set status caption "$id"
}

# Retrieve all symbol history
proc stock_history { id } {

    set now [clock seconds]
    set history [lmbox config stock_history 94608000]
    foreach id [string toupper $id] {
      # Repeat several times because Yahoo sometimes return empty history
      for { set i 0 } { $i < 3 } { incr i } {
        set data [lmbox_geturl "http://ichart.finance.yahoo.com/table.csv?s=$id&e=.csv&g=d"]
        set data [split $data "\n"]
        if { [llength $data] > 2 } { break }
      }
      foreach line $data {
        set line [split $line ,]
        set date [lindex $line 0]
        set price [lindex $line end]
        if { [catch { set date [clock scan $date] } ] } { continue }
        if { $now - $date < $history && $price != "" } {
          lmbox dbput stock_pricelog $id:$date $price
        }
      }
    }
}

proc stock_add { id } {

    if { $id == "" || $id == "Symbol" } { return }
    set price [lmbox_geturl http://quote.yahoo.com/d/quotes.csv?s=$id&f=l1&e=.csv]
    if { [string is double -strict $price] && $price > 0 } {
      lmbox_setconfig stock_symbols [lsort -unique "[string toupper $id] [lmbox config stock_symbols]"]
      stock_history $id
      stock_schedule
      stock_setup
      return lmbox_ignore
    } else {
      lmbox set status caption "Symbol $id not found"
      lmbox set stocksymbol text ""
      lmbox call symboltext setfocus
    }
}

proc stock_delete { id } {

    set symbols ""
    foreach sym [lmbox config stock_symbols] {
      if { $sym != $id } { lappend symbols $sym }
    }
    lmbox_setconfig stock_symbols $symbols
    lmbox dbdel stock $id
    lmbox_dbdel stock_pricelog $id:
    stock_setup
}

