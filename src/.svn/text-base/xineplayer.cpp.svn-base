/*
 *
 * Author Vlad Seryakov vlad@crystalballinc.com
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://mozilla.org/.
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 */

#include "lmbox.h"

#ifdef HAVE_XINE

class VideoFilter {
public:
  char *name;
  vector <ObjectProperty *> params;
};

static char *deinterlace_filters[] = {
  "Greedy Poor",
  "tvtime:method=Greedy2Frame,enabled=1,pulldown=vektor,framerate_mode=full,judder_correction=1,use_progressive_frame_flag=1,chroma_filter=1,cheap_mode=0",
  "Very low cpu usage, worst quality. Half of vertical resolution is lost. For some systems (with PCI video cards) this might decrease the cpu usage when compared to plain video playback (no deinterlacing).",
  "Greedy Good",
  "tvtime:method=Greedy2Frame,enabled=1,pulldown=vektor,framerate_mode=full,judder_correction=0,use_progressive_frame_flag=1,chroma_filter=0,cheap_mode=0",
  "Low cpu usage, poor quality. Image is blurred vertically so interlacing effects are removed.",
  "Greedy Medium",
  "tvtime:method=Greedy,enabled=1,pulldown=none,framerate_mode=half_top,judder_correction=0,use_progressive_frame_flag=1,chroma_filter=0,cheap_mode=0",
  "Medium cpu usage, medium quality. Image is analysed and areas showing interlacing artifacts are fixed (interpolated).",
  "Greedy Best",
  "tvtime:method=Greedy,enabled=1,pulldown=none,framerate_mode=half_top,judder_correction=0,use_progressive_frame_flag=1,chroma_filter=0,cheap_mode=1",
  "High cpu usage, good quality. Conversion of dvd image format improves quality and fixes chroma upsampling bug.",
  "Liner Blend",
  "tvtime:method=LinearBlend,enabled=1,pulldown=none,framerate_mode=half_top,judder_correction=0,use_progressive_frame_flag=1,chroma_filter=0,cheap_mode=1",
  "Very high cpu usage, great quality. Besides using smart deinterlacing algorithms it will also double the frame rate (30->60fps) to match the field rate of TVs. Detects and reverts 3-2 pulldown.",
  "Line Doubler",
  "tvtime:method=LineDoubler,enabled=1,pulldown=none,framerate_mode=half_top,judder_correction=0,use_progressive_frame_flag=1,chroma_filter=0,cheap_mode=1",
  "Very very high cpu usage, great quality with (experimental) improvements. Enables judder correction (play films at their original 24 fps speed) and vertical color smoothing (fixes small color stripes seen in some dvds).",
  0, 0, 0
};

static void VideoDestSize(void *data, int video_width, int video_height, double video_pixel_aspect,
                          int *dest_width, int *dest_height, double *dest_pixel_aspect)
{
    XinePlayerObject *player = (XinePlayerObject *) data;
    SDL_Rect area = player->GetAreaRaw();
    *dest_pixel_aspect = player->GetAspect();
    *dest_width = area.w;
    *dest_height = area.h;
}

static void VideoFrameOutput(void *data, int video_width, int video_height,
                             double video_pixel_aspect, int *dest_x,
                             int *dest_y, int *dest_width, int *dest_height, double *dest_pixel_aspect,
                             int *win_x, int *win_y)
{
    XinePlayerObject *player = (XinePlayerObject *) data;
    SDL_Rect area = player->GetAreaRaw();
    *dest_pixel_aspect = player->GetAspect();
    *win_x = area.x;
    *win_y = area.y;
    *dest_x = *dest_y = 0;
    *dest_width = area.w;
    *dest_height = area.h;
}

static void PlayerEvents(void *data, const xine_event_t * e)
{
    switch (e->type) {
     case XINE_EVENT_QUIT:
         return;

     case XINE_EVENT_PVR_REPORT_NAME: {
         xine_pvr_save_data_t *data  = (xine_pvr_save_data_t*)e->data;
         systemObject->Log(0, "xine: PVR: saved in %d: %s", data->id, data->name);
         return;
     }

#ifdef XINE_EVENT_AUDIO_AMP_LEVEL
     case XINE_EVENT_AUDIO_AMP_LEVEL:
#endif
     case XINE_EVENT_AUDIO_LEVEL: {

         xine_audio_level_data_t *data = (xine_audio_level_data_t *) e->data;
         systemObject->Log(0, "xine: Audio: Left=%d Right=%d Mute=%d", data->left, data->right, data->mute);
         return;
     }

     case XINE_EVENT_PROGRESS: {
         xine_progress_data_t *data = (xine_progress_data_t *) e->data;
         char *str = tlsSprintf(1, "%s %d%%", data->description, data->percent);
         systemObject->PostEvent("progress",str, NULL);
         return;
     }

     case XINE_EVENT_UI_MESSAGE: {
         xine_ui_message_data_t *data = (xine_ui_message_data_t *) e->data;
         switch(data->type) {
          case XINE_MSG_NO_ERROR:
          case XINE_MSG_GENERAL_WARNING:
          case XINE_MSG_UNKNOWN_HOST:
          case XINE_MSG_UNKNOWN_DEVICE:
          case XINE_MSG_NETWORK_UNREACHABLE:
          case XINE_MSG_CONNECTION_REFUSED:
          case XINE_MSG_FILE_NOT_FOUND:
          case XINE_MSG_READ_ERROR:
          case XINE_MSG_LIBRARY_LOAD_ERROR:
          case XINE_MSG_ENCRYPTED_SOURCE:
          case XINE_MSG_SECURITY:
          case XINE_MSG_AUDIO_OUT_UNAVAILABLE:
          default:
              if (data->explanation) {
                  systemObject->Log(0, "xine: msg: %d: %s", data->type, (char*)data + data->explanation);
              }
         }
         return;
     }
    }
    char *ptr;
    XinePlayerObject *player = (XinePlayerObject *) data;
    // Player can be busy if close has been called
    if (player->IsBusy()) {
        systemObject->Log(1, "xine: busy: %s: %d", player->GetName(), e->type);
        return;
    }
    switch (e->type) {
    case XINE_EVENT_UI_SET_TITLE: {
        xine_ui_data_t *ui = (xine_ui_data_t *) e->data;
        player->SetPlaylistTitle(ui->str);
        break;
    }

    case XINE_EVENT_UI_CHANNELS_CHANGED:
        player->SetPlaylistTitle(xine_get_meta_info(player->stream, XINE_META_INFO_TITLE));
        player->SetPlaylistArtist(xine_get_meta_info(player->stream, XINE_META_INFO_ARTIST));
        player->SetPlaylistAlbum(xine_get_meta_info(player->stream, XINE_META_INFO_ALBUM));
        player->SetPlaylistGenre(xine_get_meta_info(player->stream, XINE_META_INFO_GENRE));
        player->SetPlaylistComment(xine_get_meta_info(player->stream, XINE_META_INFO_COMMENT));
        break;

    case XINE_EVENT_UI_PLAYBACK_FINISHED:
        if (player->GetPlaylistSize() > 0) {
            if (player->GetPlaylistIndex() < player->GetPlaylistSize() - 1 || player->playlist_repeat) {
                if (!player->PlaylistNext()) {
                    break;
                }
            }
        }
        systemObject->PostEvent("fire", player->GetName(), "OnFinish");
        break;

    case XINE_EVENT_MRL_REFERENCE:
        break;

    case XINE_EVENT_MRL_REFERENCE_EXT: {
        xine_mrl_reference_data_ext_t *data  = (xine_mrl_reference_data_ext_t*)e->data;
        systemObject->Log(0, "xine: MRL: %s: %s: %d/%d", player->GetName(), data->mrl, data->start_time, data->duration);
        if (!player->TryLock()) {
            player->FileAdd(data->mrl, 0, data->start_time, data->duration, NULL);
            player->Unlock();
        }
        break;
    }

    case XINE_EVENT_FRAME_FORMAT_CHANGE: {
        xine_format_change_data_t *data = (xine_format_change_data_t*)e->data;
        systemObject->Log(0, "xine: %s: frame format changed: %dx%d, aspect=%d",
                          player->GetName(), data->width, data->height, data->aspect);
        break;
    }

    case XINE_EVENT_DROPPED_FRAMES:
        systemObject->Log(0, "xine: %s: too many dropped frames", player->GetName());
        break;

    default:
        systemObject->Log(0, "xine: event: %s: %d", player->GetName(), e->type);
        break;
    }
}

