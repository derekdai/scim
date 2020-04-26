/** @file scim_ibus_frontend.cpp
 * implementation of class IBusFrontEnd.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * $Id: scim_ibus_frontend.cpp,v 1.37 2005/07/03 08:36:42 suzhe Exp $
 *
 */

#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_FRONTEND
#define Uses_SCIM_PANEL_CLIENT
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_HOTKEY
#define Uses_STL_UTILITY
#define Uses_C_STDIO
#define Uses_C_STDLIB

#include <set>
#include <sstream>
#include <cassert>
#include <cstdarg>
#include <sys/time.h>
#include <limits.h>
#include <systemd/sd-event.h>
#include <systemd/sd-bus.h>
#include "scim_private.h"
#include "scim.h"
#include "scim_ibus_frontend.h"
#include "scim_ibus_types.h"
#include "scim_ibus_utils.h"

#define scim_module_init ibus_LTX_scim_module_init
#define scim_module_exit ibus_LTX_scim_module_exit
#define scim_frontend_module_init ibus_LTX_scim_frontend_module_init
#define scim_frontend_module_run ibus_LTX_scim_frontend_module_run

#define SCIM_CONFIG_FRONTEND_IBUS_CONFIG_READONLY "/FrontEnd/IBus/ConfigReadOnly"
#define SCIM_CONFIG_FRONTEND_IBUS_MAXCLIENTS      "/FrontEnd/IBus/MaxClients"

#define SCIM_KEYBOARD_ICON_FILE                   (SCIM_ICONDIR "/keyboard.png")

#define STRLEN(s)                                 (sizeof (s) - 1)
#define IBUS_PORTAL_SERVICE                       "org.freedesktop.portal.IBus"
#define IBUS_PORTAL_OBJECT_PATH                   "/org/freedesktop/IBus"
#define IBUS_PORTAL_INTERFACE                     "org.freedesktop.IBus.Portal"
#define IBUS_INPUTCONTEXT_OBJECT_PATH             "/org/freedesktop/IBus/InputContext_"
#define IBUS_INPUTCONTEXT_INTERFACE               "org.freedesktop.IBus.InputContext"
#define IBUS_SERVICE_INTERFACE                    "org.freedesktop.IBus.Service"
#define IBUS_INPUTCONTEXT_OBJECT_PATH_BUF_SIZE    (STRLEN (IBUS_INPUTCONTEXT_OBJECT_PATH) + 10)

#define cleanup_strv                              __attribute__((cleanup(free_strvp)))
#define autounref(t)                              __attribute__((cleanup(t ## _unrefp))) t *

using namespace scim;

struct IBusContentType
{
    uint32_t purpose;
    uint32_t hints;

    IBusContentType () : purpose (0), hints (0) { }

    int from_message (sd_bus_message *m)
    {
        int r;
        if ((r = sd_bus_message_enter_container (m, 'r', "uu")) < 0) {
            return r;
        }

        return sd_bus_message_read (m, "uu", &purpose, &hints);
    }

    int to_message (sd_bus_message *m) const
    {
        int r;
        if ((r = sd_bus_message_open_container(m, 'r', "uu")) < 0) {
            return r;
        }

        if ((r = sd_bus_message_append(m, "uu", purpose, hints)) < 0) {
            return r;
        }

        return sd_bus_message_close_container(m);
    }
};

struct IBusRect
{
    int x;
    int y;
    int w;
    int h;

    IBusRect (): x (0), y (0), w (0), h (0) { }

    int from_message (sd_bus_message *m)
    {
        return sd_bus_message_read (m, "iiii", &x, &y, &w, &h);
    }

    const IBusRect & operator +=(const IBusRect &other)
    {
        x += other.x;
        y += other.y;
        w += other.w;
        h += other.h;

        return *this;
    }
};

class IBusCtx
{
    int                         m_id;
    int                         m_siid;
    uint32_t                    m_caps;
    IBusContentType             m_content_type;
    IBusRect                    m_cursor_location;
    bool                        m_client_commit_preedit;
    bool                        m_on;
    bool                        m_shared_siid;
    String                      m_locale;
    KeyboardLayout              m_keyboard_layout;
    sd_bus_slot                *m_inputcontext_slot;
    sd_bus_slot                *m_service_slot;

    static const sd_bus_vtable  m_inputcontext_vtbl[];
    static const sd_bus_vtable  m_service_vtbl[];

public:
    IBusCtx (const String &locale, int id, int siid);

    ~IBusCtx();

    int init (sd_bus *bus, const char *path);

    int id () const { return m_id; }

    uint32_t caps () const { return m_caps; }
    uint32_t scim_caps () const;
    int caps_from_message (sd_bus_message *v);

    const IBusContentType &content_type() const { return m_content_type; }
    int content_type_from_message (sd_bus_message *v);
    int content_type_to_message (sd_bus_message *v) const;

    const IBusRect &cursor_location() const { return m_cursor_location; }
    IBusRect &cursor_location() { return m_cursor_location; }
    int cursor_location_from_message (sd_bus_message *v);
    int cursor_location_relative_from_message (sd_bus_message *v);

    int siid() const { return m_siid; }
    void siid (int siid) { m_siid = siid; }

    bool shared_siid() const { return m_shared_siid; }
    void shared_siid (bool shared) { m_shared_siid = shared; }

    const String &locale() const { return m_locale; }
    String encoding() const { return scim_get_locale_encoding (m_locale); }
    void locale (const String &locale) { m_locale = locale; }

    bool client_commit_preedit() const { return m_client_commit_preedit; }
    int client_commit_preedit_from_message (sd_bus_message *v);
    int client_commit_preedit_to_message (sd_bus_message *v) const;

    KeyboardLayout keyboard_layout() const { return m_keyboard_layout; }
    void keyboard_layout(KeyboardLayout layout) { m_keyboard_layout = layout; }

    bool is_on() const { return m_on; }
    void on() { m_on = true; }
    void off() { m_on = false; }
};

static void free_strv (char **strv);
static void free_strvp (char ***strv);

template<int (IBusFrontEnd::*mf)(sd_bus_message *m)>
static int portal_message_adapter (sd_bus_message *m,
                                   void *userdata,
                                   sd_bus_error *ret_error);

template<int (IBusFrontEnd::*mf)(IBusCtx *ctx, sd_bus_message *m)>
static int
ctx_message_adapter(sd_bus_message *m,
                    void *userdata,
                    sd_bus_error *ret_error);

template<int (IBusFrontEnd::*mf) (IBusCtx *ctx, sd_bus_message *value)>
static int
ctx_prop_adapter (sd_bus *bus,
 	              const char *path,
 	              const char *interface,
 	              const char *property,
 	              sd_bus_message *value,
 	              void *userdata,
 	              sd_bus_error *ret_error);

template<typename T, int (T::*mf)(sd_event_source *s, int fd, uint32_t revents)>
int
sd_event_io_adapter(sd_event_source *s, int fd, uint32_t revents, void *userdata)
{
    return (static_cast<T *>(userdata)->*mf)(s, fd, revents);
}

static inline const char * ctx_gen_object_path (int id,
                                                char *buf,
                                                size_t buf_size);

static Pointer <IBusFrontEnd> _scim_frontend (0);

static int _argc;
static char **_argv;

