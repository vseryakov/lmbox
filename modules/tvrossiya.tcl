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
#
#  www.tvrossiya.com support
#

lmbox_register_init tvrossiya_init

proc tvrossiya_init {} {

    # Reset session id
    lmbox setconfig tvrossiya_sid ""
}

proc tvrossiya_play { ch } {

    set login [lmbox config tvrossiya_login]
    set passwd [lmbox config tvrossiya_passwd]
    set http [lmbox config tvrossiya_http 209.20.1.182]
    set source [lmbox config tvrossiya_source MEDIA3]
    set timeout [lmbox config tvrossiya_timeout 3600]
    set debug [lmbox config tvrossiya_debug 0]

    if { $login == "" || $passwd == "" } {
      return
    }

    # Existing session, expire every hour
    if { [set sid [lmbox config tvrossiya_sid]] != "" } {
      if { ![regexp {([0-9]+)\:([0-9\.]+):([0-9]+)} $sid d sid mms time] || [clock seconds] - $time > $timeout } {
        set sid ""
      }
    }
    # Retrieve session id
    if { $sid == "" } {
      set data [tvrossiya_login $ch $http $login $passwd $source $debug]
      # Check session and mms host
      if { [set sid [lindex $data 0]] == "" || [set mms [lindex $data 1]] == "" } {
        lmbox_puts "tvrossiya_play: $ch: cannot get session"
        return
      }
      # Save in the config
      lmbox setconfig tvrossiya_sid $sid:$mms:[clock seconds]
      lmbox puts "tvrossiya_play: new session $sid"
    } else {
      lmbox puts "tvrossiya_play: reusing session $sid"
    }

    lmbox call player stop
    lmbox call player clear
    lmbox call player add mms://$mms/$ch?SID=$sid
    lmbox_puts "Playing $ch..."
    lmbox call player play
    lmbox setpage videopage
}

# Send http request
proc tvrossiya_send { host req { debug 0 }} {

    if { $debug } {
      lmbox puts "Request: $req\n"
    }
    
    set data ""
    
    if { [catch { 
      set sock [socket $host 80]
      fconfigure $sock -buffering none -encoding binary -translation binary

      puts $sock $req
      flush $sock

      while { ![eof $sock] } {
        append data [gets $sock]
      }
    } errmsg] } {
      lmbox puts "tvrossiya_login: $errmsg: $::errorInfo"
    }
    catch { close $sock }
    if { $debug } {
      lmbox puts "Result: [string map { \r "" } $data]"
    }
    return $data
}

# Login request
proc tvrossiya_login { ch host user passwd source { debug 0 }} {

    set form "phoneno=$user&pwd=$passwd&blnSubmitted=true&blnLoginPage=true&Submit=%26%231042%3B%26%231086%3B%26%231081%3B%26%231090%3B%26%231080%3B&POSTBACKURL=http%3A%2F%2F209.20.1.182%2Fapi%2FchoosePlayerAPI.asp"
    set hUA "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.4) Gecko/20070515 Firefox/2.0.0.4"
    set hType "Content-Type: application/x-www-form-urlencoded"
    set hLen "Content-Length: [string length $form]"

    # Send login request
    set data [tvrossiya_send $host "POST /api/loginAPI.asp HTTP/1.0\r\n$hUA\r\n$hType\r\n$hLen\r\n\r\n$form\r" $debug]
    if { ![regexp -nocase {ASPSESSIONID[^=]+=([^; ]+)} $data d cookie] ||
         ![regexp -nocase {Location: +([^ \r\n]+)} $data d location] } {
      return
    }

    # Send player request
    set data [tvrossiya_send $host "GET /api/web/player_russianonline.asp HTTP/1.0\r\n$hUA\r\nCookie: ASPSESSIONIDASDABRCS=$cookie\r\n\r" $debug]
    if { ![regexp -nocase {var S_ID = "([0-9]+)";} $data d sid] } {
      return
    }

    # Send stream request
    set url /api/mmsStreamAPI.asp?chnum=$ch&mmsSource=$source&S_ID=$sid&U_ID=$user
    set data [tvrossiya_send $host "GET $url HTTP/1.0\r\n$hUA\r\nCookie: ASPSESSIONIDASDABRCS=$cookie\r\n\r" $debug]
    if { ![regexp {mms\:\/\/([0-9\.]+)} $data d mms] } {
      return
    }
    return [list $sid $mms]
}