XinePlayerObject::XinePlayerObject(const char *aname):PlayerObject(aname)
{
    vo = 0;
    ao = 0;
    xine = 0;
    stream = 0;
    stream_info = 0;
    ao_info = 0;
    vo_info = 0;
    osd = 0;
    audio_filter = 0;
    audio_post = 0;

    string config = xine_get_homedir();
    config += "/.xine/config";
    xine = xine_new();
    xine_config_load(xine, config.c_str());
    xine_init(xine);
    systemObject->RegisterExternalEventHandler(this);
}

XinePlayerObject::~XinePlayerObject(void)
{
    if (xine) {
        xine_exit(xine);
    }
}

int XinePlayerObject::InfoInit(void)
{
    if (TryLock()) {
        return -1;
    }
    if (!vo_info && !(vo_info = xine_open_video_driver(xine, "auto", XINE_VISUAL_TYPE_NONE, 0))) {
        systemObject->Log(0, "InfoInit: unable to open video info driver");
        Unlock();
        return -1;
    }
    if (!ao_info && !(ao_info = xine_open_audio_driver(xine, "auto", 0))) {
        systemObject->Log(0, "InfoInit: unable to open audio driver");
        Unlock();
        return -1;
    }
    if (!stream_info && !(stream_info = xine_stream_new(xine, ao_info, vo_info))) {
        systemObject->Log(0, "InfoInit: unable to open a Xine stream");
        Unlock();
        return -1;
    }
    Unlock();
    return 0;
}

void XinePlayerObject::InfoDestroy(void)
{
    if (TryLock()) {
        return;
    }
    if (stream_info) {
        xine_dispose(stream_info);
        stream_info = 0;
    }
    if (ao_info) {
        xine_close_audio_driver(xine, ao_info);
        ao_info = 0;
    }
    if (vo_info) {
        xine_close_video_driver(xine, vo_info);
        vo_info = 0;
    }
    Unlock();
}

int XinePlayerObject::PlayerInit(void)
{
    if (stream) {
        return 0;
    }
    if (PlayerObject::PlayerInit()) {
        return -1;
    }
    xine_engine_set_param(xine, XINE_ENGINE_PARAM_VERBOSITY, systemObject->configGetInt("player_debug", 0));
    // Allocate video/audio drivers
    if (!vo) {
        x11_visual_t x11;
        x11.d = window;
        x11.display = display;
        x11.screen = DefaultScreen(display);
        x11.user_data = this;
        x11.dest_size_cb = VideoDestSize;
        x11.frame_output_cb = VideoFrameOutput;
        const char *vdriver = video_driver ? video_driver : systemObject->configGetValue("video_driver", "auto");
        const char *adriver = audio_driver ? audio_driver : systemObject->configGetValue("audio_driver", "auto");
        void *vptr = &x11;
        visual_type = XINE_VISUAL_TYPE_X11;
        if (!strcmp(vdriver, "none")) {
            vptr = NULL;
            visual_type = XINE_VISUAL_TYPE_NONE;
        } else
        if (!strcmp(vdriver, "fb")) {
            vptr = NULL;
            visual_type = XINE_VISUAL_TYPE_FB;
        }
        if (!(vo = xine_open_video_driver(xine, vdriver, visual_type, vptr))) {
            systemObject->Log(0, "PlayerInit: unable to open video driver: %s", vdriver);
	    return -1;
	}
        if (!(ao = xine_open_audio_driver(xine, adriver, 0))) {
	    systemObject->Log(0, "PlayerInit: unable to open audio driver: %s", adriver);
            xine_close_video_driver(xine, vo);
            vo = 0;
            return -1;
        }
    }
    // Allocate multimedia stream
    if (!stream) {
        if (!(stream = xine_stream_new(xine, ao, vo))) {
            systemObject->Log(0, "PlayerInit: unable to create video stream ");
            return -1;
        }
        events = xine_event_new_queue(stream);
        xine_event_create_listener_thread(events, PlayerEvents, this);
    }
    OsdInit();
    VideoWireFilter();
    AudioWireFilter();
    return 0;
}

void XinePlayerObject::PlayerExpose(void)
{
    if (stream && has_video) {
        XExposeEvent ev;
        ev.type = Expose;
        ev.send_event = False;
        ev.display = display;
        ev.window = window;
        ev.x = area.x;
        ev.y = area.y;
        ev.width = area.w;
        ev.height = area.h;
        ev.count = 0;
        xine_port_send_gui_data(vo, XINE_GUI_SEND_EXPOSE_EVENT, &ev);
    }
}