const sd_bus_vtable IBusFrontEnd::m_portal_vtbl [] = {
    SD_BUS_VTABLE_START (0),
    SD_BUS_METHOD_WITH_NAMES (
            "CreateInputContext",
            "s", SD_BUS_PARAM (client_name),
            "o", SD_BUS_PARAM (object_path),
            (&portal_message_adapter<&IBusFrontEnd::portal_create_ctx>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

const sd_bus_vtable IBusCtx::m_inputcontext_vtbl [] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_NAMES(
            "ProcessKeyEvent",
            "uuu", SD_BUS_PARAM(keyval) SD_BUS_PARAM(keycode) SD_BUS_PARAM(state),
            "b", SD_BUS_PARAM(handled),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_process_key_event>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCursorLocation",
            "iiii", SD_BUS_PARAM(x) SD_BUS_PARAM(y) SD_BUS_PARAM(w) SD_BUS_PARAM(h),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_set_cursor_location>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCursorLocationRelative",
            "iiii", SD_BUS_PARAM(x) SD_BUS_PARAM(y) SD_BUS_PARAM(w) SD_BUS_PARAM(h),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_set_cursor_location_relative>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "ProcessHandWritingEvent",
            "ad", SD_BUS_PARAM(coordinates),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_process_hand_writing_event>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "CancelHandWriting",
            "u", SD_BUS_PARAM(n_strokes),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_cancel_hand_writing>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "FocusIn", "", "",
            (&ctx_message_adapter<&IBusFrontEnd::ctx_focus_in>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "FocusOut", "", "",
            (&ctx_message_adapter<&IBusFrontEnd::ctx_focus_out>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "Reset", "", "",
            (&ctx_message_adapter<&IBusFrontEnd::ctx_reset>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCapabilities",
            "u", SD_BUS_PARAM(caps),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_set_capabilities>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "PropertyActivate",
            "su", SD_BUS_PARAM(name) SD_BUS_PARAM(state),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_property_activate>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetEngine",
            "s", SD_BUS_PARAM(name),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_set_engine>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "GetEngine",
            "", SD_BUS_PARAM(),
            "v", SD_BUS_PARAM(desc),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_get_engine>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetSurroundingText",
            "vuu", SD_BUS_PARAM(text) SD_BUS_PARAM(cursor_pos) SD_BUS_PARAM(anchor_pos),
            "v", SD_BUS_PARAM(desc),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_set_surrounding_text>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_SIGNAL_WITH_NAMES(
            "CommitText",
            "v", SD_BUS_PARAM(text),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "ForwardKeyEvent",
            "uuu", SD_BUS_PARAM(keyval) SD_BUS_PARAM(keycode) SD_BUS_PARAM(state),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "UpdatePreeditText",
            "vub", SD_BUS_PARAM(text) SD_BUS_PARAM(cursor_pos) SD_BUS_PARAM(visible),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "UpdatePreeditTextWithMode",
            "vubu", SD_BUS_PARAM(text) SD_BUS_PARAM(cursor_pos) SD_BUS_PARAM(visible) SD_BUS_PARAM(mode),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "ShowPreeditText",
            "", SD_BUS_PARAM(),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "HidePreeditText",
            "", SD_BUS_PARAM(),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "UpdateAuxiliaryTextWithMode",
            "vb", SD_BUS_PARAM(text) SD_BUS_PARAM(visible),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "ShowAuxiliaryText",
            "", SD_BUS_PARAM(),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "HideAuxiliaryText",
            "", SD_BUS_PARAM(),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "UpdateLookupTable",
            "vb", SD_BUS_PARAM(table) SD_BUS_PARAM(visible),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "ShowLookupTable",
            "", SD_BUS_PARAM(),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "HideLookupTable",
            "", SD_BUS_PARAM(),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "PageUpLookupTable",
            "", SD_BUS_PARAM(),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "PageDownLookupTable",
            "", SD_BUS_PARAM(),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "CursorUpLookupTable",
            "", SD_BUS_PARAM(),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "CursorDownLookupTable",
            "", SD_BUS_PARAM(),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "RegisterProperties",
            "v", SD_BUS_PARAM(props),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "UpdateProperty",
            "v", SD_BUS_PARAM(prop),
            0),
    SD_BUS_WRITABLE_PROPERTY(
            "ClientCommitPreedit",
            "(b)",
            (&ctx_prop_adapter<&IBusFrontEnd::ctx_get_client_commit_preedit>),
            (&ctx_prop_adapter<&IBusFrontEnd::ctx_set_client_commit_preedit>),
            0,
            SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_WRITABLE_PROPERTY(
            "ContentType",
            "(uu)",
            (&ctx_prop_adapter<&IBusFrontEnd::ctx_get_content_type>),
            (&ctx_prop_adapter<&IBusFrontEnd::ctx_set_content_type>),
            0,
            SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_VTABLE_END,
};

const sd_bus_vtable IBusCtx::m_service_vtbl[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD(
            "Destroy",
            "",
            "",
            (&ctx_message_adapter<&IBusFrontEnd::srv_destroy>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

IBusFrontEnd::IBusFrontEnd (const BackEndPointer &backend,
                                const ConfigPointer  &config)
    : FrontEndBase (backend),
      m_config (config),
      m_stay (true),
//      m_config_readonly (false),
//      m_socket_timeout (scim_get_default_socket_timeout ()),
      m_current_instance (-1),
//      m_current_socket_client (-1),
      m_current_ibus_ctx (NULL),
//      m_current_socket_client_key (0),
      m_ctx_counter (0),
      m_loop (NULL),
      m_bus (NULL),
      m_portal_slot (NULL),
      m_panel_source (NULL)//,
//      m_inputcontext_slot (NULL),
//      m_service_slot (NULL),
//      m_ctx_enum_slot (NULL),
//      m_obj_mngr_slot (NULL)
{
    log_func ();

    SCIM_DEBUG_FRONTEND (2) << " Constructing IBusFrontEnd object...\n";

    // Attach Panel Client signal.
    m_panel_client.signal_connect_reload_config                 (slot (this, &IBusFrontEnd::panel_slot_reload_config));
    m_panel_client.signal_connect_exit                          (slot (this, &IBusFrontEnd::panel_slot_exit));
    m_panel_client.signal_connect_update_lookup_table_page_size (slot (this, &IBusFrontEnd::panel_slot_update_lookup_table_page_size));
    m_panel_client.signal_connect_lookup_table_page_up          (slot (this, &IBusFrontEnd::panel_slot_lookup_table_page_up));
    m_panel_client.signal_connect_lookup_table_page_down        (slot (this, &IBusFrontEnd::panel_slot_lookup_table_page_down));
    m_panel_client.signal_connect_trigger_property              (slot (this, &IBusFrontEnd::panel_slot_trigger_property));
    m_panel_client.signal_connect_process_helper_event          (slot (this, &IBusFrontEnd::panel_slot_process_helper_event));
    m_panel_client.signal_connect_move_preedit_caret            (slot (this, &IBusFrontEnd::panel_slot_move_preedit_caret));
    m_panel_client.signal_connect_select_candidate              (slot (this, &IBusFrontEnd::panel_slot_select_candidate));
    m_panel_client.signal_connect_process_key_event             (slot (this, &IBusFrontEnd::panel_slot_process_key_event));
    m_panel_client.signal_connect_commit_string                 (slot (this, &IBusFrontEnd::panel_slot_commit_string));
    m_panel_client.signal_connect_forward_key_event             (slot (this, &IBusFrontEnd::panel_slot_forward_key_event));
    m_panel_client.signal_connect_request_help                  (slot (this, &IBusFrontEnd::panel_slot_request_help));
    m_panel_client.signal_connect_request_factory_menu          (slot (this, &IBusFrontEnd::panel_slot_request_factory_menu));
    m_panel_client.signal_connect_change_factory                (slot (this, &IBusFrontEnd::panel_slot_change_factory));
}

IBusFrontEnd::~IBusFrontEnd ()
{
    log_func ();

    SCIM_DEBUG_FRONTEND (2) << " Destructing IBusFrontEnd object...\n";

//    if (m_obj_mngr_slot) {
//        sd_bus_slot_unref (m_obj_mngr_slot);
//    }
//
//    if (m_ctx_enum_slot) {
//        sd_bus_slot_unref (m_ctx_enum_slot);
//    }
//
//    if (m_service_slot) {
//        sd_bus_slot_unref (m_service_slot);
//    }
//
//    if (m_inputcontext_slot) {
//        sd_bus_slot_unref (m_inputcontext_slot);
//    }

    panel_disconnect ();

    if (m_portal_slot) {
        sd_bus_slot_unref (m_portal_slot);
    }

    if (m_bus) {
        sd_bus_detach_event (m_bus);
        sd_bus_unref (m_bus);
    }

    if (m_loop) {
        sd_event_unref (m_loop);
    }

//    if (m_socket_server.is_running ())
//        m_socket_server.shutdown ();
}

void
IBusFrontEnd::show_preedit_string (int siid)
{
    log_func ();

    signal_ctx (siid, "ShowPreeditText");

//    if (m_current_instance == id)
//        m_send_trans.put_command (SCIM_TRANS_CMD_SHOW_PREEDIT_STRING);
}

void
IBusFrontEnd::show_aux_string (int siid)
{
    log_func ();

    signal_ctx (siid, "ShowAuxiliaryText");

//    if (m_current_instance == id)
//        m_send_trans.put_command (SCIM_TRANS_CMD_SHOW_AUX_STRING);
}

void
IBusFrontEnd::show_lookup_table (int siid)
{
    log_func ();

    signal_ctx (siid, "ShowLookupTable");

//    if (m_current_instance == id)
//        m_send_trans.put_command (SCIM_TRANS_CMD_SHOW_LOOKUP_TABLE);
}

void
IBusFrontEnd::hide_preedit_string (int siid)
{
    log_func ();

    signal_ctx (siid, "HidePreeditText");
}

void
IBusFrontEnd::hide_aux_string (int siid)
{
    log_func ();

    signal_ctx (siid, "HideAuxiliaryText");

//    if (m_current_instance == id)
//        m_send_trans.put_command (SCIM_TRANS_CMD_HIDE_AUX_STRING);
}

void
IBusFrontEnd::hide_lookup_table (int siid)
{
    log_func ();

    signal_ctx (siid, "HideLookupTable");

//    if (m_current_instance == id)
//        m_send_trans.put_command (SCIM_TRANS_CMD_HIDE_LOOKUP_TABLE);
}

void
IBusFrontEnd::update_preedit_caret (int siid, int caret)
{
    log_func_not_impl ();

//    if (m_current_instance != id) {
//        return;
//    }
//
//    char path [IBUS_INPUTCONTEXT_OBJECT_PATH_BUF_SIZE];
//    ctx_gen_object_path (id, path, sizeof (path));
//
//    int r;
//    if ((r = sd_bus_emit_signal (m_bus,
//                                 path,
//                                 IBUS_INPUTCONTEXT_INTERFACE,
//                                 "HideLookupTable",
//                                 NULL)) < 0) {
//        log_warn ("unabled to emit HideLookupTable signal: %s",
//                  strerror (r));
//    }

//    if (m_current_instance == id) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_PREEDIT_CARET);
//        m_send_trans.put_data ((uint32) caret);
//    }
}

void
IBusFrontEnd::update_preedit_string (int siid,
                                       const WideString & str,
                                       const AttributeList & attrs)
{
    log_func ();

    signal_ctx (siid, "UpdatePreeditText", &str, &attrs);

//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_PREEDIT_STRING);
//        m_send_trans.put_data (str);
//        m_send_trans.put_data (attrs);
//    }
}

void
IBusFrontEnd::update_aux_string (int siid,
                                 const WideString & str,
                                 const AttributeList & attrs)
{
    log_func ();

    signal_ctx (siid, "UpdateAuxiliaryText", &str, &attrs);

//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_AUX_STRING);
//        m_send_trans.put_data (str);
//        m_send_trans.put_data (attrs);
//    }
}

void
IBusFrontEnd::commit_string (int siid, const WideString & str)
{
    log_func ();

    signal_ctx (siid, "CommitText", &str);

//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_COMMIT_STRING);
//        m_send_trans.put_data (str);
//    }
}

void
IBusFrontEnd::forward_key_event (int siid, const KeyEvent & key)
{
    log_func ();

    signal_ctx (siid, "ForwardKeyEvent", &key);

//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_FORWARD_KEY_EVENT);
//        m_send_trans.put_data (key);
//    }
}

void
IBusFrontEnd::update_lookup_table (int siid, const LookupTable & table)
{
    log_func ();

    signal_ctx (siid, "UpdateLookupTable", &table);

//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE);
//        m_send_trans.put_data (table);
//    }
}

void
IBusFrontEnd::register_properties (int siid, const PropertyList &properties)
{
    log_func();

    PropertyList::const_iterator it = properties.begin();
    for (; it != properties.end (); it ++) {
        log_debug ("Prop key=%s, label=%s, icon=%s, tip=%s, %s, %s",
                it->get_key ().c_str (),
                it->get_label ().c_str (),
                it->get_icon ().c_str (),
                it->get_tip ().c_str (),
                it->active () ? "active" : "inactive",
                it->visible () ? "visible" : "invisible");
    }

    signal_ctx (siid, "RegisterProperties", &properties);

//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_REGISTER_PROPERTIES);
//        m_send_trans.put_data (properties);
//    }
}

void
IBusFrontEnd::update_property (int siid, const Property &property)
{
    log_func();

    log_debug ("Prop key=%s, label=%s, icon=%s, tip=%s, %s, %s",
            property.get_key ().c_str (),
            property.get_label ().c_str (),
            property.get_icon ().c_str (),
            property.get_tip ().c_str (),
            property.active () ? "active" : "inactive",
            property.visible () ? "visible" : "invisible");

    signal_ctx (siid, "UpdateProperty", &property);

//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_PROPERTY);
//        m_send_trans.put_data (property);
//    }
}

void
IBusFrontEnd::beep (int siid)
{
    log_func_not_impl ();

//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_BEEP);
//    }
}

void
IBusFrontEnd::start_helper (int siid, const String &helper_uuid)
{
    log_func_not_impl ();

//    SCIM_DEBUG_FRONTEND (2) << "start_helper (" << helper_uuid << ")\n";
//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_START_HELPER);
//        m_send_trans.put_data (helper_uuid);
//    }
}

void
IBusFrontEnd::stop_helper (int siid, const String &helper_uuid)
{
    log_func_not_impl ();

//    SCIM_DEBUG_FRONTEND (2) << "stop_helper (" << helper_uuid << ")\n";
//
//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_STOP_HELPER);
//        m_send_trans.put_data (helper_uuid);
//    }
}

void
IBusFrontEnd::send_helper_event (int siid, const String &helper_uuid, const Transaction &trans)
{
    log_func_not_impl ();

//    if (m_current_instance == siid) {
//        m_send_trans.put_command (SCIM_TRANS_CMD_SEND_HELPER_EVENT);
//        m_send_trans.put_data (helper_uuid);
//        m_send_trans.put_data (trans);
//    }
}

bool
IBusFrontEnd::get_surrounding_text (int siid, WideString &text, int &cursor, int maxlen_before, int maxlen_after)
{
    log_func_not_impl (false);

//    text.clear ();
//    cursor = 0;
//
//    if (m_current_instance == siid && m_current_socket_client >= 0 && (maxlen_before != 0 || maxlen_after != 0)) {
//        if (maxlen_before < 0) maxlen_before = -1;
//        if (maxlen_after < 0) maxlen_after = -1;
//
//        m_temp_trans.clear ();
//        m_temp_trans.put_command (SCIM_TRANS_CMD_REPLY);
//        m_temp_trans.put_command (SCIM_TRANS_CMD_GET_SURROUNDING_TEXT);
//        m_temp_trans.put_data ((uint32) maxlen_before);
//        m_temp_trans.put_data ((uint32) maxlen_after);
//
//        Socket socket_client (m_current_socket_client);
//
//        if (m_temp_trans.write_to_socket (socket_client) &&
//            m_temp_trans.read_from_socket (socket_client, m_socket_timeout)) {
//
//            int cmd;
//            uint32 key;
//            uint32 cur;
//
//            if (m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REQUEST &&
//                m_temp_trans.get_data (key) && key == m_current_socket_client_key &&
//                m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_GET_SURROUNDING_TEXT &&
//                m_temp_trans.get_data (text) && m_temp_trans.get_data (cur)) {
//                cursor = (int) cur;
//                return true;
//            }
//        }
//    }
//    return false;
}

bool
IBusFrontEnd::delete_surrounding_text (int siid, int offset, int len)
{
    log_func_not_impl (false);

//    if (m_current_instance == siid && m_current_socket_client >= 0 && len > 0) {
//        m_temp_trans.clear ();
//        m_temp_trans.put_command (SCIM_TRANS_CMD_REPLY);
//        m_temp_trans.put_command (SCIM_TRANS_CMD_DELETE_SURROUNDING_TEXT);
//        m_temp_trans.put_data ((uint32) offset);
//        m_temp_trans.put_data ((uint32) len);
//
//        Socket socket_client (m_current_socket_client);
//
//        if (m_temp_trans.write_to_socket (socket_client) &&
//            m_temp_trans.read_from_socket (socket_client, m_socket_timeout)) {
//
//            int cmd;
//            uint32 key;
//
//            if (m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REQUEST &&
//                m_temp_trans.get_data (key) && key == m_current_socket_client_key &&
//                m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_DELETE_SURROUNDING_TEXT &&
//                m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
//                return true;
//        }
//    }
//    return false;
}

void
IBusFrontEnd::init (int argc, char **argv)
{
    log_func();

    int max_clients = -1;

    reload_config_callback (m_config);

    if (!m_config.null ()) {
//        String str;
//
//        m_config_readonly = m_config->read (String (SCIM_CONFIG_FRONTEND_IBUS_CONFIG_READONLY), false);
//
//        max_clients = m_config->read (String (SCIM_CONFIG_FRONTEND_IBUS_MAXCLIENTS), -1);

        m_config->signal_connect_reload (slot (this, &IBusFrontEnd::reload_config_callback));
    }
//    } else {
//        m_config_readonly = false;
//        max_clients = -1;
//    }

//    if (!m_socket_server.create (scim_get_default_socket_frontend_address ()))
//        throw FrontEndError ("IBusFrontEnd -- Cannot create SocketServer.");
//
//    m_socket_server.set_max_clients (max_clients);
//
//    m_socket_server.signal_connect_accept (
//        slot (this, &IBusFrontEnd::ibus_accept_callback));
//
//    m_socket_server.signal_connect_receive (
//        slot (this, &IBusFrontEnd::ibus_receive_callback));
//
//    m_socket_server.signal_connect_exception(
//        slot (this, &IBusFrontEnd::ibus_exception_callback));

    if (argv && argc > 1) {
        for (int i = 1; i < argc && argv [i]; ++i) {
            if (String ("--no-stay") == argv [i])
                m_stay = false;
        }
    }

    if (sd_event_new (&m_loop) < 0) {
        throw FrontEndError ("IBusFrontEnd -- Cannot create event loop.");
    }

    if (sd_bus_open_user (&m_bus) < 0) {
        throw FrontEndError ("IBusFrontEnd -- Cannot connect to session bus.");
    }

    if (sd_bus_attach_event (m_bus, m_loop, SD_EVENT_PRIORITY_NORMAL) < 0) {
        throw FrontEndError ("IBusFrontEnd -- Cannot attach bus to loop.");
    }

    if (sd_bus_request_name (m_bus,
                             IBUS_PORTAL_SERVICE,
                             SD_BUS_NAME_ALLOW_REPLACEMENT |
                                 SD_BUS_NAME_REPLACE_EXISTING) < 0) {
        throw FrontEndError (String ("IBus -- failed to aquire service name!"));
    }

    if (sd_bus_add_object_vtable (m_bus,
                                  &m_portal_slot,
                                  IBUS_PORTAL_OBJECT_PATH,
                                  IBUS_PORTAL_INTERFACE,
                                  m_portal_vtbl,
                                  this) < 0) {
        throw FrontEndError (String ("IBus -- failed to add portal object!"));
    }

    if (panel_connect() < 0) {
        throw FrontEndError (String ("IBus -- failed to connect to the panel daemon!"));
    }
}

void
IBusFrontEnd::run ()
{
    log_func();

    if (m_loop) {
        sd_event_loop (m_loop);
    }
//    if (m_socket_server.valid ())
//        m_socket_server.run ();
}

int
IBusFrontEnd::generate_ctx_id ()
{
    return ++ m_ctx_counter;
}

bool
IBusFrontEnd::check_client_connection (const Socket &client) const
{
    log_func_not_impl (false);

//    SCIM_DEBUG_FRONTEND (1) << "check_client_connection (" << client.get_id () << ").\n";
//
//    unsigned char buf [sizeof(uint32)];
//
//    int nbytes = client.read_with_timeout (buf, sizeof(uint32), m_socket_timeout);
//
//    if (nbytes == sizeof (uint32))
//        return true;
//
//    if (nbytes < 0) {
//        SCIM_DEBUG_FRONTEND (2) << " Error occurred when reading ibus (" << client.get_id ()
//            << "):" << client.get_error_message () << "\n";
//    } else {
//        SCIM_DEBUG_FRONTEND (2) << " Timeout when reading ibus (" << client.get_id ()
//            << ").\n";
//    }
//
//    return false;
}

void
IBusFrontEnd::ibus_accept_callback (SocketServer *server, const Socket &client)
{
    log_func_not_impl ();

//    SCIM_DEBUG_FRONTEND (1) << "ibus_accept_callback (" << client.get_id () << ").\n";
}

void
IBusFrontEnd::ibus_receive_callback (SocketServer *server, const Socket &client)
{
    log_func_not_impl ();

//    int id = client.get_id ();
//    int cmd;
//    uint32 key;
//
//    ClientInfo client_info;
//
//    SCIM_DEBUG_FRONTEND (1) << "ibus_receive_callback (" << id << ").\n";
//
//    // Check if the client is closed.
//    if (!check_client_connection (client)) {
//        SCIM_DEBUG_FRONTEND (2) << " closing client connection.\n";
//        ibus_close_connection (server, client);
//        return;
//    }
//
//    client_info = ibus_get_client_info (client);
//
//    // If it's a new client, then request to open the connection first.
//    if (client_info.type == UNKNOWN_CLIENT) {
//        ibus_open_connection (server, client);
//        return;
//    }
//
//    // If can not read the transaction,
//    // or the transaction is not started with SCIM_TRANS_CMD_REQUEST,
//    // or the key is mismatch,
//    // just return.
//    if (!m_receive_trans.read_from_socket (client, m_socket_timeout) ||
//        !m_receive_trans.get_command (cmd) || cmd != SCIM_TRANS_CMD_REQUEST ||
//        !m_receive_trans.get_data (key) || key != (uint32) client_info.key)
//        return;
//
//    m_current_socket_client     = id;
//    m_current_socket_client_key = key;
//
//    m_send_trans.clear ();
//    m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
//
//    // Move the read ptr to the end.
//    m_send_trans.get_command (cmd);
//
//    while (m_receive_trans.get_command (cmd)) {
//        if (cmd == SCIM_TRANS_CMD_PROCESS_KEY_EVENT)
//            ibus_process_key_event (id);
//        else if (cmd == SCIM_TRANS_CMD_MOVE_PREEDIT_CARET)
//            ibus_move_preedit_caret (id);
//        else if (cmd == SCIM_TRANS_CMD_SELECT_CANDIDATE)
//            ibus_select_candidate (id);
//        else if (cmd == SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE_PAGE_SIZE)
//            ibus_update_lookup_table_page_size (id);
//        else if (cmd == SCIM_TRANS_CMD_LOOKUP_TABLE_PAGE_UP)
//            ibus_lookup_table_page_up (id);
//        else if (cmd == SCIM_TRANS_CMD_LOOKUP_TABLE_PAGE_DOWN)
//            ibus_lookup_table_page_down (id);
//        else if (cmd == SCIM_TRANS_CMD_RESET)
//            ibus_reset (id);
//        else if (cmd == SCIM_TRANS_CMD_FOCUS_IN)
//            ibus_focus_in (id);
//        else if (cmd == SCIM_TRANS_CMD_FOCUS_OUT)
//            ibus_focus_out (id);
//        else if (cmd == SCIM_TRANS_CMD_TRIGGER_PROPERTY)
//            ibus_trigger_property (id);
//        else if (cmd == SCIM_TRANS_CMD_PROCESS_HELPER_EVENT)
//            ibus_process_helper_event (id);
//        else if (cmd == SCIM_TRANS_CMD_UPDATE_CLIENT_CAPABILITIES)
//            ibus_update_client_capabilities (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_LIST)
//            ibus_get_factory_list (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_NAME)
//            ibus_get_factory_name (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_AUTHORS)
//            ibus_get_factory_authors (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_CREDITS)
//            ibus_get_factory_credits (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_HELP)
//            ibus_get_factory_help (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_LOCALES)
//            ibus_get_factory_locales (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_ICON_FILE)
//            ibus_get_factory_icon_file (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_LANGUAGE)
//            ibus_get_factory_language (id);
//        else if (cmd == SCIM_TRANS_CMD_NEW_INSTANCE)
//            ibus_new_instance (id);
//        else if (cmd == SCIM_TRANS_CMD_DELETE_INSTANCE)
//            ibus_delete_instance (id);
//        else if (cmd == SCIM_TRANS_CMD_DELETE_ALL_INSTANCES)
//            ibus_delete_all_instances (id);
//        else if (cmd == SCIM_TRANS_CMD_FLUSH_CONFIG)
//            ibus_flush_config (id);
//        else if (cmd == SCIM_TRANS_CMD_ERASE_CONFIG)
//            ibus_erase_config (id);
//        else if (cmd == SCIM_TRANS_CMD_RELOAD_CONFIG)
//            ibus_reload_config (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_STRING)
//            ibus_get_config_string (id);
//        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_STRING)
//            ibus_set_config_string (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_INT)
//            ibus_get_config_int (id);
//        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_INT)
//            ibus_set_config_int (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_BOOL)
//            ibus_get_config_bool (id);
//        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_BOOL)
//            ibus_set_config_bool (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_DOUBLE)
//            ibus_get_config_double (id);
//        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_DOUBLE)
//            ibus_set_config_double (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_VECTOR_STRING)
//            ibus_get_config_vector_string (id);
//        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_VECTOR_STRING)
//            ibus_set_config_vector_string (id);
//        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_VECTOR_INT)
//            ibus_get_config_vector_int (id);
//        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_VECTOR_INT)
//            ibus_set_config_vector_int (id);
//        else if (cmd == SCIM_TRANS_CMD_LOAD_FILE)
//            ibus_load_file (id);
//        else if (cmd == SCIM_TRANS_CMD_CLOSE_CONNECTION) {
//            ibus_close_connection (server, client);
//            m_current_socket_client     = -1;
//            m_current_socket_client_key = 0;
//            return;
//        }
//    }
//
//    // Send reply to client
//    if (m_send_trans.get_data_type () == SCIM_TRANS_DATA_UNKNOWN)
//        m_send_trans.put_command (SCIM_TRANS_CMD_FAIL);
//
//    m_send_trans.write_to_socket (client);
//
//    m_current_socket_client     = -1;
//    m_current_socket_client_key = 0;
//
//    SCIM_DEBUG_FRONTEND (1) << "End of ibus_receive_callback (" << id << ").\n";
}

bool
IBusFrontEnd::ibus_open_connection (SocketServer *server, const Socket &client)
{
    log_func_not_impl (false);

//    SCIM_DEBUG_FRONTEND (2) << " Open ibus connection for client " << client.get_id () << "  number of clients=" << m_socket_client_repository.size () << ".\n";
//
//    uint32 key;
//    String type = scim_socket_accept_connection (key,
//                                                 String ("IBusFrontEnd"), 
//                                                 String ("IBusIMEngine,IBusConfig"),
//                                                 client,
//                                                 m_socket_timeout);
//
//    if (type.length ()) {
//        ClientInfo info;
//        info.key = key;
//        info.type = ((type == "IBusIMEngine") ? IMENGINE_CLIENT : CONFIG_CLIENT);
//
//        SCIM_DEBUG_MAIN (2) << " Add client to repository. Type=" << type << " key=" << key << "\n";
//        m_socket_client_repository [client.get_id ()] = info;
//        return true;
//    }
//
//    // Client did not pass the registration process, close it.
//    SCIM_DEBUG_FRONTEND (2) << " Failed to create new connection.\n"; 
//    server->close_connection (client);
//    return false;
}

void
IBusFrontEnd::ibus_close_connection (SocketServer *server, const Socket &client)
{
    log_func_not_impl ();

//    SCIM_DEBUG_FRONTEND (2) << " Close client connection " << client.get_id () << "  number of clients=" << m_socket_client_repository.size () << ".\n";
//
//    ClientInfo client_info = ibus_get_client_info (client);
//
//    server->close_connection (client);
//
//    if (client_info.type != UNKNOWN_CLIENT) {
//        m_socket_client_repository.erase (client.get_id ());
//
//        if (client_info.type == IMENGINE_CLIENT)
//            ibus_delete_all_instances (client.get_id ());
//
//        if (!m_socket_client_repository.size () && !m_stay)
//            server->shutdown ();
//    }
}

IBusFrontEnd::ClientInfo
IBusFrontEnd::ibus_get_client_info (const Socket &client)
{
    static ClientInfo null_client = { 0, UNKNOWN_CLIENT };

    log_func_not_impl (null_client);

//    IBusClientRepository::iterator it = m_socket_client_repository.find (client.get_id ());
//
//    if (it != m_socket_client_repository.end ())
//        return it->second;
//
//    return null_client;
}

void
IBusFrontEnd::ibus_exception_callback (SocketServer *server, const Socket &client)
{
    log_func_not_impl ();

//    SCIM_DEBUG_FRONTEND (1) << "ibus_exception_callback (" << client.get_id () << ").\n";
//
//    ibus_close_connection (server, client);
}

//client_id is client's ibus id
void
IBusFrontEnd::ibus_get_factory_list (int /*client_id*/)
{
    log_func_not_impl ();

//    String encoding;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_list.\n";
//
//    if (m_receive_trans.get_data (encoding)) {
//        std::vector<String> uuids;
//
//        get_factory_list_for_encoding (uuids, encoding);
//
//        SCIM_DEBUG_FRONTEND (3) << "  Encoding (" << encoding
//            << ") Num(" << uuids.size () << ").\n";
//
//        m_send_trans.put_data (uuids);
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_factory_name (int /*client_id*/)
{
    log_func_not_impl ();

//    String sfid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_name.\n";
//
//    if (m_receive_trans.get_data (sfid)) {
//        WideString name = get_factory_name (sfid);
//
//        m_send_trans.put_data (name);
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_factory_authors (int /*client_id*/)
{
    log_func_not_impl ();

//    String sfid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_authors.\n";
//
//    if (m_receive_trans.get_data (sfid)) {
//        WideString authors = get_factory_authors (sfid);
//
//        m_send_trans.put_data (authors);
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_factory_credits (int /*client_id*/)
{
    log_func_not_impl ();

//    String sfid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_credits.\n";
//
//    if (m_receive_trans.get_data (sfid)) {
//        WideString credits = get_factory_credits (sfid);
//
//        m_send_trans.put_data (credits);
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_factory_help (int /*client_id*/)
{
    log_func_not_impl ();

//    String sfid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_help.\n";
//
//    if (m_receive_trans.get_data (sfid)) {
//        WideString help = get_factory_help (sfid);
//
//        m_send_trans.put_data (help);
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_factory_locales (int /*client_id*/)
{
    log_func_not_impl ();

//    String sfid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_locales.\n";
//
//    if (m_receive_trans.get_data (sfid)) {
//        String locales = get_factory_locales (sfid);
//
//        SCIM_DEBUG_FRONTEND (3) << "  Locales (" << locales << ").\n";
//
//        m_send_trans.put_data (locales);
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_factory_icon_file (int /*client_id*/)
{
    log_func_not_impl ();

//    String sfid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_icon_file.\n";
//
//    if (m_receive_trans.get_data (sfid)) {
//        String iconfile = get_factory_icon_file (sfid);
//
//        SCIM_DEBUG_FRONTEND (3) << "  ICON File (" << iconfile << ").\n";
//
//        m_send_trans.put_data (iconfile);
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_factory_language (int /*client_id*/)
{
    log_func_not_impl ();

//    String sfid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_language.\n";
//
//    if (m_receive_trans.get_data (sfid)) {
//        String language = get_factory_language (sfid);
//
//        SCIM_DEBUG_FRONTEND (3) << "  Language (" << language << ").\n";
//
//        m_send_trans.put_data (language);
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_new_instance (int client_id)
{
    log_func_not_impl ();

//    String sfid;
//    String encoding;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_new_instance.\n";
//
//    if (m_receive_trans.get_data (sfid) &&
//        m_receive_trans.get_data (encoding)) {
//        int siid = new_instance (sfid, encoding);
//
//        // Instance created OK.
//        if (siid >= 0) {
//            IBusInstanceRepository::iterator it =
//                std::lower_bound (m_socket_instance_repository.begin (),
//                                  m_socket_instance_repository.end (),
//                                  std::pair <int, int> (client_id, siid));
//
//            if (it == m_socket_instance_repository.end ())
//                m_socket_instance_repository.push_back (std::pair <int, int> (client_id, siid));
//            else
//                m_socket_instance_repository.insert (it, std::pair <int, int> (client_id, siid));
//
//            SCIM_DEBUG_FRONTEND (3) << "  InstanceID (" << siid << ").\n";
//
//            m_send_trans.put_data ((uint32)siid);
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//        }
//    }
}

void
IBusFrontEnd::ibus_delete_instance (int client_id)
{
    log_func_not_impl ();

//    uint32 siid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_delete_instance.\n";
//
//    if (m_receive_trans.get_data (siid)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  InstanceID (" << siid << ").\n";
//
//        m_current_instance = (int) siid;
//
//        delete_instance ((int) siid);
//
//        m_current_instance = -1;
//
//        IBusInstanceRepository::iterator it =
//            std::lower_bound (m_socket_instance_repository.begin (),
//                              m_socket_instance_repository.end (),
//                              std::pair <int, int> (client_id, siid));
//
//        if (it != m_socket_instance_repository.end () &&
//            *it == std::pair <int, int> (client_id, siid))
//            m_socket_instance_repository.erase (it);
//
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_delete_all_instances (int client_id)
{
    log_func_not_impl ();

//    SCIM_DEBUG_FRONTEND (2) << " ibus_delete_all_instances.\n";
//
//    IBusInstanceRepository::iterator it;
//
//    IBusInstanceRepository::iterator lit =
//        std::lower_bound (m_socket_instance_repository.begin (),
//                          m_socket_instance_repository.end (),
//                          std::pair <int, int> (client_id, 0));
//
//    IBusInstanceRepository::iterator uit =
//        std::upper_bound (m_socket_instance_repository.begin (),
//                          m_socket_instance_repository.end (),
//                          std::pair <int, int> (client_id, INT_MAX));
//
//    if (lit != uit) {
//        for (it = lit; it != uit; ++it) {
//            m_current_instance = it->second;
//            delete_instance (it->second);
//        }
//        m_current_instance = -1;
//        m_socket_instance_repository.erase (lit, uit);
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_process_key_event (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32   siid;
//    KeyEvent event;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_process_key_event.\n";
//
//    if (m_receive_trans.get_data (siid) &&
//        m_receive_trans.get_data (event)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ") KeyEvent ("
//            << event.code << "," << event.mask << ").\n";
//
//        m_current_instance = (int) siid;
//
//        if (process_key_event ((int) siid, event))
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//        else
//            m_send_trans.put_command (SCIM_TRANS_CMD_FAIL);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_move_preedit_caret (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//    uint32 caret;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_move_preedit_caret.\n";
//
//    if (m_receive_trans.get_data (siid) &&
//        m_receive_trans.get_data (caret)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid
//            << ") Caret (" << caret << ").\n";
//
//        m_current_instance = (int) siid;
//
//        move_preedit_caret ((int) siid, caret); 
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_select_candidate (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//    uint32 item;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_select_candidate.\n";
//
//    if (m_receive_trans.get_data (siid) &&
//        m_receive_trans.get_data (item)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ") Item (" << item << ").\n";
//
//        m_current_instance = (int) siid;
//
//        select_candidate ((int) siid, item); 
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_update_lookup_table_page_size (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//    uint32 size;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_update_lookup_table_page_size.\n";
//
//    if (m_receive_trans.get_data (siid) &&
//        m_receive_trans.get_data (size)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ") PageSize (" << size << ").\n";
//
//        m_current_instance = (int) siid;
//
//        update_lookup_table_page_size ((int) siid, size); 
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_lookup_table_page_up (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_lookup_table_page_up.\n";
//
//    if (m_receive_trans.get_data (siid)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";
//
//        m_current_instance = (int) siid;
//
//        lookup_table_page_up ((int) siid); 
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_lookup_table_page_down (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_lookup_table_page_down.\n";
//
//    if (m_receive_trans.get_data (siid)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";
//
//        m_current_instance = (int) siid;
//
//        lookup_table_page_down ((int) siid); 
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_reset (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_reset.\n";
//
//    if (m_receive_trans.get_data (siid)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";
//
//        m_current_instance = (int) siid;
//
//        reset ((int) siid); 
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_focus_in (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_focus_in.\n";
//
//    if (m_receive_trans.get_data (siid)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";
//
//        m_current_instance = (int) siid;
//
//        focus_in ((int) siid); 
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_focus_out (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_focus_out.\n";
//
//    if (m_receive_trans.get_data (siid)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";
//
//        m_current_instance = (int) siid;
//
//        focus_out ((int) siid); 
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_trigger_property (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//    String property;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_trigger_property.\n";
//
//    if (m_receive_trans.get_data (siid) &&
//        m_receive_trans.get_data (property)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";
//
//        m_current_instance = (int) siid;
//
//        trigger_property ((int) siid, property); 
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_process_helper_event (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//    String helper_uuid;
//    Transaction trans;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_process_helper_event.\n";
//
//    if (m_receive_trans.get_data (siid) &&
//        m_receive_trans.get_data (helper_uuid) &&
//        m_receive_trans.get_data (trans)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";
//
//        m_current_instance = (int) siid;
//
//        process_helper_event ((int) siid, helper_uuid, trans); 
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}

void
IBusFrontEnd::ibus_update_client_capabilities (int /*client_id*/)
{
    log_func_not_impl ();

//    uint32 siid;
//    uint32 cap;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_update_client_capabilities.\n";
//
//    if (m_receive_trans.get_data (siid) && m_receive_trans.get_data (cap)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";
//
//        m_current_instance = (int) siid;
//
//        update_client_capabilities ((int) siid, cap); 
//
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//
//        m_current_instance = -1;
//    }
}


void
IBusFrontEnd::ibus_flush_config (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config_readonly || m_config.null ())
//        return;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_flush_config.\n";
//
//    if (m_config->flush ())
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
}

void
IBusFrontEnd::ibus_erase_config (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config_readonly || m_config.null ())
//        return;
//
//    String key;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_erase_config.\n";
//
//    if (m_receive_trans.get_data (key)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key   (" << key << ").\n";
//
//        if (m_config->erase (key))
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_reload_config (int /*client_id*/)
{
    log_func_not_impl ();

//    static timeval last_timestamp = {0, 0};
//
//    if (m_config.null ())
//        return;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_reload_config.\n";
//
//    timeval timestamp;
//
//    gettimeofday (&timestamp, 0);
//
//    if (timestamp.tv_sec > last_timestamp.tv_sec + 1)
//        m_config->reload ();
//
//    gettimeofday (&last_timestamp, 0);
//
//    m_send_trans.put_command (SCIM_TRANS_CMD_OK);
}

void
IBusFrontEnd::ibus_get_config_string (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config.null ()) return;
//
//    String key;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_string.\n";
//
//    if (m_receive_trans.get_data (key)) {
//        String value;
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";
//
//        if (m_config->read (key, &value)) {
//            m_send_trans.put_data (value);
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//        }
//    }
}

void
IBusFrontEnd::ibus_set_config_string (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config_readonly || m_config.null ())
//        return;
//
//    String key;
//    String value;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_string.\n";
//
//    if (m_receive_trans.get_data (key) &&
//        m_receive_trans.get_data (value)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key   (" << key << ").\n";
//        SCIM_DEBUG_FRONTEND (3) << "  Value (" << value << ").\n";
//
//        if (m_config->write (key, value))
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_config_int (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config.null ()) return;
//
//    String key;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_int.\n";
//
//    if (m_receive_trans.get_data (key)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";
//
//        int value;
//        if (m_config->read (key, &value)) {
//            m_send_trans.put_data ((uint32) value);
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//        }
//    }
}

void
IBusFrontEnd::ibus_set_config_int (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config_readonly || m_config.null ())
//        return;
//
//    String key;
//    uint32 value;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_int.\n";
//
//    if (m_receive_trans.get_data (key) &&
//        m_receive_trans.get_data (value)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key   (" << key << ").\n";
//        SCIM_DEBUG_FRONTEND (3) << "  Value (" << value << ").\n";
//
//        if (m_config->write (key, (int) value))
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_config_bool (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config.null ()) return;
//
//    String key;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_bool.\n";
//
//    if (m_receive_trans.get_data (key)) {
//        bool value;
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";
//
//        if (m_config->read (key, &value)) {
//            m_send_trans.put_data ((uint32) value);
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//        }
//    }
}

void
IBusFrontEnd::ibus_set_config_bool (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config_readonly || m_config.null ())
//        return;
//
//    String key;
//    uint32 value;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_bool.\n";
//
//    if (m_receive_trans.get_data (key) &&
//        m_receive_trans.get_data (value)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key   (" << key << ").\n";
//        SCIM_DEBUG_FRONTEND (3) << "  Value (" << value << ").\n";
//
//        if (m_config->write (key, (bool) value))
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_config_double (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config.null ()) return;
//
//    String key;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_double.\n";
//
//    if (m_receive_trans.get_data (key)) {
//        double value;
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";
//
//        if (m_config->read (key, &value)) {
//            char buf [80];
//            snprintf (buf, 79, "%lE", value);
//            m_send_trans.put_data (String (buf));
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//        }
//    }
}

void
IBusFrontEnd::ibus_set_config_double (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config_readonly || m_config.null ())
//        return;
//
//    String key;
//    String str;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_double.\n";
//
//    if (m_receive_trans.get_data (key) &&
//        m_receive_trans.get_data (str)) {
//        double value;
//        sscanf (str.c_str (), "%lE", &value);
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key   (" << key << ").\n";
//        SCIM_DEBUG_FRONTEND (3) << "  Value (" << value << ").\n";
//
//        if (m_config->write (key, value))
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_config_vector_string (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config.null ()) return;
//
//    String key;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_vector_string.\n";
//
//    if (m_receive_trans.get_data (key)) {
//        std::vector <String> vec;
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";
//
//        if (m_config->read (key, &vec)) {
//            m_send_trans.put_data (vec);
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//        }
//    }
}

void
IBusFrontEnd::ibus_set_config_vector_string (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config_readonly || m_config.null ())
//        return;
//
//    String key;
//    std::vector<String> vec;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_vector_string.\n";
//
//    if (m_receive_trans.get_data (key) &&
//        m_receive_trans.get_data (vec)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";
//
//        if (m_config->write (key, vec))
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_get_config_vector_int (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config.null ()) return;
//
//    String key;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_vector_int.\n";
//
//    if (m_receive_trans.get_data (key)) {
//        std::vector <int> vec;
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";
//
//        if (m_config->read (key, &vec)) {
//            std::vector <uint32> reply;
//
//            for (uint32 i=0; i<vec.size (); ++i)
//                reply.push_back ((uint32) vec[i]);
//
//            m_send_trans.put_data (reply);
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//        }
//    }
}

void
IBusFrontEnd::ibus_set_config_vector_int (int /*client_id*/)
{
    log_func_not_impl ();

//    if (m_config_readonly || m_config.null ())
//        return;
//
//    String key;
//    std::vector<uint32> vec;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_vector_int.\n";
//
//    if (m_receive_trans.get_data (key) &&
//        m_receive_trans.get_data (vec)) {
//        std::vector<int> req;
//
//        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";
//
//        for (uint32 i=0; i<vec.size (); ++i)
//            req.push_back ((int) vec[i]);
//
//        if (m_config->write (key, req))
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }
}

void
IBusFrontEnd::ibus_load_file (int /*client_id*/)
{
    log_func_not_impl ();

//    String filename;
//    char *bufptr = 0;
//    size_t filesize = 0;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_load_file.\n";
//
//    if (m_receive_trans.get_data (filename)) {
//        SCIM_DEBUG_FRONTEND (3) << "  File (" << filename << ").\n";
//
//        if ((filesize = scim_load_file (filename, &bufptr)) > 0) {
//            m_send_trans.put_data (bufptr, filesize);
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//        }
//
//        delete [] bufptr;
//    }
}

void
IBusFrontEnd::reload_config_callback (const ConfigPointer &config)
{
    log_func();

//    SCIM_DEBUG_FRONTEND (1) << "Reload configuration.\n";
//
//    int max_clients = -1;
//
//    m_config_readonly = config->read (String (SCIM_CONFIG_FRONTEND_IBUS_CONFIG_READONLY), false);
//    max_clients = config->read (String (SCIM_CONFIG_FRONTEND_IBUS_MAXCLIENTS), -1);
//
//    m_socket_server.set_max_clients (max_clients);

    m_frontend_hotkey_matcher.load_hotkeys (config);
    m_imengine_hotkey_matcher.load_hotkeys (config);
}

/**
 * IBusFrontEnd::ibus_new_instance
 */
int IBusFrontEnd::portal_create_ctx(sd_bus_message *m)
{
    log_func();

    String locale = scim_get_current_locale ();
    String encoding = scim_get_locale_encoding (locale);
    String language = scim_get_current_language ();
    String sfid = get_default_factory (language, encoding);

    log_debug ("locale=%s", locale.c_str ());

    int siid = new_instance (sfid, encoding);

    int id = generate_ctx_id ();
    char path[IBUS_INPUTCONTEXT_OBJECT_PATH_BUF_SIZE];
    ctx_gen_object_path(id, path, sizeof(path));

    log_debug ("instance name=%s, authors=%s, encoding=%s, uuid=%s",
            utf8_wcstombs (get_instance_name (siid)).c_str(),
            utf8_wcstombs (get_instance_authors (siid)).c_str(),
            get_instance_encoding (siid).c_str (),
            get_instance_uuid (siid).c_str ());

    int r;

    IBusCtx *ctx = new IBusCtx (locale, id, siid);
    if ((r = ctx->init (m_bus, path))) {
        return r;
    }

    log_info("new ctx created %s", path);

    m_panel_client.prepare (id);
    m_panel_client.register_input_context (id, get_instance_uuid (siid));
    m_panel_client.send ();

    m_id_ctx_map[id] = ctx;
    m_siid_ctx_map[siid] = ctx;

    if ((r = sd_bus_reply_method_return (m, "o", path)) < 0) {
        delete ctx;
    }

    return r;
}

int IBusFrontEnd::srv_destroy(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_not_impl (-ENOSYS);

    if (m_id_ctx_map.find (ctx->id ()) != m_id_ctx_map.end ())  {
        m_id_ctx_map.erase (ctx->id ());
    }

//    ibus_close_connection (server, client);
//    =====
//    if (client_info.type != UNKNOWN_CLIENT) {
//        m_socket_client_repository.erase (client.get_id ());
//
//        if (client_info.type == IMENGINE_CLIENT)
//            ibus_delete_all_instances (client.get_id ());
//
//        if (!m_socket_client_repository.size () && !m_stay)
//            server->shutdown ();
//    }
//    =====
//    if (lit != uit) {
//        for (it = lit; it != uit; ++it) {
//            m_current_instance = it->second;
//            delete_instance (it->second);
//        }
        m_current_instance = -1;
//        m_socket_instance_repository.erase (lit, uit);
//        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//    }

    if (ctx == m_current_ibus_ctx) {
        m_current_ibus_ctx = NULL;
    }
//    m_current_socket_client     = -1;
//    m_current_socket_client_key = 0;
}

int IBusFrontEnd::ctx_process_key_event(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    uint32_t keyval = 0;
    uint32_t keycode = 0;
    uint32_t state = 0;

    if (sd_bus_message_read(m, "uuu", &keyval, &keycode, &state) < 0) {
        return -1;
    }

    KeyEvent event = scim_ibus_keyevent_to_scim_keyevent (ctx->keyboard_layout (),
                                                          keyval,
                                                          keycode,
                                                          state);

    int siid = ctx->siid ();

    m_current_instance = siid;
    m_panel_client.prepare (ctx->id ());
    bool consumed = filter_hotkeys (ctx, event)
                    ? true
                    : ctx->is_on() && process_key_event (siid, event)
                        ? true
                        : false;
//    else {
//        //IMForwardEvent (ims, (XPointer) call_data);
//    }
    m_panel_client.send ();
    m_current_instance = -1;

    String eventstr;
    scim_key_to_string (eventstr, event);
    log_debug ("%s %s",
               eventstr.c_str (),
               consumed ? "consumed" : "ignored");

    return sd_bus_reply_method_return (m, "b", consumed);

//    uint32   siid;
//    KeyEvent event;
//
//    SCIM_DEBUG_FRONTEND (2) << " ibus_process_key_event.\n";
//
//    if (m_receive_trans.get_data (siid) &&
//        m_receive_trans.get_data (event)) {
//
//        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ") KeyEvent ("
//            << event.code << "," << event.mask << ").\n";
//
//        m_current_instance = (int) siid;
//
//        if (process_key_event ((int) siid, event))
//            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
//        else
//            m_send_trans.put_command (SCIM_TRANS_CMD_FAIL);
//
//        m_current_instance = -1;
//    }
}

/**
 * @i 306
 * @i 211
 * @i 0
 * @i 18
 */
int IBusFrontEnd::ctx_set_cursor_location(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    int r;
    if ((r = ctx->cursor_location_from_message (m)) < 0) {
        return r;
    }

    m_panel_client.prepare (ctx->id ());
    panel_req_update_spot_location (ctx);
    m_panel_client.send ();

    return 0;
}

int IBusFrontEnd::ctx_set_cursor_location_relative(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    int r;
    if ((r = ctx->cursor_location_relative_from_message (m)) < 0) {
        return r;
    }

    m_panel_client.prepare (ctx->id ());
    panel_req_update_spot_location (ctx);
    m_panel_client.send ();

    return 0;
}


int IBusFrontEnd::ctx_process_hand_writing_event(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_cancel_hand_writing(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_property_activate(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_set_engine(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_get_engine(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_set_surrounding_text(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_focus_in(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    uint32 siid = ctx->siid();

    m_current_instance = siid;

    focus_in (siid); 

    m_panel_client.prepare (ctx->id ());
    panel_req_focus_in (ctx);
    m_panel_client.send ();

    m_current_instance = -1;

    m_current_ibus_ctx = ctx;

    return 0;
}

int IBusFrontEnd::ctx_focus_out(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    m_current_ibus_ctx = NULL;

    uint32 siid = ctx->siid ();

    m_current_instance = siid;

    focus_out (siid); 

    m_current_instance = -1;

    return 0;
}

int IBusFrontEnd::ctx_reset(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    uint32 siid = ctx->siid ();

    m_current_instance = siid;

    reset (siid); 

    m_current_instance = -1;

    return 0;
}

/**
 * IBusFrontEnd::ibus_update_client_capabilities
 */
int IBusFrontEnd::ctx_set_capabilities(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

//    SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";

    int r;
    if ((r = ctx->caps_from_message (m)) < 0) {
        return r;
    }

    int siid = ctx->siid ();

    m_current_instance = siid;
    update_client_capabilities (siid, ctx->scim_caps ()); 
    m_current_instance = -1;

    log_debug("IBus caps=%s => SCIM caps=%s",
              ibus_caps_to_str (ctx->caps()).c_str(),
              scim_caps_to_str (ctx->scim_caps()).c_str());

    return r;
}

int IBusFrontEnd::ctx_get_client_commit_preedit (IBusCtx *ctx, sd_bus_message *value)
{
    log_func ();

    return ctx->client_commit_preedit_to_message (value);
}

int IBusFrontEnd::ctx_set_client_commit_preedit (IBusCtx *ctx, sd_bus_message *value)
{
    log_func ();

    return ctx->client_commit_preedit_from_message (value);
}

int IBusFrontEnd::ctx_get_content_type (IBusCtx *ctx, sd_bus_message *value)
{
    log_func ();

    return ctx->content_type_to_message (value);
}

int IBusFrontEnd::ctx_set_content_type (IBusCtx *ctx, sd_bus_message *value)
{
    log_func ();

    return ctx->content_type_from_message (value);
}

IBusCtx *IBusFrontEnd::find_ctx_by_siid (int siid) const
{
    IBusIDCtxMap::const_iterator it = m_siid_ctx_map.find (siid);
    if (it == m_siid_ctx_map.end ()) {
        return NULL;
    }

    return it->second;
}

IBusCtx *IBusFrontEnd::find_ctx (int id) const
{
    IBusIDCtxMap::const_iterator it = m_id_ctx_map.find (id);
    if (it == m_id_ctx_map.end ()) {
        return NULL;
    }

    return it->second;
}

IBusCtx *IBusFrontEnd::find_ctx (const char *path) const
{
    if (strncmp (IBUS_INPUTCONTEXT_OBJECT_PATH,
                 path,
                 STRLEN(IBUS_INPUTCONTEXT_OBJECT_PATH))) {
        return NULL;
    }

    const char *id_str = strrchr (path + STRLEN(IBUS_INPUTCONTEXT_OBJECT_PATH),
                                  '_');
    if (!id_str) {
        return NULL;
    }

    int id = (int) strtol (id_str + 1, NULL, 10);
    if (!id) {
        // id start from 1
        return NULL;
    }

    return _scim_frontend->find_ctx (id);
}

inline bool IBusFrontEnd::validate_ctx (IBusCtx *ctx) const {
    return ctx != NULL &&
           m_id_ctx_map.find (ctx->id ()) != m_id_ctx_map.end ();
}

void IBusFrontEnd::start_ctx (IBusCtx *ctx)
{
    log_func ();

    if (validate_ctx (ctx)) {
//        if (m_xims_dynamic) {
//            IMPreeditStateStruct ips;
//            ips.major_code = 0;
//            ips.minor_code = 0;
//            ips.icid = ic->icid;
//            ips.connect_id = ic->connect_id;
//            IMPreeditStart (m_xims, (XPointer) & ips);
//        }

        ctx->on ();

        panel_req_update_screen (ctx);
        panel_req_update_spot_location (ctx);
        panel_req_update_factory_info (ctx);

        m_panel_client.turn_on (ctx->id ());
        m_panel_client.hide_preedit_string (ctx->id ());
        m_panel_client.hide_aux_string (ctx->id ());
        m_panel_client.hide_lookup_table (ctx->id ());

        if (ctx->shared_siid ()) reset (ctx->siid ());
        focus_in (ctx->siid ());
    }
}

void IBusFrontEnd::stop_ctx (IBusCtx *ctx)
{
    log_func ();

    if (validate_ctx (ctx)) {
        focus_out (ctx->siid ());
        if (ctx->shared_siid ()) reset (ctx->siid ());

//        if (ims_is_preedit_callback_mode (ic))
//            ims_preedit_callback_done (ic);

        panel_req_update_factory_info (ctx);
        m_panel_client.turn_off (ctx->id ());

//        if (m_xims_dynamic) {
//            IMPreeditStateStruct ips;
//            ips.major_code = 0;
//            ips.minor_code = 0;
//            ips.icid = ic->icid;
//            ips.connect_id = ic->connect_id;
//            IMPreeditEnd (m_xims, (XPointer) & ips);
//        }

        ctx->off ();
    }
}

static int fill_signal (sd_bus_message *m, va_list args)
{
    return 0;
}

/**
 * @v <@(sa{sv}sv) ("IBusText", {}, "\20013\25991\23383", <@(sa{sv}av) ("IBusAttrList", {}, [])>)>
 */
static int fill_commit_text_signal (sd_bus_message *m, va_list args)
{
    WideString &wstr = *va_arg (args, WideString *);
    String str = utf8_wcstombs (wstr);

    log_func ();

    log_debug ("commit text: '%s'", str.c_str());

    return sd_bus_message_append (m,
                                  "v", "(sa{sv}sv)",
                                  "IBusText",
                                  0,
                                  str.c_str(),
                                  "(sa{sv}av)",
                                      "IBusAttrList",
                                      0,
                                      0);

}

static int fill_forward_key_event_signal (sd_bus_message *m, va_list args)
{
    KeyEvent &event = *va_arg (args, KeyEvent *);

    log_func_not_impl (-ENOSYS);
}

/**
 * UpdatePreeditTextWithMode
 * 
 * @v <@(sa{sv}sv) ("IBusText", {}, "\12563", <@(sa{sv}av) ("IBusAttrList", {}, [<@(sa{sv}uuuu) ("IBusAttribute", {}, 1, 1, 0, 1)>, <@(sa{sv}uuuu) ("IBusAttribute", {}, 3, 13158640, 0, 1)>, <@(sa{sv}uuuu) ("IBusAttribute", {}, 2, 0, 0, 1)>])>)>
 * @u 0
 * @b True
 * @u 1
 *
 * @v <@(sa{sv}sv) ("IBusText", {}, "\12563\12584", <@(sa{sv}av) ("IBusAttrList", {}, [<@(sa{sv}uuuu) ("IBusAttribute", {}, 1, 1, 0, 2)>, <@(sa{sv}uuuu) ("IBusAttribute", {}, 3, 13158640, 0, 1)>, <@(sa{sv}uuuu) ("IBusAttribute", {}, 2, 0, 0, 1)>])>)>
 * @u 0
 * @b True
 * @u 1
 *
 * @v <@(sa{sv}sv) ("IBusText", {}, "\12563\12584\12581", <@(sa{sv}av) ("IBusAttrList", {}, [<@(sa{sv}uuuu) ("IBusAttribute", {}, 1, 1, 0, 3)>, <@(sa{sv}uuuu) ("IBusAttribute", {}, 3, 13158640, 0, 1)>, <@(sa{sv}uuuu) ("IBusAttribute", {}, 2, 0, 0, 1)>])>)>
 * @u 0
 * @b True
 * @u 1
 *
 * @v <@(sa{sv}sv) ("IBusText", {}, "\20013\25991\12567", <@(sa{sv}av) ("IBusAttrList", {}, [<@(sa{sv}uuuu) ("IBusAttribute", {}, 1, 1, 0, 3)>, <@(sa{sv}uuuu) ("IBusAttribute", {}, 1, 2, 0, 2)>, <@(sa{sv}uuuu) ("IBusAttribute", {}, 3, 13158640, 2, 3)>, <@(sa{sv}uuuu) ("IBusAttribute", {}, 2, 0, 2, 3)>])>)>
 * @u 2
 * @b True
 * @u 1
 *
 * @v <@(sa{sv}sv) ("IBusText", {}, "", <@(sa{sv}av) ("IBusAttrList", {}, [])>)>
 * @u 0
 * @b False
 * @u 0
 */
static int fill_update_preedit_text_signal (sd_bus_message *m, va_list args)
{
    const WideString &wstr = *va_arg (args, const WideString *);
    const AttributeList &attrs = *va_arg (args, const AttributeList *);

    log_func_not_impl (-ENOSYS);
}

static int fill_update_auxiliary_text_signal (sd_bus_message *m, va_list args)
{
    const WideString &wstr = *va_arg (args, const WideString *);
    const AttributeList &attrs = *va_arg (args, const AttributeList *);

    log_func_not_impl (-ENOSYS);
}

static int fill_update_lookup_table_signal (sd_bus_message *m, va_list args)
{
    const LookupTable &table = *va_arg (args, const LookupTable *);

    log_func_not_impl (-ENOSYS);
}

static int fill_register_properties_signal (sd_bus_message *m, va_list args)
{
    const PropertyList &properties = *va_arg (args, const PropertyList *);

    log_func_not_impl (-ENOSYS);
}

static int fill_update_property_signal (sd_bus_message *m, va_list args)
{
    const Property &property = *va_arg (args, const Property *);

    log_func_not_impl (-ENOSYS);
}

void
IBusFrontEnd::signal_ctx (int siid, const char *signal, ...) const
{
    log_func ();

    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (!validate_ctx (ctx)) {
        return;
    }

    char path [IBUS_INPUTCONTEXT_OBJECT_PATH_BUF_SIZE];
    ctx_gen_object_path (ctx->id (), path, sizeof (path));

    int (*filler) (sd_bus_message *m, va_list args) = NULL;

    if ("ShowPreditText" == signal) {
        filler = &fill_commit_text_signal;
    } else if ("ForwardKeyEvent" == signal) {
        filler = &fill_forward_key_event_signal;
    } else if ("UpdatePreeditText" == signal ||
               "UpdatePreeditTextWithMode" == signal) {
        filler = &fill_update_preedit_text_signal;
    } else if ("UpdateAuxiliaryText" == signal) {
        filler = &fill_update_auxiliary_text_signal;
    } else if ("UpdateLookupTable" == signal) {
        filler = &fill_update_lookup_table_signal;
    } else if ("RegisterProperties" == signal) {
        filler = &fill_register_properties_signal;
    } else if ("UpdateProperty" == signal) {
        filler = &fill_update_property_signal;
    } else if ("CommitText" == signal) {
        filler = &fill_commit_text_signal;
    } else if ("ShowPreditText" == signal ||
               "HidePreeditText" == signal ||
               "ShowAuxiliaryText" == signal ||
               "HideAuxiliaryText" == signal ||
               "ShowLookupTable" == signal ||
               "HideLookupTable" == signal ||
               "PageUpLookupTable" == signal ||
               "PageDownLookupTable" == signal ||
               "CursorUpLookupTableLookupTable" == signal ||
               "CursorDownLookupTableLookupTable" == signal) {
        filler = fill_signal;
    } else {
        log_error ("unknown signal %s", signal);
        return;
    }

    int r;
    autounref(sd_bus_message) m = NULL;
    if ((r = sd_bus_message_new_signal (m_bus,
                                        &m,
                                        path,
                                        IBUS_INPUTCONTEXT_INTERFACE,
                                        signal)) < 0) {
        log_warn ("unable to create signal: %s", strerror (-r));
    }

    va_list args;
    va_start (args, signal);
    if ((r = filler (m, args)) < 0) {
        log_warn ("error occured while appending signal arguments: %s",
                  strerror (-r));
        return;
    }

    log_trace ("emit %s => %s", signal, path);

    if ((r = sd_bus_send (m_bus, m, NULL)) < 0) {
        log_warn ("unabled to emit %s.%s: %s",
                  sd_bus_message_get_interface (m),
                  sd_bus_message_get_member (m),
                  strerror (-r));
    }
}

int IBusFrontEnd::panel_connect ()
{
    log_func();

    if (m_panel_client.is_connected()) {
        return 0;
    }

    int r;
    if ((r = m_panel_client.open_connection (m_config->get_name (), getenv ("DISPLAY"))) < 0) {
        return r;
    }

    int fd = m_panel_client.get_connection_number();
    if ((r = sd_event_add_io (m_loop,
                              &m_panel_source,
                              fd,
                              EPOLLIN,
                              &sd_event_io_adapter<IBusFrontEnd, &IBusFrontEnd::panel_handle_io>,
                              this)) < 0) {
        return r;
    }

    return r;
}

void IBusFrontEnd::panel_disconnect ()
{
    log_func();

    if (m_panel_source) {
        sd_event_source_unref(m_panel_source);
        m_panel_source = NULL; 
    }

    if (m_panel_client.is_connected()) {
        m_panel_client.close_connection();
    }
}

int
IBusFrontEnd::panel_handle_io (sd_event_source *s, int fd, uint32_t revents)
{
    log_func();

    if (!m_panel_client.filter_event ()) {
        panel_disconnect();
        panel_connect();
    }

    return 0;
}

void
IBusFrontEnd::panel_slot_reload_config (int context)
{
    log_func ();

    m_config->reload ();
}

void
IBusFrontEnd::panel_slot_exit (int context)
{
    log_func ();

    sd_event_exit (m_loop, 0);
}

void
IBusFrontEnd::panel_slot_update_lookup_table_page_size (int context, int page_size)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        update_lookup_table_page_size (ctx->siid (), page_size);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_lookup_table_page_up (int context)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        lookup_table_page_up (ctx->siid ());
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_lookup_table_page_down (int context)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        lookup_table_page_down (ctx->siid ());
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_trigger_property (int context, const String &property)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        trigger_property (ctx->siid (), property);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_process_helper_event (int context, const String &target_uuid, const String &helper_uuid, const Transaction &trans)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx) && get_instance_uuid (ctx->siid ()) == target_uuid) {
        m_panel_client.prepare (ctx->id ());
        process_helper_event (ctx->siid (), helper_uuid, trans);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_move_preedit_caret (int context, int caret_pos)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        move_preedit_caret (ctx->siid (), caret_pos);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_select_candidate (int context, int cand_index)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        select_candidate (ctx->siid (), cand_index);
        m_panel_client.send ();
    }
    log_func_not_impl ();
}
void
IBusFrontEnd::panel_slot_process_key_event (int context, const KeyEvent &key)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());

        if (!filter_hotkeys (ctx, key)) {
            if (!ctx->is_on () || !process_key_event (ctx->siid (), key)) {
//                if (!m_fallback_instance->process_key_event (key))
//                    ims_forward_key_event (ctx, key);
            }
        }

        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_commit_string (int context, const WideString &wstr)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        commit_string (ctx->id (), wstr);
//        ims_commit_string (ctx, wstr);
    }
}
void
IBusFrontEnd::panel_slot_forward_key_event (int context, const KeyEvent &key)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        forward_key_event (ctx->id (), key);
//        ims_forward_key_event (ic, key);
    }
}
void
IBusFrontEnd::panel_slot_request_help (int context)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        panel_req_show_help (ctx);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_request_factory_menu (int context)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        panel_req_show_factory_menu (ctx);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_change_factory (int context, const String &uuid)
{
    log_func ();

    SCIM_DEBUG_FRONTEND (1) << "panel_slot_change_factory " << uuid << "\n";

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        if (uuid.length () == 0 && ctx->is_on ()) {
            SCIM_DEBUG_FRONTEND (2) << "panel_slot_change_factory : turn off.\n";
            start_ctx (ctx);
//            ims_turn_off_ic (ctx);
        }else if(uuid.length () == 0 && (ctx->is_on ()  == false)){
    		panel_req_update_factory_info (ctx);
        	m_panel_client.turn_off (ctx->id ());        	
        }else if (uuid.length ()) {
            String encoding = scim_get_locale_encoding (ctx->locale ());
            String language = scim_get_locale_language (ctx->locale ());
            if (validate_factory (uuid, encoding)) {
                stop_ctx (ctx);
//                ims_turn_off_ic (ctx);
                replace_instance (ctx->siid (), uuid);
                m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
//                set_ic_capabilities (ctx);
                set_default_factory (language, uuid);
                start_ctx (ctx);
//                ims_turn_on_ic (ctx);
            }
        }
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::panel_req_update_screen (const IBusCtx *ctx)
{
    log_func_not_impl ();

//    Window target = ic->focus_win ? ic->focus_win : ic->client_win;
//    XWindowAttributes xwa;
//    if (target && 
//        XGetWindowAttributes (m_display, target, &xwa) &&
//        validate_ctx (ic)) {
//        int num = ScreenCount (m_display);
//        int idx;
//        for (idx = 0; idx < num; ++ idx) {
//            if (ScreenOfDisplay (m_display, idx) == xwa.screen) {
//                m_panel_client.update_screen (ic->icid, idx);
                m_panel_client.update_screen (ctx->id (), 0);
//                return;
//            }
//        }
//    }
}

void
IBusFrontEnd::panel_req_show_help (const IBusCtx *ctx)
{
    log_func ();

    String help;
    String tmp;

    help =  String (_("Smart Common Input Method platform ")) +
            String (SCIM_VERSION) +
            String (_("\n(C) 2002-2005 James Su <suzhe@tsinghua.org.cn>\n\n"));

    if (ctx->is_on ()) {
        help += utf8_wcstombs (get_instance_name (ctx->siid ()));
        help += String (_(":\n\n"));

        help += utf8_wcstombs (get_instance_authors (ctx->siid ()));
        help += String (_("\n\n"));

        help += utf8_wcstombs (get_instance_help (ctx->siid ()));
        help += String (_("\n\n"));

        help += utf8_wcstombs (get_instance_credits (ctx->siid ()));
    }

    m_panel_client.show_help (ctx->id (), help);
}

void
IBusFrontEnd::panel_req_show_factory_menu (const IBusCtx *ctx)
{
    log_func ();

    std::vector<String> uuids;
    if (get_factory_list_for_encoding (uuids, ctx->encoding ())) {
        std::vector <PanelFactoryInfo> menu;
        for (size_t i = 0; i < uuids.size (); ++ i) {
            menu.push_back (PanelFactoryInfo (
                                    uuids [i],
                                    utf8_wcstombs (get_factory_name (uuids [i])),
                                    get_factory_language (uuids [i]),
                                    get_factory_icon_file (uuids [i])));
        }
        m_panel_client.show_factory_menu (ctx->id (), menu);
    }
}

void
IBusFrontEnd::panel_req_focus_in (const IBusCtx *ctx)
{
    log_func ();

    m_panel_client.focus_in (ctx->id (), get_instance_uuid (ctx->siid ()));
}

void
IBusFrontEnd::panel_req_update_factory_info (const IBusCtx *ctx)
{
    log_func ();

//    if (is_focused_ic (ic)) {
        PanelFactoryInfo info;
        if (ctx->is_on ()) {
            String uuid = get_instance_uuid (ctx->siid ());
            info = PanelFactoryInfo (uuid,
                                     utf8_wcstombs (get_factory_name (uuid)),
                                     get_factory_language (uuid),
                                     get_factory_icon_file (uuid));
        } else {
            info = PanelFactoryInfo (String (""),
                                     String (_("English/Keyboard")),
                                     String ("C"),
                                     String (SCIM_KEYBOARD_ICON_FILE));
        }
        m_panel_client.update_factory_info (ctx->id (), info);
//    }
}

void
IBusFrontEnd::panel_req_update_spot_location (const IBusCtx *ctx)
{
    log_func ();

//    Window target = ic->focus_win ? ic->focus_win : ic->client_win;
//    XWindowAttributes xwa;
//
//    if (target && 
//        XGetWindowAttributes (m_display, target, &xwa) &&
//        validate_ctx (ic)) {
//
//        int spot_x, spot_y;
//        Window child;
//
//        if (m_focus_ic->pre_attr.spot_location.x >= 0 &&
//            m_focus_ic->pre_attr.spot_location.y >= 0) {
//            XTranslateCoordinates (m_display, target,
//                xwa.root,
//                m_focus_ic->pre_attr.spot_location.x + 8,
//                m_focus_ic->pre_attr.spot_location.y + 8,
//                &spot_x, &spot_y, &child);
//        } else {
//            XTranslateCoordinates (m_display, target,
//                xwa.root,
//                0,
//                xwa.height,
//                &spot_x, &spot_y, &child);
//        }
        m_panel_client.update_spot_location (ctx->id (),
                                             ctx->cursor_location().x,
                                             ctx->cursor_location().y);
//    }
}

bool
IBusFrontEnd::filter_hotkeys (IBusCtx *ctx, const KeyEvent &scimkey)
{
    bool ok = false;

    if (!is_current_ctx (ctx)) return false;

    m_frontend_hotkey_matcher.push_key_event (scimkey);
    m_imengine_hotkey_matcher.push_key_event (scimkey);

    FrontEndHotkeyAction hotkey_action = m_frontend_hotkey_matcher.get_match_result ();

    // Match trigger and show factory menu hotkeys.
    if (hotkey_action == SCIM_FRONTEND_HOTKEY_TRIGGER) {
        log_debug ("hotkey-trigger");
        if (!ctx->is_on ())
            start_ctx (ctx);
        else
            stop_ctx (ctx);
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_ON) {
        log_debug ("hotkey-on");
        if (!ctx->is_on ()) {
            start_ctx (ctx);
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_OFF) {
        log_debug ("hotkey-off");
        if (ctx->is_on ()) {
            stop_ctx (ctx);
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY) {
        log_debug ("hotkey-next");
        String encoding = scim_get_locale_encoding (ctx->locale ());
        String language = scim_get_locale_language (ctx->locale ());
        String sfid = get_next_factory ("", encoding, get_instance_uuid (ctx->siid ()));
        if (validate_factory (sfid, encoding)) {
            stop_ctx (ctx);
            replace_instance (ctx->siid (), sfid);
            m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
//            set_ic_capabilities (ctx);
            set_default_factory (language, sfid);
            start_ctx (ctx);

            log_debug ("instance name=%s, authors=%s, encoding=%s, uuid=%s",
                    utf8_wcstombs (get_instance_name (ctx->siid ())).c_str(),
                    utf8_wcstombs (get_instance_authors (ctx->siid ())).c_str(),
                    get_instance_encoding (ctx->siid ()).c_str (),
                    get_instance_uuid (ctx->siid ()).c_str ());
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY) {
        log_debug ("hotkey-prev");
        String encoding = scim_get_locale_encoding (ctx->locale ());
        String language = scim_get_locale_language (ctx->locale ());
        String sfid = get_previous_factory ("", encoding, get_instance_uuid (ctx->siid ()));
        if (validate_factory (sfid, encoding)) {
            stop_ctx (ctx);
            replace_instance (ctx->siid (), sfid);
            m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
//            set_ic_capabilities (ctx);
            set_default_factory (language, sfid);
            start_ctx (ctx);

            log_debug ("instance name=%s, authors=%s, encoding=%s, uuid=%s",
                    utf8_wcstombs (get_instance_name (ctx->siid ())).c_str(),
                    utf8_wcstombs (get_instance_authors (ctx->siid ())).c_str(),
                    get_instance_encoding (ctx->siid ()).c_str (),
                    get_instance_uuid (ctx->siid ()).c_str ());
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) {
        log_debug ("hotkey-show-factory-menu");
        panel_req_show_factory_menu (ctx);
        ok = true;
    } else if (m_imengine_hotkey_matcher.is_matched ()) {
        log_debug ("hotkey-im-hotkey");
        String encoding = scim_get_locale_encoding (ctx->locale ());
        String language = scim_get_locale_language (ctx->locale ());
        String sfid = m_imengine_hotkey_matcher.get_match_result ();
        if (validate_factory (sfid, encoding)) {
            stop_ctx (ctx);
            replace_instance (ctx->siid (), sfid);
            m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
//            set_ic_capabilities (ctx);
            set_default_factory (language, sfid);
            start_ctx (ctx);

            log_debug ("instance name=%s, authors=%s, encoding=%s, uuid=%s",
                    utf8_wcstombs (get_instance_name (ctx->siid ())).c_str(),
                    utf8_wcstombs (get_instance_authors (ctx->siid ())).c_str(),
                    get_instance_encoding (ctx->siid ()).c_str (),
                    get_instance_uuid (ctx->siid ()).c_str ());
        }
        ok = true;
    }

    return ok;
}

//int IBusFrontEnd::enum_ctx (sd_bus *bus,
//                            const char *prefix,
//                            void *userdata,
//                            char ***ret_nodes,
//                            sd_bus_error *ret_error)
//{
//    log_func ();
//
//    log_debug ("prefix=%s", prefix);
//
//    IBusFrontEnd *self = (IBusFrontEnd *) userdata;
//    if (!self->m_id_ctx_map.size ()) {
//        *ret_nodes = NULL;
//        return 0;
//    }
//
//    cleanup_strv char **nodes = (char **) calloc (1,
//                                                  (sizeof (char *) + 1) *
//                                                   self->m_id_ctx_map.size ());
//    if (!nodes) {
//        return -ENOMEM;
//    }
//
//    char **p = nodes;
//    IBusIDCtxMap::iterator it = self->m_id_ctx_map.begin();
//    for (; it != self->m_id_ctx_map.end(); it ++) {
//        asprintf(p, IBUS_INPUTCONTEXT_OBJECT_PATH "/%d", it->first);
//        if (!*p) {
//            return -ENOMEM;
//        }
//
//        ++ p;
//    }
//    nodes [self->m_id_ctx_map.size ()] = NULL;
//
//    *ret_nodes = nodes;
//    nodes = NULL;
//
//    return 0;
//}

IBusCtx::IBusCtx (const String &locale, int id, int siid)
    : m_id (id),
      m_siid (siid),
      m_caps (0),
      m_client_commit_preedit (true),
      m_on (false),
      m_shared_siid (false),
      m_locale (locale),
      m_inputcontext_slot (NULL),
      m_service_slot (NULL)
{
}

IBusCtx::~IBusCtx()
{
    if (m_inputcontext_slot) {
        sd_bus_slot_unref (m_inputcontext_slot);
    }

    if (m_service_slot) {
        sd_bus_slot_unref (m_service_slot);
    }
}

int IBusCtx::init (sd_bus *bus, const char *path)
{
    int r;
    if ((r = sd_bus_add_object_vtable (bus,
                                       &m_inputcontext_slot,
                                       path,
                                       IBUS_INPUTCONTEXT_INTERFACE,
                                       m_inputcontext_vtbl,
                                       this)) < 0) {
        return r;
    }

    if ((r = sd_bus_add_object_vtable (bus,
                                       &m_service_slot,
                                       path,
                                       IBUS_SERVICE_INTERFACE,
                                       m_service_vtbl,
                                       this)) < 0) {
        return r;
    }

    return 0;
}

uint32_t IBusCtx::scim_caps () const
{
    uint32_t caps = 0;

    if (m_caps & IBUS_CAP_PREEDIT_TEXT) {
        caps |= SCIM_CLIENT_CAP_ONTHESPOT_PREEDIT;
    }

    if (m_caps & IBUS_CAP_AUXILIARY_TEXT) {
        // no corresponding caps in SCIM
    }

    if (m_caps & IBUS_CAP_LOOKUP_TABLE) {
        caps |= SCIM_CLIENT_CAP_HELPER_MODULE;
    }

    if (m_caps & IBUS_CAP_FOCUS) {
        // no corresponding caps in SCIM
    }

    if (m_caps & IBUS_CAP_PROPERTY) {
        caps |= SCIM_CLIENT_CAP_SINGLE_LEVEL_PROPERTY |
                SCIM_CLIENT_CAP_TRIGGER_PROPERTY;
    }

    if (m_caps & IBUS_CAP_SURROUNDING_TEXT) {
        caps |= SCIM_CLIENT_CAP_SURROUNDING_TEXT;
    }

    // for testing
    caps = SCIM_CLIENT_CAP_ALL_CAPABILITIES;

    return caps;
}

int IBusCtx::caps_from_message (sd_bus_message *v)
{
    return sd_bus_message_read (v, "u", &m_caps);
}

int IBusCtx::content_type_from_message (sd_bus_message *v)
{
    return m_content_type.from_message (v);
}

int IBusCtx::content_type_to_message (sd_bus_message *v) const
{
    return m_content_type.to_message (v);
}

int IBusCtx::cursor_location_from_message (sd_bus_message *v)
{
    return m_cursor_location.from_message (v);
}

int IBusCtx::cursor_location_relative_from_message (sd_bus_message *v)
{
    IBusRect old = m_cursor_location;

    int r;
    if ((r = m_cursor_location.from_message (v)) < 0 ) {
        return r;
    }

    m_cursor_location += old;

    return 0;
}

int IBusCtx::client_commit_preedit_from_message (sd_bus_message *v)
{
    int r;
    if ((r = sd_bus_message_enter_container (v, 'r', "b")) < 0) {
        return r;
    }

    return sd_bus_message_read (v, "b", &m_client_commit_preedit);
}

int IBusCtx::client_commit_preedit_to_message (sd_bus_message *v) const
{
    int r;
    if ((r = sd_bus_message_open_container (v, 'r', "b")) < 0) {
        return r;
    }

    if ((r = sd_bus_message_append (v, "b", &m_client_commit_preedit)) < 0) {
        return r;
    }

    return sd_bus_message_close_container (v);
}

static void free_strvp (char ***strvp)
{
    if (*strvp) {
        free_strv (*strvp);
    }
}

static void free_strv (char **strv)
{
    while (*strv) {
        free (*strv);
        ++ strv;
    }
}

template<int (IBusFrontEnd::*mf)(sd_bus_message *m)>
static int
portal_message_adapter (sd_bus_message *m,
                        void *userdata,
                        sd_bus_error *ret_error)
{
    log_trace ("%s => %s.%s",
               sd_bus_message_get_sender (m),
               sd_bus_message_get_path (m),
               sd_bus_message_get_member (m));

    return (_scim_frontend->*mf)(m);
}

template<int (IBusFrontEnd::*mf)(IBusCtx *ctx, sd_bus_message *m)>
static int
ctx_message_adapter (sd_bus_message *m,
                     void *userdata,
                     sd_bus_error *ret_error)
{
    log_trace ("%s => %s.%s",
               sd_bus_message_get_sender (m),
               sd_bus_message_get_path (m),
               sd_bus_message_get_member (m));

    return (_scim_frontend->*mf)((IBusCtx *) userdata, m);
}

template<int (IBusFrontEnd::*mf) (IBusCtx *userdata, sd_bus_message *value)>
static int
ctx_prop_adapter (sd_bus *bus,
 	              const char *path,
 	              const char *interface,
 	              const char *property,
 	              sd_bus_message *value,
 	              void *userdata,
 	              sd_bus_error *ret_error)
{
    sd_bus_message *curr_msg = sd_bus_get_current_message (bus);
    log_trace ("%s => %s.%s",
               sd_bus_message_get_sender (curr_msg),
               path,
               property);

    return (_scim_frontend->*mf)((IBusCtx *) userdata, value);
}

static inline const char *
ctx_gen_object_path (int id, char *buf, size_t buf_size)
{
    assert (buf_size >= STRLEN (IBUS_INPUTCONTEXT_OBJECT_PATH) + 10);

    snprintf(buf, buf_size, IBUS_INPUTCONTEXT_OBJECT_PATH "%d", id);

    return buf;
}

//Module Interface
extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_FRONTEND(1) << "Initializing Socket FrontEnd module...\n";
    }

    void scim_module_exit (void)
    {
        SCIM_DEBUG_FRONTEND(1) << "Exiting Socket FrontEnd module...\n";
        _scim_frontend.reset ();
    }

    void scim_frontend_module_init (const BackEndPointer &backend,
                                    const ConfigPointer &config,
                                    int argc,
                                     char **argv)
    {
        if (_scim_frontend.null ()) {
            SCIM_DEBUG_FRONTEND(1) << "Initializing Socket FrontEnd module (more)...\n";
            _scim_frontend = new IBusFrontEnd (backend, config);
            _argc = argc;
            _argv = argv;
        }
    }

    void scim_frontend_module_run (void)
    {
        if (!_scim_frontend.null ()) {
            SCIM_DEBUG_FRONTEND(1) << "Starting Socket FrontEnd module...\n";
            _scim_frontend->init (_argc, _argv);
            _scim_frontend->run ();
        }
    }
}

/*
vi:ts=4:nowrap:expandtab
*/