char *XinePlayerObject::PlayerEvent(const char *action)
{
    if (TryLock()) {
        return "";
    }
    if (stream) {
        xine_event_t event;
        memset(&event, 0, sizeof(event));
        systemObject->Log(0, "player_event: %s", action);
        if (!strcmp(action, "accept"))
            event.type = XINE_EVENT_INPUT_SELECT;
        else
        if (!strcmp(action, "up"))
            event.type = XINE_EVENT_INPUT_UP;
        else
        if (!strcmp(action, "down"))
            event.type = XINE_EVENT_INPUT_DOWN;
        else
        if (!strcmp(action, "prev"))
            event.type = XINE_EVENT_INPUT_LEFT;
        else
        if (!strcmp(action, "nexttrack"))
            event.type = XINE_EVENT_INPUT_NEXT;
        else
        if (!strcmp(action, "prevtrack"))
            event.type = XINE_EVENT_INPUT_PREVIOUS;
        else
        if (!strcmp(action, "next"))
            event.type = XINE_EVENT_INPUT_RIGHT;
        else
        if (!strcmp(action, "menu"))
            event.type = XINE_EVENT_INPUT_MENU1;
        else
        if (!strcmp(action, "menu1"))
            event.type = XINE_EVENT_INPUT_MENU1;
        else
        if (!strcmp(action, "menu2"))
            event.type = XINE_EVENT_INPUT_MENU2;
        else
        if (!strcmp(action, "menu3"))
            event.type = XINE_EVENT_INPUT_MENU3;
        else
        if (!strcmp(action, "menu4"))
            event.type = XINE_EVENT_INPUT_MENU4;
        else
        if (!strcmp(action, "menu5"))
            event.type = XINE_EVENT_INPUT_MENU5;
        else
        if (!strcmp(action, "menu6"))
            event.type = XINE_EVENT_INPUT_MENU6;
        else
        if (!strcmp(action, "menu7"))
            event.type = XINE_EVENT_INPUT_MENU7;
        else
        if (!strcmp(action, "number0"))
            event.type = XINE_EVENT_INPUT_NUMBER_0;
        else
        if (!strcmp(action, "number1"))
            event.type = XINE_EVENT_INPUT_NUMBER_1;
        else
        if (!strcmp(action, "number2"))
            event.type = XINE_EVENT_INPUT_NUMBER_2;
        else
        if (!strcmp(action, "number3"))
            event.type = XINE_EVENT_INPUT_NUMBER_3;
        else
        if (!strcmp(action, "number4"))
            event.type = XINE_EVENT_INPUT_NUMBER_4;
        else
        if (!strcmp(action, "number5"))
            event.type = XINE_EVENT_INPUT_NUMBER_5;
        else
        if (!strcmp(action, "number6"))
            event.type = XINE_EVENT_INPUT_NUMBER_6;
        else
        if (!strcmp(action, "number7"))
            event.type = XINE_EVENT_INPUT_NUMBER_7;
        else
        if (!strcmp(action, "number8"))
            event.type = XINE_EVENT_INPUT_NUMBER_8;
        else
        if (!strcmp(action, "number9"))
            event.type = XINE_EVENT_INPUT_NUMBER_9;
        else
        if (!strncmp(action, "v4l:", 4)) {
            // format: v4l:input frequency channel ?session?
            // Example: v4l:0 6534 60
            xine_set_v4l2_data_t *v4l = (xine_set_v4l2_data_t *) calloc(1, sizeof(xine_set_v4l2_data_t));
            v4l->input = atoi(&action[4]);
            if (!(action = strchr(action, ' '))) {
                free(v4l);
                action = "";
                goto exit;
            }
            v4l->frequency = atoi(++action);
            if (!(action = strchr(action, ' '))) {
                free(v4l);
                action = "";
                goto exit;
            }
            v4l->channel = atoi(++action);
            // Session is optional
            if ((action = strchr(action, ' '))) {
                v4l->session_id = atoi(++action);
            }
            event.data = (xine_set_v4l2_data_t *) v4l;
            event.type = XINE_EVENT_SET_V4L2;
            event.data_length = sizeof(xine_set_v4l2_data_t);
        } else
        if (!strncmp(action, "pvr:", 4)) {
            // format: pvr:mode ?filename?
            // mode is: -1 nothing, 0 - save from now on, 1 - save from last sync point, 2 - save everything
            // Example: pvr:0 MyShow
            xine_pvr_save_data_t *pvr = (xine_pvr_save_data_t *) calloc(1, sizeof(xine_pvr_save_data_t));
            pvr->mode = atoi(&action[4]);
            if ((action = strchr(action, ' '))) {
                strncpy(pvr->name,++action, sizeof(pvr->name) - 1);
            }
            event.data = (xine_pvr_save_data_t *) pvr;
            event.type = XINE_EVENT_PVR_SAVE;
            event.data_length = sizeof(xine_pvr_save_data_t);
        }
        if (event.type != 0) {
            event.stream = stream;
            gettimeofday(&event.tv, 0);
            xine_event_send(stream, &event);
            action = "";
        }
    }
exit:
    Unlock();
    return tlsStr(0, action);
}

int XinePlayerObject::IsPlaying(void)
{
    return stream && xine_get_status(stream) == XINE_STATUS_PLAY;
}

int XinePlayerObject::Play(void)
{
    if (!stream) {
        return -1;
    }
    if (!playlist_files.size()) {
        return -1;
    }
    // Randomize play list
    if (playlist_shuffle) {
        for (int i = 0; i < playlist_files.size(); i++) {
            int j = (int) ((playlist_files.size() - 1) * (rand() / (RAND_MAX + 1.0)));
            if (i == j) {
                continue;
            }
            FileEntry *file = playlist_files[j];
            playlist_files[j] = playlist_files[i];
            playlist_files[i] = file;
        }
    }
    if (xine_get_status(stream) == XINE_STATUS_PLAY) {
        xine_close(stream);
    }
    // Try all in the playlist until success
    while (playlist_index < playlist_files.size()) {
       if (xine_open(stream, playlist_files[playlist_index]->name())) {
           break;
       }
       systemObject->Log(0, "Play: %s: error open: %d/%d: %s", GetName(), xine_get_error(stream), playlist_index, playlist_files[playlist_index]->name());
       playlist_index++;
    }
    if (playlist_index >= playlist_files.size()) {
        systemObject->Log(0, "Play: %s: no valid streams found", GetName());
        Stop();
        return -1;
    }
    // Set presets, especially volume related
    if (volume) {
        SetVolume(volume);
    }
    if (amp_level) {
        SetAmpLevel(amp_level);
    }
    xine_set_param(stream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL, audio_channel);
    xine_set_param(stream, XINE_PARAM_SPU_CHANNEL, spu_channel);
    // Start position can be specified by the playlist item as well
    if (!xine_play(stream, 0, pos_time ? pos_time : playlist_files[playlist_index]->start_time)) {
        systemObject->Log(0, "Play: %s: error playing: %d/%d: %s", GetName(), xine_get_error(stream), playlist_index, playlist_files[playlist_index]->name());
        Stop();
        return -1;
    }
    systemObject->Log(0, "Play: %s: playing %s", GetName(), playlist_files[playlist_index]->name());
    // Save original dimensions and stream capabilities
    vo_width = xine_get_stream_info (stream, XINE_STREAM_INFO_VIDEO_WIDTH);
    vo_height = xine_get_stream_info (stream, XINE_STREAM_INFO_VIDEO_HEIGHT);
    if (visual_type != XINE_VISUAL_TYPE_NONE) {
        has_video = xine_get_stream_info (stream, XINE_STREAM_INFO_HAS_VIDEO);
    }
    has_audio = xine_get_stream_info (stream, XINE_STREAM_INFO_HAS_AUDIO);
    if (audio_post) {
        has_video = true;
    }
    // If this item has duration, register handler that will stop the stream
    if (playlist_files[playlist_index]->duration > 0) {
        end_time = time(0) + playlist_files[playlist_index]->duration/1000;
    }
    // Set skip flag to indicate that skipping is enabled for this file
    skip_flag = playlist_files[playlist_index]->skip_list.size();

    //xine_port_send_gui_data(vo, XINE_GUI_SEND_DRAWABLE_CHANGED, (void *) window);
    //xine_port_send_gui_data(vo, XINE_GUI_SEND_VIDEOWIN_VISIBLE, (void *) 1);
    // Now we are ready to show raise the window
    return PlayerObject::Play();
}

void XinePlayerObject::Pause(void)
{
    PlayerObject::Pause();
    if (stream && xine_get_status(stream) == XINE_STATUS_PLAY) {
        switch (xine_get_param(stream, XINE_PARAM_SPEED)) {
        case XINE_SPEED_PAUSE:
            xine_set_param(stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
            break;
        default:
            xine_set_param(stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
        }
    }
}

void XinePlayerObject::Stop(void)
{
    if (!stream) {
        return;
    }
    PlayerObject::Stop();
    SetFlags(OBJECT_BUSY);
    xine_close(stream);
    Destroy();
    ClearFlags(OBJECT_BUSY);
}

void XinePlayerObject::Destroy(void)
{
    AudioUnwireFilter();
    VideoUnwireFilter();
    if (osd) {
        xine_osd_free(osd);
        osd = 0;
    }
    if (stream) {
        xine_event_dispose_queue(events);
        xine_dispose(stream);
        stream = 0;
    }
    if (ao) {
        xine_close_audio_driver(xine, ao);
        ao = 0;
    }
    if (vo) {
        xine_close_video_driver(xine, vo);
        vo = 0;
    }
    PlayerObject::Destroy();
}

void XinePlayerObject::OsdInit(void)
{
    if (stream && !osd) {
        osd = xine_osd_new(stream, 0, 0, area.w, area.h);
        xine_osd_set_font(osd, "sans", 20);
        xine_osd_set_text_palette(osd, XINE_TEXTPALETTE_WHITE_BLACK_TRANSPARENT, XINE_OSD_TEXT1);
    }
}

void XinePlayerObject::OsdFree(void)
{
    if (osd) {
        xine_osd_free(osd);
        osd = 0;
    }
}

int XinePlayerObject::OsdSet(const char *cmd)
{
    if (TryLock()) {
        return -1;
    }
    if (stream && osd) {
        if (!strcmp(cmd, "show")) {
            xine_osd_show_unscaled(osd, 0);
            osd_flag = 1;
        } else
        if (!strcmp(cmd, "hide")) {
            xine_osd_hide(osd, 0);
            osd_flag = 0;
        } else
        if (!strcmp(cmd, "clear")) {
            xine_osd_clear(osd);
        }
    }
    int rc = osd_flag;
    Unlock();
    return rc;
}

void XinePlayerObject::OsdText(int x, int y, const char *text)
{
    char *s;
    int ow, oh;

    if (TryLock()) {
        return;
    }
    if (stream && osd && text) {
        xine_osd_get_text_size(osd, text, &ow, &oh);
        while (text && *text) {
            if ((s = strchr(text, '\n'))) {
                *s++ = 0;
            }
            xine_osd_draw_text(osd, x, y, text, XINE_OSD_TEXT1);
            y += oh;
            text = s;
        }
    }
    Unlock();
}

void XinePlayerObject::OsdSlider(int val, int max, const char *str)
{
    if (TryLock()) {
        return;
    }
    if (stream && osd && str) {
        char buf[32];
        int ow = 200, oh = 20, tw, th;
        int ox = (area.w - ow) / 2, oy = (int) (area.h / 2);

        int pos = val * 100 / (max > 0 ? max : 1);
        if (pos < 0) {
            pos = 0;
        }
        if (pos > 100) {
            pos = max;
        }
        sprintf(buf, "%d%%", pos);
        string text = str;
        text += buf;

        xine_osd_clear(osd);
        xine_osd_get_text_size(osd, text.c_str(), &tw, &th);
        xine_osd_draw_text(osd, (area.w - tw) / 2, oy - th - 2, text.c_str(), XINE_OSD_TEXT1);
        xine_osd_draw_rect(osd, ox, oy, ox + ow, oy + oh, XINE_OSD_TEXT1 + 7, 1);
        xine_osd_draw_rect(osd, ox + 3, oy + 3, ox + ow - 3, oy + oh - 3, XINE_OSD_TEXT1 + 6, 1);
        xine_osd_draw_rect(osd, ox + 3, oy + 3, ox + (pos <= 6 ? 6 : pos * ow / 100) - 3, oy + oh - 3, XINE_OSD_TEXT1 + 9, 1);
    }
    Unlock();
}

void XinePlayerObject::OsdInfo(const char *text)
{
    int ow, oh, ox = 30, oy = 30;
    char *sptr, *eptr, buffer[512];
    char *info = GetStreamInfo(stream);

    if (TryLock()) {
        return;
    }
    if (stream && osd) {
        struct tm ltm;
        time_t now = time(0);
        localtime_r(&now, &ltm);
        strftime(buffer, sizeof(buffer), "%a %b %d %Y %T", &ltm);
        xine_osd_get_text_size(osd, buffer, &ow, &oh);
        xine_osd_draw_text(osd, ox, oy, buffer, XINE_OSD_TEXT1);
        oy += oh;

        for (sptr = (char*)text; sptr && *sptr; sptr = eptr) {
          eptr = strchr(sptr,'\n');
          if (eptr) {
              *eptr++ = 0;
          }
          if (*sptr) {
              xine_osd_draw_text(osd, ox, oy, sptr, XINE_OSD_TEXT1);
              oy += oh;
          }
        }
        for (sptr = info; sptr && *sptr; sptr = eptr) {
          eptr = strchr(sptr,'\n');
          if (eptr) {
              *eptr++ = 0;
          }
          if (*sptr) {
              xine_osd_draw_text(osd, ox, oy, sptr, XINE_OSD_TEXT1);
              oy += oh;
          }
        }
    }
    Unlock();
}

int XinePlayerObject::GetTimePosition(int *len)
{
    int pos = -1, length = 0, cnt = 0;

    if (stream && xine_get_status(stream) == XINE_STATUS_PLAY &&
        xine_get_stream_info(stream, XINE_STREAM_INFO_SEEKABLE)) {
        while (!xine_get_pos_length(stream, 0, &pos, &length) && ++cnt < 10) {
            SDL_Delay(50);
        }
        xine_set_param(stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
        if (cnt >= 10) {
            pos = 0;
        }
    }
    if (len) {
        *len = length;
    }
    return pos;
}

void XinePlayerObject::OnEvent(void)
{
    if (!stream || (!end_time && !skip_flag) || TryLock()) {
        return;
    }
    // End stream event
    if (end_time > 0 && stream && playlist_index < playlist_files.size() && playlist_files[playlist_index]->duration > 0) {
        int pos_time = GetTimePosition(NULL);
        if ((pos_time > 0 && pos_time > playlist_files[playlist_index]->duration) || time(0) >= end_time) {
            systemObject->Log(0, "OnEvent: %s: stopping after %d ms, %d/%lu", GetName(), playlist_files[playlist_index]->duration, pos_time, end_time);
            xine_set_param(stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
            xine_event_t event = { XINE_EVENT_UI_PLAYBACK_FINISHED, stream, NULL };
            xine_event_send(stream, &event);
        }
    }
    // Skip scene event
    if (skip_flag > 0 && stream && playlist_index < playlist_files.size() > 0 && playlist_files[playlist_index]->skip_list.size() > 0) {
	int pos_time = GetTimePosition(NULL)/1000;
        // Scan all skip intervals for the match
        for (int i = 0; i < playlist_files[playlist_index]->skip_list.size(); i++) {
	    PositionEntry pos = playlist_files[playlist_index]->skip_list[i];
            if (pos_time >= pos.start_time && pos_time < pos.start_time+pos.duration) {
                // We are still within the skipping interval, do nothing
                if (skip_flag == pos.start_time) {
                    break;
                }
                pos_time += pos.duration - (pos_time - pos.start_time);
                xine_play(stream, 0, pos_time*1000);
                systemObject->Log(0, "OnEvent: %s: skipping from %d for %d secs, actual=%d", GetName(), pos.start_time, pos.duration, pos_time - pos.start_time);
                // Mark the interval we are currently in
                skip_flag = pos.start_time;
                break;
	    }
	}
    }
    Unlock();
}

int XinePlayerObject::Seek(int offset, bool absolute)
{
    int pos = 0, length = 0;

    if (time(0) - seek_time <= 0) {
        return -1;
    }
    if ((pos = GetTimePosition(&length)) < 0) {
        return -1;
    }
    pos_time = (absolute ? 0 : pos);
    pos_time /= 1000;
    pos_time += offset;
    // Check all skip intervals, set the end of the matched skip interval
    if (playlist_index < playlist_files.size() > 0 && playlist_files[playlist_index]->skip_list.size() > 0) {
        for (int i = 0; i < playlist_files[playlist_index]->skip_list.size(); i++) {
	     PositionEntry pos = playlist_files[playlist_index]->skip_list[i];
	     if (pos_time >= pos.start_time && pos_time < pos.start_time+pos.duration) {
	        pos_time = pos.start_time + pos.duration;
                systemObject->Log(0, "Seek: %s: moving to the end of skip interval %d of %d secs", GetName(), pos.start_time, pos.duration);
		break;
	    }
	}
        // Set skip flag to reset current skipping interval
        skip_flag = playlist_files[playlist_index]->skip_list.size();
    }
    pos_time *= 1000;
    if (pos_time > length - 2000) {
        pos_time = length - 2000;
    }
    if (pos_time < 0) {
        pos_time = 0;
    }
    if (stream) {
        if (systemObject->configGetInt("video_seek_dvd", 0)) {
            // Emulate DVD fast forwarding by quickly showing scenes between the seeking points
            int delta = (pos_time - pos)/3;
            xine_set_param(stream, XINE_PARAM_SPEED, XINE_SPEED_FAST_2);
            while (offset > 0 ? pos < pos_time : pos > pos_time) {
                xine_play(stream, 0, pos);
                pos += delta;
            }
            xine_set_param(stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
        } else {
            // Seek directly to destination
            xine_play(stream, 0, pos_time);
        }
    }
    seek_time = time(0);
    return 0;
}

char *XinePlayerObject::FileInfo(const char *mrl)
{
    char *info = tlsStr(0, "");
    if (!InfoInit()) {
        if (mrl) {
            if (!xine_open(stream_info, mrl)) {
                systemObject->Log(0, "FileInfo: error open: %d: %s", xine_get_error(stream_info), mrl);
                return info;
            }
            info = GetStreamInfo(stream_info);
            xine_close(stream_info);
        } else {
            info = GetStreamInfo(stream);
        }
    }
    return info;
}

void XinePlayerObject::VideoParseFilter(const char *filters)
{
    if (TryLock()) {
        return;
    }
    while (video_filter.size() > 0) {
        VideoFilter *filter = (VideoFilter*)video_filter.back();
        free(filter->name);
        while (filter->params.size() > 0) {
            delete filter->params.back();
            filter->params.pop_back();
        }
        delete filter;
        video_filter.pop_back();
    }
    if (!filters || !*filters) {
        Unlock();
        return;
    }
    char *p, *name, *value, *data = strdup(filters);
    while ((name = strsep(&data, ";"))) {
        VideoFilter *filter = new VideoFilter;
        name = trimString(name);
        if ((p = strchr(name, ':'))) {
	    *p++ = 0;
	}
        filter->name = name;
        video_filter.push_back((char*)filter);
        if (!p || !*p) {
            continue;
        }
	while ((name = strsep(&p, ","))) {
            if ((value = strchr(name, '='))) {
  	        *value++ = 0;
            }
            if (!value) {
                continue;
            }
            ObjectProperty *param = new ObjectProperty(name);
            param->SetValue(value);
            filter->params.push_back(param);
        }
    }
    free(data);
    Unlock();
}

void XinePlayerObject::VideoUnwireFilter(void)
{
    if (!xine) {
        return;
    }
    Lock();
    if (stream) {
        xine_post_wire_video_port(xine_get_video_source(stream), vo);
    }
    while (video_post.size() > 0) {
        xine_post_dispose(xine, video_post.back());
        video_post.pop_back();
    }
    Unlock();
}

void XinePlayerObject::VideoWireFilter(void)
{
    if (!stream || !video_filter.size()) {
        return;
    }
    if (TryLock()) {
        return;
    }
    int i, readonly;
    char *param_data, *value;
    vector < char *> params;
    xine_post_t *post;
    xine_post_api_t *post_api;
    xine_post_in_t *input_api;
    xine_post_api_descr_t *descr;
    xine_post_api_parameter_t *param;

    for (int j = 0; j < video_filter.size(); j++) {
        VideoFilter *filter = (VideoFilter*)video_filter[j];
	post = xine_post_init(xine, filter->name, 0, &ao, &vo);
	if (!post) {
            break;
        }
        video_post.push_back(post);
        if (!filter->params.size()) {
           continue;
        }
        while (params.size() > 0) {
            free(params.back());
            params.pop_back();
        }
        input_api = (xine_post_in_t *) xine_post_input(post, "parameters");
        if (!input_api) {
            continue;
        }
        post_api = (xine_post_api_t *) input_api->data;
        descr = post_api->get_param_descr();
        param = descr->parameter;
        param_data = (char*)malloc(descr->struct_size);
        while (param->type != POST_PARAM_TYPE_LAST) {
            post_api->get_parameters(post, param_data);
            params.push_back(strdup(param->name));
            param++;
        }
        if (!params.size()) {
            free(param_data);
            continue;
        }
	for (int j = 0; j < filter->params.size(); j++) {
	    for (i = 0; i < params.size(); i++) {
                 if (!strcasecmp(params[i], filter->params[j]->GetName())) {
                     break;
                 }
            }
	    if (i >= params.size()) {
                continue;
            }
            value = (char*)filter->params[j]->GetString();
	    param = descr->parameter;
	    param += i;
	    readonly = param->readonly;
   	    switch (param->type) {
	     case POST_PARAM_TYPE_INT:
	        if (readonly) {
                    break;
                }
   	        if (param->enum_values) {
 	            char **values = param->enum_values;
	            i = 0;
	            while (values[i]) {
	                if (!strcasecmp(values[i], value)) {
 	                    *(int *)(param_data + param->offset) = i;
	                    break;
	                }
	                i++;
	            }
	            if (!values[i]) {
	                *(int *)(param_data + param->offset) = (int) strtol(value, 0, 10);
                    }
	        } else {
	           *(int *)(param_data + param->offset) = (int) strtol(value, 0, 10);
	        }
                post_api->set_parameters(post, param_data);
                post_api->get_parameters(post, param_data);
	        break;

             case POST_PARAM_TYPE_DOUBLE:
	        if (readonly) {
	           break;
                }
                *(double *)(param_data + param->offset) = strtod(value, 0);
      	        post_api->set_parameters(post, param_data);
                post_api->get_parameters(post, param_data);
                break;

	     case POST_PARAM_TYPE_CHAR:
	     case POST_PARAM_TYPE_STRING:
	        if (readonly) {
	            break;
                }
                if (param->type == POST_PARAM_TYPE_CHAR) {
	            int maxlen = param->size / sizeof(char);
	            snprintf((char *)(param_data + param->offset), maxlen, "%s", value);
	            post_api->set_parameters(post, param_data);
                    post_api->get_parameters(post, param_data);
                 } else {
	           fprintf(stderr, "parameter type POST_PARAM_TYPE_STRING not supported yet.\n");
	         }
  	         break;

	     case POST_PARAM_TYPE_STRINGLIST:
	        if (readonly) {
	            break;
                }
                fprintf(stderr, "parameter type POST_PARAM_TYPE_STRINGLIST not supported yet.\n");
       	        break;

	     case POST_PARAM_TYPE_BOOL:
	        if (readonly) {
	            break;
                }
                *(int *)(param_data + param->offset) = ((int) strtol(value, 0, 10)) ? 1 : 0;
	        post_api->set_parameters(post, param_data);
                post_api->get_parameters(post, param_data);
                break;
	    }
	}
	free(param_data);
    }
    while (params.size() > 0) {
        free(params.back());
        params.pop_back();
    }

    if (!video_post.size()) {
        Unlock();
        return;
    }
    for (int i = video_post.size() - 1; i >= 0; i--) {
        const char *const *outs = xine_post_list_outputs(video_post[i]);
        const xine_post_out_t *vo_out = xine_post_output(video_post[i], (char *) *outs);
        if (i == video_post.size() - 1) {
	    xine_post_wire_video_port((xine_post_out_t *)vo_out, vo);
        } else {
	   const xine_post_in_t *vo_in = xine_post_input(video_post[i + 1], "video");
	   xine_post_wire((xine_post_out_t *)vo_out, (xine_post_in_t *)vo_in);
        }
    }
    xine_post_wire_video_port(xine_get_video_source(stream), video_post[0]->video_input[0]);
    Unlock();
}

void XinePlayerObject::AudioParseFilter(const char *filter)
{
    if (TryLock()) {
        return;
    }
    lmbox_free(audio_filter);
    audio_filter = lmbox_strdup(filter);
    Unlock();
}

void XinePlayerObject::AudioWireFilter(void)
{
    if (TryLock()) {
        return;
    }
    if (stream && !isEmptyString(audio_filter)) {
        audio_post = xine_post_init(xine, audio_filter, 0, &ao, &vo);
        if (audio_post) {
            xine_post_wire_audio_port(xine_get_audio_source(stream), audio_post->audio_input[0]);
        }
    }
    Unlock();
}

void XinePlayerObject::AudioUnwireFilter(void)
{
    Lock();
    if (stream) {
        xine_post_wire_audio_port(xine_get_audio_source(stream), ao);
    }
    if (audio_post) {
        xine_post_dispose(xine, audio_post);
        audio_post = 0;
    }
    Unlock();
}

int XinePlayerObject::Snapshot(const char *filename)
{
#ifdef LOAD_PNG
    FILE* fd;
    png_infop pnginfo;
    png_structp pngstruct;
    uint8_t *yuv, *rgb = 0;
    int width, height, ratio, format = 0;

    if (!filename || !filename[0]) {
        if (TryLock()) {
            return -1;
        }
        if (playlist_files.size() > 0 &&
            playlist_index >= 0 &&
            playlist_index < playlist_files.size()) {
            char *file = tlsAlloc(1, strlen(playlist_files[playlist_index]->name()) + 4);
            strcpy(file, playlist_files[playlist_index]->name());
            char *dot = strrchr(file, '.');
            if (dot) {
                *dot = 0;
            }
            strcat(file, ".png");
            filename = file;
        }
        Unlock();
    }
    if (!filename || !filename[0]) {
        systemObject->Log(0, "Snapshot: no file");
        return -1;
    }
    if (TryLock()) {
        return -1;
    }
    if (stream) {
        xine_get_current_frame (stream, &width, &height, &ratio, &format, NULL);
        if (format == XINE_IMGFMT_YUY2) {
            yuv = (uint8_t*)malloc ((width+8) * (height+1) * 2);
            xine_get_current_frame (stream, &width, &height, &ratio, &format, yuv);
            rgb = yuy2torgb (yuv, width, height);
            free (yuv);
        }
        if (format == XINE_IMGFMT_YV12) {
            yuv = (uint8_t*)malloc ((width+8) * (height+1) * 2);
            xine_get_current_frame (stream, &width, &height, &ratio, &format, yuv);
            rgb = yv12torgb (yuv, width, height);
            free (yuv);
        }
    }
    Unlock();
    if (!rgb) {
        systemObject->Log(0, "Snapshot: %s: no image", filename);
        return -1;
    }
    pngstruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (pngstruct == NULL) {
        return -1;
    }
    pnginfo = png_create_info_struct (pngstruct);
    if (pnginfo == NULL) {
        return -1;
    }
    if (setjmp(png_jmpbuf(pngstruct))) {
        lmbox_free(rgb);
        return -1;
    }
    fd = fopen (filename, "wb");
    if (fd == NULL) {
        systemObject->Log(0, "Snapshot: %s: %s", filename, strerror(errno));
        free(rgb);
        return -1;
    }
    png_init_io (pngstruct, fd);
    png_set_filter(pngstruct, 0, PNG_FILTER_NONE  | PNG_FILTER_VALUE_NONE );
    png_set_IHDR (pngstruct, pnginfo, width, height, 8,
	          PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
	          PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(pngstruct, pnginfo);
    png_bytep* row = (png_bytep*)calloc(1,sizeof(png_bytep) * height);
    for (int i = 0; i < height; i++) {
        row[i] = rgb + i * (width*3);
    }
    png_write_image(pngstruct, row);
    free(row);
    png_write_end(pngstruct, pnginfo);
    png_destroy_write_struct(&pngstruct, &pnginfo);
    fclose(fd);
    lmbox_free(rgb);
#endif
    return 0;
}

char *XinePlayerObject::GetStreamInfo(xine_stream_t *xstream)
{
    if (!xstream) {
        return 0;
    }
    char *result = tlsStr(1, "");
    const char *title = xine_get_meta_info(xstream, XINE_META_INFO_TITLE);
    const char *vcodec = xine_get_meta_info(xstream, XINE_META_INFO_VIDEOCODEC);
    const char *acodec = xine_get_meta_info(xstream, XINE_META_INFO_AUDIOCODEC);
    int vwidth = xine_get_stream_info(xstream, XINE_STREAM_INFO_VIDEO_WIDTH);
    int vheight = xine_get_stream_info(xstream, XINE_STREAM_INFO_VIDEO_HEIGHT);
    int vbitrate = xine_get_stream_info(xstream, XINE_STREAM_INFO_VIDEO_BITRATE);
    int xbitrate = xine_get_stream_info(xstream, XINE_STREAM_INFO_BITRATE);
    uint32_t vfcc = xine_get_stream_info(xstream, XINE_STREAM_INFO_VIDEO_FOURCC);
    int asrate = xine_get_stream_info(xstream, XINE_STREAM_INFO_AUDIO_SAMPLERATE);
    int audiochannel = xine_get_param(xstream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL);
    int achannels = xine_get_stream_info(xstream, XINE_STREAM_INFO_AUDIO_CHANNELS);
    int abits = xine_get_stream_info(xstream, XINE_STREAM_INFO_AUDIO_BITS);
    int spuchannel = xine_get_param(xstream, XINE_PARAM_SPU_CHANNEL);
    char buffer[512];

    if (!title) {
        title = GetPlaylistFile();
    }
    if (title && *title) {
        snprintf(buffer, sizeof(buffer), "%s", title);
        if (!strncmp(title, "pvr:/", 5) || !strncmp(title, "v4l:/", 5)) {
            DynamicObject *tv = systemObject->FindObject("tv");
            if (tv) {
                Variant norm = tv->GetPropertyValue("norm");
                Variant chan = tv->GetPropertyValue("channel");
                snprintf(buffer, sizeof(buffer), "TV Channel: %s/%s", (char*)chan, (char*)norm);
                tv->DecRef();
            }
        }
        tlsStrAppend(1, buffer);
    }

    if ((title = xine_get_meta_info(xstream, XINE_META_INFO_TITLE))) {
        snprintf(buffer, sizeof(buffer), "\nTitle: %s", title);
        tlsStrAppend(1, buffer);
    }
    if ((title = xine_get_meta_info(xstream, XINE_META_INFO_ARTIST))) {
        snprintf(buffer, sizeof(buffer), "\nArtist: %s", title);
        tlsStrAppend(1, buffer);
    }
    if ((title = xine_get_meta_info(xstream, XINE_META_INFO_ALBUM))) {
        snprintf(buffer, sizeof(buffer), "\nAlbum: %s", title);
        tlsStrAppend(1, buffer);
    }
    if ((title = xine_get_meta_info(xstream, XINE_META_INFO_GENRE))) {
        snprintf(buffer, sizeof(buffer), "\nGenre: %s", title);
        tlsStrAppend(1, buffer);
    }
    if ((title = xine_get_meta_info(xstream, XINE_META_INFO_COMMENT))) {
        snprintf(buffer, sizeof(buffer), "\nComment: %s", title);
        tlsStrAppend(1, buffer);
    }

    if (vcodec && vwidth && vheight) {
        char sfcc[5] = { 0, 0, 0, 0, 0 };
        sfcc[0] = vfcc | 0xFFFFFF00;
        sfcc[1] = vfcc >> 8 | 0xFFFFFF00;
        sfcc[2] = vfcc >> 16 | 0xFFFFFF00;
        sfcc[3] = vfcc >> 24 | 0xFFFFFF00;
        if (vfcc <= 0xFFFF) {
            snprintf(sfcc, sizeof(sfcc), "0x%x", vfcc);
        }
        if ((sfcc[0] == 'm') && (sfcc[1] == 's')) {
            if ((sfcc[2] = 0x0) && (sfcc[3] == 0x55)) {
                *(uint32_t *) sfcc = 0x33706d2e;
            }
        }
        snprintf(buffer, sizeof(buffer), "\n%s: %s %dX%d %d bit/s", vcodec, sfcc, vwidth, vheight, vbitrate ? vbitrate : xbitrate);
        tlsStrAppend(1, buffer);
    }

    if (acodec && asrate) {
        snprintf(buffer, sizeof(buffer), "\n%s: %d ch %d Hz %d bits", acodec, achannels, asrate, abits);
        tlsStrAppend(1, buffer);
    }

    switch (audiochannel) {
    case -2:
        strcpy(buffer, "Audio: off");
        break;
    case -1:
        if (!xine_get_audio_lang(xstream, audiochannel, &buffer[strlen(buffer)])) {
            strcpy(buffer, "Audio: auto");
        }
        break;
    default:
        if (!xine_get_audio_lang(xstream, audiochannel, &buffer[strlen(buffer)])) {
            sprintf(&buffer[strlen(buffer)], "Audio: %3d", audiochannel);
        }
        break;
    }

    switch (spuchannel) {
    case -2:
        strcat(buffer, ", Spu: off");
        break;
    case -1:
        if (!xine_get_spu_lang(xstream, spuchannel, &buffer[strlen(buffer)])) {
            strcat(buffer, ", Spu: auto");
        }
        break;
    default:
        if (!xine_get_spu_lang(xstream, spuchannel, &buffer[strlen(buffer)])) {
            sprintf(&buffer[strlen(buffer)], ", Spu: %3d", spuchannel);
        }
        break;
    }
    tlsStrAppend(1, "\n");
    result = tlsStrAppend(1, buffer);

    buffer[0] = 0;
    int length = 0;
    int pos = GetTimePosition(&length);
    if (pos >= 0) {
        pos /= 1000;
        length /= 1000;
        int days = pos / (3600 * 24);
        if (days > 0) {
            sprintf(buffer, "%d::%02d ", days, pos / 3600);
        } else {
            sprintf(buffer, "%d:%02d:%02d ", pos / 3600, (pos % 3600) / 60, pos % 60);
        }
    }

    if (length > 0) {
        int days = length / (3600 * 24);
        sprintf(buffer, "%s(%.0f%%) of ", buffer, ((float) pos / (float) length) * 100);
        if (days > 0) {
            sprintf(buffer, "%s%d::%02d", buffer, days, length / 3600);
        } else {
            sprintf(buffer, "%s%d:%02d:%02d", buffer, length / 3600, (length % 3600) / 60, length % 60);
        }
    }
    if (buffer[0]) {
        tlsStrAppend(1, "\n");
        result = tlsStrAppend(1, buffer);
    }
    return result;
}

int XinePlayerObject::GetAVOffset(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_AV_OFFSET);
    }
    return rc;
}

void XinePlayerObject::SetAVOffset(int val)
{
    if (stream) {
        xine_set_param(stream, XINE_PARAM_AV_OFFSET, val);
    }
    PlayerObject::SetAVOffset(val);
}

int XinePlayerObject::GetSPUOffset(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_SPU_OFFSET);
    }
    return rc;
}

void XinePlayerObject::SetSPUOffset(int val)
{
    if (stream) {
        xine_set_param(stream, XINE_PARAM_SPU_OFFSET, val);
    }
    PlayerObject::SetSPUOffset(val);
}

int XinePlayerObject::GetVolume(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_AUDIO_VOLUME);
    }
    return rc;
}

void XinePlayerObject::SetVolume(int val)
{
    if (stream) {
        if (val < 0) {
            val = 0;
        }
        if (val > 100) {
            val = 100;
        }
        xine_set_param(stream, XINE_PARAM_AUDIO_VOLUME, val);
    }
    PlayerObject::SetVolume(val);
}

int XinePlayerObject::GetMute(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_AUDIO_MUTE);
    }
    return rc;
}

void XinePlayerObject::SetMute(int val)
{
    if (stream) {
        if (val < 0) {
            val = 0;
        }
        if (val > 1) {
            val = 1;
        }
        xine_set_param(stream, XINE_PARAM_AUDIO_MUTE, val);
    }
    PlayerObject::SetMute(val);
}

int XinePlayerObject::GetAmpMute(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_AUDIO_AMP_MUTE);
    }
    return rc;
}

void XinePlayerObject::SetAmpMute(int val)
{
    if (stream) {
        if (val < 0) {
            val = 0;
        }
        if (val > 1) {
            val = 1;
        }
        xine_set_param(stream, XINE_PARAM_AUDIO_AMP_MUTE, val);
    }
    PlayerObject::SetAmpMute(val);
}

int XinePlayerObject::GetAmpLevel(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_AUDIO_AMP_LEVEL);
    }
    return rc;
}

void XinePlayerObject::SetAmpLevel(int val)
{
    if (stream) {
        if (val < 0) {
            val = 0;
        }
        if (val > 100) {
            val = 100;
        }
        xine_set_param(stream, XINE_PARAM_AUDIO_AMP_LEVEL, val);
    }
    PlayerObject::SetAmpLevel(val);
}

int XinePlayerObject::GetHue(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_VO_HUE);
    }
    return rc;
}

void XinePlayerObject::SetHue(int val)
{
    if (stream) {
        if (val < 0) {
            val = 0;
        }
        if (val > 65535) {
            val = 65535;
        }
        xine_set_param(stream, XINE_PARAM_VO_HUE, val);
    }
    PlayerObject::SetHue(val);
}

int XinePlayerObject::GetSaturation(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_VO_SATURATION);
    }
    return rc;
}

void XinePlayerObject::SetSaturation(int val)
{
    if (stream) {
        if (val < 0) {
            val = 0;
        }
        if (val > 65535) {
            val = 65535;
        }
        xine_set_param(stream, XINE_PARAM_VO_SATURATION, val);
    }
    PlayerObject::SetSaturation(val);
}

int XinePlayerObject::GetBrightness(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_VO_BRIGHTNESS);
    }
    return rc;
}

void XinePlayerObject::SetBrightness(int val)
{
    if (stream) {
        if (val < 0) {
            val = 0;
        }
        if (val > 65535) {
            val = 65535;
        }
        xine_set_param(stream, XINE_PARAM_VO_BRIGHTNESS, val);
    }
    PlayerObject::SetBrightness(val);
}

int XinePlayerObject::GetContrast(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_VO_CONTRAST);
    }
    return rc;
}

void XinePlayerObject::SetContrast(int val)
{
    if (stream) {
        if (val < 0) {
            val = 0;
        }
        if (val > 65535) {
            val = 65535;
        }
        xine_set_param(stream, XINE_PARAM_VO_CONTRAST, val);
    }
    PlayerObject::SetContrast(val);
}

int XinePlayerObject::GetAudioChannel(void)
{
    if (stream) {
        return xine_get_param(stream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL);
    }
    return PlayerObject::GetAudioChannel();
}

void XinePlayerObject::SetAudioChannel(int val)
{
    if (stream) {
        if (val < -2) {
            val = -2;
        }
        if (val > 100) {
            val = 100;
        }
        xine_set_param(stream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL, val);
    }
    PlayerObject::SetAudioChannel(val);
}

int XinePlayerObject::GetSPUChannel(void)
{
    if (stream) {
        return xine_get_param(stream, XINE_PARAM_SPU_CHANNEL);
    }
    return PlayerObject::GetSPUChannel();
}

void XinePlayerObject::SetSPUChannel(int val)
{
    if (stream) {
        xine_set_param(stream, XINE_PARAM_SPU_CHANNEL, val);
    }
    PlayerObject::SetSPUChannel(val);
}

int XinePlayerObject::GetSpeed(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_param(stream, XINE_PARAM_SPEED);
    }
    return rc;
}

void XinePlayerObject::SetSpeed(char *value)
{
    static char *speeds[] = { "normal", (char*)XINE_SPEED_NORMAL,
                              "pause", (char*)XINE_SPEED_PAUSE,
                              "slow4", (char*)XINE_SPEED_SLOW_4,
                              "slow2", (char*)XINE_SPEED_SLOW_2,
                              "fast2", (char*)XINE_SPEED_FAST_2,
                              "fast4", (char*)XINE_SPEED_FAST_4,
                              0, 0};
    int val = atoi(value);
    char *sval = value;
    for (int i = 0; speeds[i]; i+= 2) {
        if (!strcmp(sval, speeds[i])) {
            val = (int)speeds[i+1];
            break;
        }
    }
    if (stream) {
        xine_set_param(stream, XINE_PARAM_SPEED, val);
    }
    PlayerObject::SetSpeed(value);
}

int XinePlayerObject::GetLength(void)
{
    int len = 0;
    GetTimePosition(&len);
    return len;
}

char *XinePlayerObject::GetDeinterlaceFilters(void)
{
    char *rc = tlsStr(0, "");
    if (TryLock()) {
        return rc;
    }
    for (int i = 0; deinterlace_filters[i]; i += 3) {
        rc = tlsStrAppend(0, "\"");
        rc = tlsStrAppend(0, deinterlace_filters[i]);
        rc = tlsStrAppend(0, "\" \"");
        rc = tlsStrAppend(0, deinterlace_filters[i + 1]);
        rc = tlsStrAppend(0, "\" \"");
        rc = tlsStrAppend(0, deinterlace_filters[i + 2]);
        rc = tlsStrAppend(0, "\" ");
    }
    Unlock();
    return rc;
}

char *XinePlayerObject::GetVideoFilter(void)
{
    char *rc = tlsStr(0, "");
    if (TryLock()) {
        return rc;
    }
    for (int i = 0; i < video_filter.size(); i++) {
        VideoFilter *filter = (VideoFilter*)video_filter[i];
        rc = tlsStrAppend(0, filter->name);
        rc = tlsStrAppend(0, ":");
        for (int j = 0; j < filter->params.size(); j++) {
            rc = tlsStrAppend(0, filter->params[j]->GetName());
            rc = tlsStrAppend(0, "=");
            rc = tlsStrAppend(0, filter->params[j]->GetString());
            if (j < filter->params.size() - 1) {
                rc = tlsStrAppend(0, ",");
            }
        }
    }
    Unlock();
    return rc;
}

void XinePlayerObject::SetVideoFilter(const char *value)
{
    VideoParseFilter(value);
    VideoUnwireFilter();
    VideoWireFilter();
}

char *XinePlayerObject::GetAudioFilter(void)
{
    return tlsStr(0, audio_filter ? audio_filter : "");
}

void XinePlayerObject::SetAudioFilter(const char *value)
{
    AudioParseFilter(value);
    AudioUnwireFilter();
    AudioWireFilter();
}

void XinePlayerObject::SetBroadcastPort(int value)
{
    if (TryLock()) {
        return;
    }
    broadcast_port = value;
    Unlock();
    if (stream) {
        xine_set_param(stream, XINE_PARAM_BROADCASTER_PORT, value);
    }
}

char *XinePlayerObject::GetVisualizationFilters(void)
{
    char *rc = tlsStr(0, "");
    const char *const *plist = xine_list_post_plugins_typed(xine, XINE_POST_TYPE_AUDIO_VISUALIZATION);
    for (int  i = 0;plist && plist[i]; i++) {
         rc = tlsStrAppend(0, plist[i]);
         rc = tlsStrAppend(0, " ");
    }
    return rc;
}

const char *XinePlayerObject::GetStatus(void)
{
    int rc = 0;
    if (stream) {
        rc = xine_get_status(stream);
    }
    switch (rc) {
    case XINE_STATUS_PLAY:
        if (xine_get_param(stream, XINE_PARAM_SPEED) == XINE_SPEED_PAUSE) {
            return "pause";
        } else {
            return "play";
        }
    case XINE_STATUS_STOP:
        return "stop";
    case XINE_STATUS_QUIT:
        return "quit";
    }
    return "idle";
}

void XinePlayerObject::GetAutoPlay(const char *mrl)
{
    if (PlayerInit()) {
        return;
    }
    if (TryLock()) {
        return;
    }
    int i, nmrls;
    char **mrls = xine_get_autoplay_mrls(xine, mrl, &nmrls);
    if (mrls) {
        for (i = 0; i < nmrls; i++) {
            playlist_files.push_back(new FileEntry(mrls[i], 0, 0, NULL));
        }
    }
    Unlock();
}

#endif
