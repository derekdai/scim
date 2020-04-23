/** @file scim_socket_frontend.cpp
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
 * $Id: scim_socket_frontend.cpp,v 1.37 2005/07/03 08:36:42 suzhe Exp $
 *
 */

#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_FRONTEND
#define Uses_SCIM_IBUS
#define Uses_SCIM_TRANSACTION
#define Uses_STL_UTILITY
#define Uses_C_STDIO
#define Uses_C_STDLIB

#include <sys/time.h>
#include <limits.h>
#include <systemd/sd-event.h>
#include <systemd/sd-bus.h>
#include "scim_private.h"
#include "scim.h"
#include "scim_ibus_frontend.h"
#include "scim_ibus_utils.h"

#define scim_module_init ibus_LTX_scim_module_init
#define scim_module_exit ibus_LTX_scim_module_exit
#define scim_frontend_module_init ibus_LTX_scim_frontend_module_init
#define scim_frontend_module_run ibus_LTX_scim_frontend_module_run

#define SCIM_CONFIG_FRONTEND_IBUS_CONFIG_READONLY    "/FrontEnd/IBus/ConfigReadOnly"
#define SCIM_CONFIG_FRONTEND_IBUS_MAXCLIENTS        "/FrontEnd/IBus/MaxClients"

#define IBUS_PORTAL_SERVICE                       "org.freedesktop.portal.IBus"
#define IBUS_PORTAL_OBJECT_PATH                   "/org/freedesktop/IBus"
#define IBUS_PORTAL_INTERFACE                     "org.freedesktop.IBus.Portal"

using namespace scim;

template<int (IBusFrontEnd::*mf)(sd_bus_message *m, sd_bus_error *ret_error)>
int
ibus_frontend_message_adapter(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    return sd_bus_message_adapter<IBusFrontEnd, mf>(m, userdata, ret_error);
}

template<int (IBusFrontEnd::*mf) (sd_bus *bus,
 	                              const char *path,
 	                              const char *interface,
 	                              const char *property,
 	                              sd_bus_message *value,
 	                              sd_bus_error *ret_error)>
int
ibus_frontend_prop_adapter (sd_bus *bus,
 	                        const char *path,
 	                        const char *interface,
 	                        const char *property,
 	                        sd_bus_message *value,
 	                        void *userdata,
 	                        sd_bus_error *ret_error)
{
    return sd_bus_prop_adapter<IBusFrontEnd, mf> (bus,
                                                      path,
                                                      interface,
                                                      property,
                                                      value,
                                                      userdata,
                                                      ret_error);
}

static Pointer <IBusFrontEnd> _scim_frontend (0);

static int _argc;
static char **_argv;

const sd_bus_vtable IBusFrontEnd::m_portal_vtbl[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_NAMES(
            "CreateFrontEnd",
            "s", SD_BUS_PARAM(client_name),
            "o", SD_BUS_PARAM(object_path),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::portal_create_ctx>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

const sd_bus_vtable IBusFrontEnd::m_inputcontext_vtbl[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_NAMES(
            "ProcessKeyEvent",
            "uuu", SD_BUS_PARAM(keyval) SD_BUS_PARAM(keycode) SD_BUS_PARAM(state),
            "b", SD_BUS_PARAM(handled),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_process_key_event>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCursorLocation",
            "iiii", SD_BUS_PARAM(x) SD_BUS_PARAM(y) SD_BUS_PARAM(w) SD_BUS_PARAM(h),
            "", SD_BUS_PARAM(),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_set_cursor_location>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCursorLocationRelative",
            "iiii", SD_BUS_PARAM(x) SD_BUS_PARAM(y) SD_BUS_PARAM(w) SD_BUS_PARAM(h),
            "", SD_BUS_PARAM(),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_set_cursor_location_relative>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "ProcessHandWritingEvent",
            "ad", SD_BUS_PARAM(coordinates),
            "", SD_BUS_PARAM(),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_process_hand_writing_event>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "CancelHandWriting",
            "u", SD_BUS_PARAM(n_strokes),
            "", SD_BUS_PARAM(),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_cancel_hand_writing>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "FocusIn", "", "",
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_focus_in>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "FocusOut", "", "",
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_focus_out>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "Reset", "", "",
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_reset>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCapabilities",
            "u", SD_BUS_PARAM(caps),
            "", SD_BUS_PARAM(),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_set_capabilities>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "PropertyActivate",
            "su", SD_BUS_PARAM(name) SD_BUS_PARAM(state),
            "", SD_BUS_PARAM(),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_property_activate>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetEngine",
            "s", SD_BUS_PARAM(name),
            "", SD_BUS_PARAM(),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_set_engine>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "GetEngine",
            "", SD_BUS_PARAM(),
            "v", SD_BUS_PARAM(desc),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_get_engine>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetSurroundingText",
            "vuu", SD_BUS_PARAM(text) SD_BUS_PARAM(cursor_pos) SD_BUS_PARAM(anchor_pos),
            "v", SD_BUS_PARAM(desc),
            (&ibus_frontend_message_adapter<&IBusFrontEnd::ctx_set_surrounding_text>),
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
            (&ibus_frontend_prop_adapter<&IBusFrontEnd::ctx_get_client_commit_preedit>),
            (&ibus_frontend_prop_adapter<&IBusFrontEnd::ctx_set_client_commit_preedit>),
            0,
            SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_WRITABLE_PROPERTY(
            "ContentType",
            "(uu)",
            (&ibus_frontend_prop_adapter<&IBusFrontEnd::ctx_get_content_type>),
            (&ibus_frontend_prop_adapter<&IBusFrontEnd::ctx_set_content_type>),
            0,
            SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_VTABLE_END,
};

const sd_bus_vtable IBusFrontEnd::m_service_vtbl[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD(
            "Destroy",
            "",
            "",
            (&ibus_frontend_message_adapter<&IBusFrontEnd::srv_destroy>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

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

IBusFrontEnd::IBusFrontEnd (const BackEndPointer &backend,
                                const ConfigPointer  &config)
    : FrontEndBase (backend),
      m_config (config),
      m_stay (true),
      m_config_readonly (false),
      m_socket_timeout (scim_get_default_socket_timeout ()),
      m_current_instance (-1),
      m_current_socket_client (-1),
      m_current_socket_client_key (0),
      m_ctx_counter (0),
      m_loop (NULL),
      m_bus (NULL),
      m_portal_slot (NULL)
{
    SCIM_DEBUG_FRONTEND (2) << " Constructing IBusFrontEnd object...\n";
}

IBusFrontEnd::~IBusFrontEnd ()
{
    SCIM_DEBUG_FRONTEND (2) << " Destructing IBusFrontEnd object...\n";
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

    if (m_socket_server.is_running ())
        m_socket_server.shutdown ();
}

void
IBusFrontEnd::show_preedit_string (int id)
{
    if (m_current_instance == id)
        m_send_trans.put_command (SCIM_TRANS_CMD_SHOW_PREEDIT_STRING);
}

void
IBusFrontEnd::show_aux_string (int id)
{
    if (m_current_instance == id)
        m_send_trans.put_command (SCIM_TRANS_CMD_SHOW_AUX_STRING);
}

void
IBusFrontEnd::show_lookup_table (int id)
{
    if (m_current_instance == id)
        m_send_trans.put_command (SCIM_TRANS_CMD_SHOW_LOOKUP_TABLE);
}

void
IBusFrontEnd::hide_preedit_string (int id)
{
    if (m_current_instance == id)
        m_send_trans.put_command (SCIM_TRANS_CMD_HIDE_PREEDIT_STRING);
}

void
IBusFrontEnd::hide_aux_string (int id)
{
    if (m_current_instance == id)
        m_send_trans.put_command (SCIM_TRANS_CMD_HIDE_AUX_STRING);
}

void
IBusFrontEnd::hide_lookup_table (int id)
{
    if (m_current_instance == id)
        m_send_trans.put_command (SCIM_TRANS_CMD_HIDE_LOOKUP_TABLE);
}

void
IBusFrontEnd::update_preedit_caret (int id, int caret)
{
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_PREEDIT_CARET);
        m_send_trans.put_data ((uint32) caret);
    }
}

void
IBusFrontEnd::update_preedit_string (int id,
                                       const WideString & str,
                                       const AttributeList & attrs)
{
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_PREEDIT_STRING);
        m_send_trans.put_data (str);
        m_send_trans.put_data (attrs);
    }
}

void
IBusFrontEnd::update_aux_string (int id,
                                   const WideString & str,
                                   const AttributeList & attrs)
{
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_AUX_STRING);
        m_send_trans.put_data (str);
        m_send_trans.put_data (attrs);
    }
}

void
IBusFrontEnd::commit_string (int id, const WideString & str)
{
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_COMMIT_STRING);
        m_send_trans.put_data (str);
    }
}

void
IBusFrontEnd::forward_key_event (int id, const KeyEvent & key)
{
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_FORWARD_KEY_EVENT);
        m_send_trans.put_data (key);
    }
}

void
IBusFrontEnd::update_lookup_table (int id, const LookupTable & table)
{
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE);
        m_send_trans.put_data (table);
    }
}

void
IBusFrontEnd::register_properties (int id, const PropertyList &properties)
{
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_REGISTER_PROPERTIES);
        m_send_trans.put_data (properties);
    }
}

void
IBusFrontEnd::update_property (int id, const Property &property)
{
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_PROPERTY);
        m_send_trans.put_data (property);
    }
}

void
IBusFrontEnd::beep (int id)
{
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_BEEP);
    }
}

void
IBusFrontEnd::start_helper (int id, const String &helper_uuid)
{
    SCIM_DEBUG_FRONTEND (2) << "start_helper (" << helper_uuid << ")\n";
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_START_HELPER);
        m_send_trans.put_data (helper_uuid);
    }
}

void
IBusFrontEnd::stop_helper (int id, const String &helper_uuid)
{
    SCIM_DEBUG_FRONTEND (2) << "stop_helper (" << helper_uuid << ")\n";

    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_STOP_HELPER);
        m_send_trans.put_data (helper_uuid);
    }
}

void
IBusFrontEnd::send_helper_event (int id, const String &helper_uuid, const Transaction &trans)
{
    if (m_current_instance == id) {
        m_send_trans.put_command (SCIM_TRANS_CMD_SEND_HELPER_EVENT);
        m_send_trans.put_data (helper_uuid);
        m_send_trans.put_data (trans);
    }
}

bool
IBusFrontEnd::get_surrounding_text (int id, WideString &text, int &cursor, int maxlen_before, int maxlen_after)
{
    text.clear ();
    cursor = 0;

    if (m_current_instance == id && m_current_socket_client >= 0 && (maxlen_before != 0 || maxlen_after != 0)) {
        if (maxlen_before < 0) maxlen_before = -1;
        if (maxlen_after < 0) maxlen_after = -1;

        m_temp_trans.clear ();
        m_temp_trans.put_command (SCIM_TRANS_CMD_REPLY);
        m_temp_trans.put_command (SCIM_TRANS_CMD_GET_SURROUNDING_TEXT);
        m_temp_trans.put_data ((uint32) maxlen_before);
        m_temp_trans.put_data ((uint32) maxlen_after);

        Socket socket_client (m_current_socket_client);

        if (m_temp_trans.write_to_socket (socket_client) &&
            m_temp_trans.read_from_socket (socket_client, m_socket_timeout)) {

            int cmd;
            uint32 key;
            uint32 cur;

            if (m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REQUEST &&
                m_temp_trans.get_data (key) && key == m_current_socket_client_key &&
                m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_GET_SURROUNDING_TEXT &&
                m_temp_trans.get_data (text) && m_temp_trans.get_data (cur)) {
                cursor = (int) cur;
                return true;
            }
        }
    }
    return false;
}

bool
IBusFrontEnd::delete_surrounding_text (int id, int offset, int len)
{
    if (m_current_instance == id && m_current_socket_client >= 0 && len > 0) {
        m_temp_trans.clear ();
        m_temp_trans.put_command (SCIM_TRANS_CMD_REPLY);
        m_temp_trans.put_command (SCIM_TRANS_CMD_DELETE_SURROUNDING_TEXT);
        m_temp_trans.put_data ((uint32) offset);
        m_temp_trans.put_data ((uint32) len);

        Socket socket_client (m_current_socket_client);

        if (m_temp_trans.write_to_socket (socket_client) &&
            m_temp_trans.read_from_socket (socket_client, m_socket_timeout)) {

            int cmd;
            uint32 key;

            if (m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REQUEST &&
                m_temp_trans.get_data (key) && key == m_current_socket_client_key &&
                m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_DELETE_SURROUNDING_TEXT &&
                m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
                return true;
        }
    }
    return false;
}

void
IBusFrontEnd::init (int argc, char **argv)
{
    int max_clients = -1;

    if (!m_config.null ()) {
        String str;

        m_config_readonly = m_config->read (String (SCIM_CONFIG_FRONTEND_IBUS_CONFIG_READONLY), false);

        max_clients = m_config->read (String (SCIM_CONFIG_FRONTEND_IBUS_MAXCLIENTS), -1);

        m_config->signal_connect_reload (slot (this, &IBusFrontEnd::reload_config_callback));
    } else {
        m_config_readonly = false;
        max_clients = -1;
    }

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

    /**
     * initialize the random number generator.
     */
//    srand (time (0));
}

void
IBusFrontEnd::run ()
{
//    if (m_socket_server.valid ())
//        m_socket_server.run ();
    if (m_loop) {
        sd_event_loop (m_loop);
    }
}

int
IBusFrontEnd::generate_ctx_id ()
{
    return ++ m_ctx_counter;
}

bool
IBusFrontEnd::check_client_connection (const Socket &client) const
{
    SCIM_DEBUG_FRONTEND (1) << "check_client_connection (" << client.get_id () << ").\n";

    unsigned char buf [sizeof(uint32)];

    int nbytes = client.read_with_timeout (buf, sizeof(uint32), m_socket_timeout);

    if (nbytes == sizeof (uint32))
        return true;

    if (nbytes < 0) {
        SCIM_DEBUG_FRONTEND (2) << " Error occurred when reading ibus (" << client.get_id ()
            << "):" << client.get_error_message () << "\n";
    } else {
        SCIM_DEBUG_FRONTEND (2) << " Timeout when reading ibus (" << client.get_id ()
            << ").\n";
    }

    return false;
}

void
IBusFrontEnd::ibus_accept_callback (SocketServer *server, const Socket &client)
{
    SCIM_DEBUG_FRONTEND (1) << "ibus_accept_callback (" << client.get_id () << ").\n";
}

void
IBusFrontEnd::ibus_receive_callback (SocketServer *server, const Socket &client)
{
    int id = client.get_id ();
    int cmd;
    uint32 key;

    ClientInfo client_info;

    SCIM_DEBUG_FRONTEND (1) << "ibus_receive_callback (" << id << ").\n";

    // Check if the client is closed.
    if (!check_client_connection (client)) {
        SCIM_DEBUG_FRONTEND (2) << " closing client connection.\n";
        ibus_close_connection (server, client);
        return;
    }

    client_info = ibus_get_client_info (client);

    // If it's a new client, then request to open the connection first.
    if (client_info.type == UNKNOWN_CLIENT) {
        ibus_open_connection (server, client);
        return;
    }

    // If can not read the transaction,
    // or the transaction is not started with SCIM_TRANS_CMD_REQUEST,
    // or the key is mismatch,
    // just return.
    if (!m_receive_trans.read_from_socket (client, m_socket_timeout) ||
        !m_receive_trans.get_command (cmd) || cmd != SCIM_TRANS_CMD_REQUEST ||
        !m_receive_trans.get_data (key) || key != (uint32) client_info.key)
        return;

    m_current_socket_client     = id;
    m_current_socket_client_key = key;

    m_send_trans.clear ();
    m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);

    // Move the read ptr to the end.
    m_send_trans.get_command (cmd);

    while (m_receive_trans.get_command (cmd)) {
        if (cmd == SCIM_TRANS_CMD_PROCESS_KEY_EVENT)
            ibus_process_key_event (id);
        else if (cmd == SCIM_TRANS_CMD_MOVE_PREEDIT_CARET)
            ibus_move_preedit_caret (id);
        else if (cmd == SCIM_TRANS_CMD_SELECT_CANDIDATE)
            ibus_select_candidate (id);
        else if (cmd == SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE_PAGE_SIZE)
            ibus_update_lookup_table_page_size (id);
        else if (cmd == SCIM_TRANS_CMD_LOOKUP_TABLE_PAGE_UP)
            ibus_lookup_table_page_up (id);
        else if (cmd == SCIM_TRANS_CMD_LOOKUP_TABLE_PAGE_DOWN)
            ibus_lookup_table_page_down (id);
        else if (cmd == SCIM_TRANS_CMD_RESET)
            ibus_reset (id);
        else if (cmd == SCIM_TRANS_CMD_FOCUS_IN)
            ibus_focus_in (id);
        else if (cmd == SCIM_TRANS_CMD_FOCUS_OUT)
            ibus_focus_out (id);
        else if (cmd == SCIM_TRANS_CMD_TRIGGER_PROPERTY)
            ibus_trigger_property (id);
        else if (cmd == SCIM_TRANS_CMD_PROCESS_HELPER_EVENT)
            ibus_process_helper_event (id);
        else if (cmd == SCIM_TRANS_CMD_UPDATE_CLIENT_CAPABILITIES)
            ibus_update_client_capabilities (id);
        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_LIST)
            ibus_get_factory_list (id);
        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_NAME)
            ibus_get_factory_name (id);
        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_AUTHORS)
            ibus_get_factory_authors (id);
        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_CREDITS)
            ibus_get_factory_credits (id);
        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_HELP)
            ibus_get_factory_help (id);
        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_LOCALES)
            ibus_get_factory_locales (id);
        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_ICON_FILE)
            ibus_get_factory_icon_file (id);
        else if (cmd == SCIM_TRANS_CMD_GET_FACTORY_LANGUAGE)
            ibus_get_factory_language (id);
        else if (cmd == SCIM_TRANS_CMD_NEW_INSTANCE)
            ibus_new_instance (id);
        else if (cmd == SCIM_TRANS_CMD_DELETE_INSTANCE)
            ibus_delete_instance (id);
        else if (cmd == SCIM_TRANS_CMD_DELETE_ALL_INSTANCES)
            ibus_delete_all_instances (id);
        else if (cmd == SCIM_TRANS_CMD_FLUSH_CONFIG)
            ibus_flush_config (id);
        else if (cmd == SCIM_TRANS_CMD_ERASE_CONFIG)
            ibus_erase_config (id);
        else if (cmd == SCIM_TRANS_CMD_RELOAD_CONFIG)
            ibus_reload_config (id);
        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_STRING)
            ibus_get_config_string (id);
        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_STRING)
            ibus_set_config_string (id);
        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_INT)
            ibus_get_config_int (id);
        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_INT)
            ibus_set_config_int (id);
        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_BOOL)
            ibus_get_config_bool (id);
        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_BOOL)
            ibus_set_config_bool (id);
        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_DOUBLE)
            ibus_get_config_double (id);
        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_DOUBLE)
            ibus_set_config_double (id);
        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_VECTOR_STRING)
            ibus_get_config_vector_string (id);
        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_VECTOR_STRING)
            ibus_set_config_vector_string (id);
        else if (cmd == SCIM_TRANS_CMD_GET_CONFIG_VECTOR_INT)
            ibus_get_config_vector_int (id);
        else if (cmd == SCIM_TRANS_CMD_SET_CONFIG_VECTOR_INT)
            ibus_set_config_vector_int (id);
        else if (cmd == SCIM_TRANS_CMD_LOAD_FILE)
            ibus_load_file (id);
        else if (cmd == SCIM_TRANS_CMD_CLOSE_CONNECTION) {
            ibus_close_connection (server, client);
            m_current_socket_client     = -1;
            m_current_socket_client_key = 0;
            return;
        }
    }

    // Send reply to client
    if (m_send_trans.get_data_type () == SCIM_TRANS_DATA_UNKNOWN)
        m_send_trans.put_command (SCIM_TRANS_CMD_FAIL);

    m_send_trans.write_to_socket (client);

    m_current_socket_client     = -1;
    m_current_socket_client_key = 0;

    SCIM_DEBUG_FRONTEND (1) << "End of ibus_receive_callback (" << id << ").\n";
}

bool
IBusFrontEnd::ibus_open_connection (SocketServer *server, const Socket &client)
{
    SCIM_DEBUG_FRONTEND (2) << " Open ibus connection for client " << client.get_id () << "  number of clients=" << m_socket_client_repository.size () << ".\n";

    uint32 key;
    String type = scim_socket_accept_connection (key,
                                                 String ("IBusFrontEnd"), 
                                                 String ("IBusIMEngine,IBusConfig"),
                                                 client,
                                                 m_socket_timeout);

    if (type.length ()) {
        ClientInfo info;
        info.key = key;
        info.type = ((type == "IBusIMEngine") ? IMENGINE_CLIENT : CONFIG_CLIENT);

        SCIM_DEBUG_MAIN (2) << " Add client to repository. Type=" << type << " key=" << key << "\n";
        m_socket_client_repository [client.get_id ()] = info;
        return true;
    }

    // Client did not pass the registration process, close it.
    SCIM_DEBUG_FRONTEND (2) << " Failed to create new connection.\n"; 
    server->close_connection (client);
    return false;
}

void
IBusFrontEnd::ibus_close_connection (SocketServer *server, const Socket &client)
{
    SCIM_DEBUG_FRONTEND (2) << " Close client connection " << client.get_id () << "  number of clients=" << m_socket_client_repository.size () << ".\n";

    ClientInfo client_info = ibus_get_client_info (client);

    server->close_connection (client);

    if (client_info.type != UNKNOWN_CLIENT) {
        m_socket_client_repository.erase (client.get_id ());

        if (client_info.type == IMENGINE_CLIENT)
            ibus_delete_all_instances (client.get_id ());

        if (!m_socket_client_repository.size () && !m_stay)
            server->shutdown ();
    }
}

IBusFrontEnd::ClientInfo
IBusFrontEnd::ibus_get_client_info (const Socket &client)
{
    static ClientInfo null_client = { 0, UNKNOWN_CLIENT };
    IBusClientRepository::iterator it = m_socket_client_repository.find (client.get_id ());

    if (it != m_socket_client_repository.end ())
        return it->second;

    return null_client;
}

void
IBusFrontEnd::ibus_exception_callback (SocketServer *server, const Socket &client)
{
    SCIM_DEBUG_FRONTEND (1) << "ibus_exception_callback (" << client.get_id () << ").\n";

    ibus_close_connection (server, client);
}

//client_id is client's ibus id
void
IBusFrontEnd::ibus_get_factory_list (int /*client_id*/)
{
    String encoding;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_list.\n";

    if (m_receive_trans.get_data (encoding)) {
        std::vector<String> uuids;

        get_factory_list_for_encoding (uuids, encoding);

        SCIM_DEBUG_FRONTEND (3) << "  Encoding (" << encoding
            << ") Num(" << uuids.size () << ").\n";

        m_send_trans.put_data (uuids);
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_factory_name (int /*client_id*/)
{
    String sfid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_name.\n";

    if (m_receive_trans.get_data (sfid)) {
        WideString name = get_factory_name (sfid);

        m_send_trans.put_data (name);
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_factory_authors (int /*client_id*/)
{
    String sfid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_authors.\n";

    if (m_receive_trans.get_data (sfid)) {
        WideString authors = get_factory_authors (sfid);

        m_send_trans.put_data (authors);
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_factory_credits (int /*client_id*/)
{
    String sfid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_credits.\n";

    if (m_receive_trans.get_data (sfid)) {
        WideString credits = get_factory_credits (sfid);

        m_send_trans.put_data (credits);
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_factory_help (int /*client_id*/)
{
    String sfid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_help.\n";

    if (m_receive_trans.get_data (sfid)) {
        WideString help = get_factory_help (sfid);

        m_send_trans.put_data (help);
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_factory_locales (int /*client_id*/)
{
    String sfid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_locales.\n";

    if (m_receive_trans.get_data (sfid)) {
        String locales = get_factory_locales (sfid);

        SCIM_DEBUG_FRONTEND (3) << "  Locales (" << locales << ").\n";

        m_send_trans.put_data (locales);
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_factory_icon_file (int /*client_id*/)
{
    String sfid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_icon_file.\n";

    if (m_receive_trans.get_data (sfid)) {
        String iconfile = get_factory_icon_file (sfid);

        SCIM_DEBUG_FRONTEND (3) << "  ICON File (" << iconfile << ").\n";

        m_send_trans.put_data (iconfile);
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_factory_language (int /*client_id*/)
{
    String sfid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_factory_language.\n";

    if (m_receive_trans.get_data (sfid)) {
        String language = get_factory_language (sfid);

        SCIM_DEBUG_FRONTEND (3) << "  Language (" << language << ").\n";

        m_send_trans.put_data (language);
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_new_instance (int client_id)
{
    String sfid;
    String encoding;

    SCIM_DEBUG_FRONTEND (2) << " ibus_new_instance.\n";

    if (m_receive_trans.get_data (sfid) &&
        m_receive_trans.get_data (encoding)) {
        int siid = new_instance (sfid, encoding);

        // Instance created OK.
        if (siid >= 0) {
            IBusInstanceRepository::iterator it =
                std::lower_bound (m_socket_instance_repository.begin (),
                                  m_socket_instance_repository.end (),
                                  std::pair <int, int> (client_id, siid));

            if (it == m_socket_instance_repository.end ())
                m_socket_instance_repository.push_back (std::pair <int, int> (client_id, siid));
            else
                m_socket_instance_repository.insert (it, std::pair <int, int> (client_id, siid));

            SCIM_DEBUG_FRONTEND (3) << "  InstanceID (" << siid << ").\n";

            m_send_trans.put_data ((uint32)siid);
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
        }
    }
}

void
IBusFrontEnd::ibus_delete_instance (int client_id)
{
    uint32 siid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_delete_instance.\n";

    if (m_receive_trans.get_data (siid)) {

        SCIM_DEBUG_FRONTEND (3) << "  InstanceID (" << siid << ").\n";

        m_current_instance = (int) siid;

        delete_instance ((int) siid);

        m_current_instance = -1;

        IBusInstanceRepository::iterator it =
            std::lower_bound (m_socket_instance_repository.begin (),
                              m_socket_instance_repository.end (),
                              std::pair <int, int> (client_id, siid));

        if (it != m_socket_instance_repository.end () &&
            *it == std::pair <int, int> (client_id, siid))
            m_socket_instance_repository.erase (it);

        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_delete_all_instances (int client_id)
{
    SCIM_DEBUG_FRONTEND (2) << " ibus_delete_all_instances.\n";

    IBusInstanceRepository::iterator it;

    IBusInstanceRepository::iterator lit =
        std::lower_bound (m_socket_instance_repository.begin (),
                          m_socket_instance_repository.end (),
                          std::pair <int, int> (client_id, 0));

    IBusInstanceRepository::iterator uit =
        std::upper_bound (m_socket_instance_repository.begin (),
                          m_socket_instance_repository.end (),
                          std::pair <int, int> (client_id, INT_MAX));

    if (lit != uit) {
        for (it = lit; it != uit; ++it) {
            m_current_instance = it->second;
            delete_instance (it->second);
        }
        m_current_instance = -1;
        m_socket_instance_repository.erase (lit, uit);
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_process_key_event (int /*client_id*/)
{
    uint32   siid;
    KeyEvent event;

    SCIM_DEBUG_FRONTEND (2) << " ibus_process_key_event.\n";

    if (m_receive_trans.get_data (siid) &&
        m_receive_trans.get_data (event)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ") KeyEvent ("
            << event.code << "," << event.mask << ").\n";

        m_current_instance = (int) siid;

        if (process_key_event ((int) siid, event))
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
        else
            m_send_trans.put_command (SCIM_TRANS_CMD_FAIL);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_move_preedit_caret (int /*client_id*/)
{
    uint32 siid;
    uint32 caret;

    SCIM_DEBUG_FRONTEND (2) << " ibus_move_preedit_caret.\n";

    if (m_receive_trans.get_data (siid) &&
        m_receive_trans.get_data (caret)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid
            << ") Caret (" << caret << ").\n";

        m_current_instance = (int) siid;

        move_preedit_caret ((int) siid, caret); 
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_select_candidate (int /*client_id*/)
{
    uint32 siid;
    uint32 item;

    SCIM_DEBUG_FRONTEND (2) << " ibus_select_candidate.\n";

    if (m_receive_trans.get_data (siid) &&
        m_receive_trans.get_data (item)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ") Item (" << item << ").\n";

        m_current_instance = (int) siid;

        select_candidate ((int) siid, item); 
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_update_lookup_table_page_size (int /*client_id*/)
{
    uint32 siid;
    uint32 size;

    SCIM_DEBUG_FRONTEND (2) << " ibus_update_lookup_table_page_size.\n";

    if (m_receive_trans.get_data (siid) &&
        m_receive_trans.get_data (size)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ") PageSize (" << size << ").\n";

        m_current_instance = (int) siid;

        update_lookup_table_page_size ((int) siid, size); 
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_lookup_table_page_up (int /*client_id*/)
{
    uint32 siid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_lookup_table_page_up.\n";

    if (m_receive_trans.get_data (siid)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";

        m_current_instance = (int) siid;

        lookup_table_page_up ((int) siid); 
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_lookup_table_page_down (int /*client_id*/)
{
    uint32 siid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_lookup_table_page_down.\n";

    if (m_receive_trans.get_data (siid)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";

        m_current_instance = (int) siid;

        lookup_table_page_down ((int) siid); 
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_reset (int /*client_id*/)
{
    uint32 siid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_reset.\n";

    if (m_receive_trans.get_data (siid)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";

        m_current_instance = (int) siid;

        reset ((int) siid); 
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_focus_in (int /*client_id*/)
{
    uint32 siid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_focus_in.\n";

    if (m_receive_trans.get_data (siid)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";

        m_current_instance = (int) siid;

        focus_in ((int) siid); 
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_focus_out (int /*client_id*/)
{
    uint32 siid;

    SCIM_DEBUG_FRONTEND (2) << " ibus_focus_out.\n";

    if (m_receive_trans.get_data (siid)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";

        m_current_instance = (int) siid;

        focus_out ((int) siid); 
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_trigger_property (int /*client_id*/)
{
    uint32 siid;
    String property;

    SCIM_DEBUG_FRONTEND (2) << " ibus_trigger_property.\n";

    if (m_receive_trans.get_data (siid) &&
        m_receive_trans.get_data (property)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";

        m_current_instance = (int) siid;

        trigger_property ((int) siid, property); 
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_process_helper_event (int /*client_id*/)
{
    uint32 siid;
    String helper_uuid;
    Transaction trans;

    SCIM_DEBUG_FRONTEND (2) << " ibus_process_helper_event.\n";

    if (m_receive_trans.get_data (siid) &&
        m_receive_trans.get_data (helper_uuid) &&
        m_receive_trans.get_data (trans)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";

        m_current_instance = (int) siid;

        process_helper_event ((int) siid, helper_uuid, trans); 
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}

void
IBusFrontEnd::ibus_update_client_capabilities (int /*client_id*/)
{
    uint32 siid;
    uint32 cap;

    SCIM_DEBUG_FRONTEND (2) << " ibus_update_client_capabilities.\n";

    if (m_receive_trans.get_data (siid) && m_receive_trans.get_data (cap)) {

        SCIM_DEBUG_FRONTEND (3) << "  SI (" << siid << ").\n";

        m_current_instance = (int) siid;

        update_client_capabilities ((int) siid, cap); 

        m_send_trans.put_command (SCIM_TRANS_CMD_OK);

        m_current_instance = -1;
    }
}


void
IBusFrontEnd::ibus_flush_config (int /*client_id*/)
{
    if (m_config_readonly || m_config.null ())
        return;

    SCIM_DEBUG_FRONTEND (2) << " ibus_flush_config.\n";

    if (m_config->flush ())
        m_send_trans.put_command (SCIM_TRANS_CMD_OK);
}

void
IBusFrontEnd::ibus_erase_config (int /*client_id*/)
{
    if (m_config_readonly || m_config.null ())
        return;

    String key;

    SCIM_DEBUG_FRONTEND (2) << " ibus_erase_config.\n";

    if (m_receive_trans.get_data (key)) {

        SCIM_DEBUG_FRONTEND (3) << "  Key   (" << key << ").\n";

        if (m_config->erase (key))
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_reload_config (int /*client_id*/)
{
    static timeval last_timestamp = {0, 0};

    if (m_config.null ())
        return;

    SCIM_DEBUG_FRONTEND (2) << " ibus_reload_config.\n";

    timeval timestamp;

    gettimeofday (&timestamp, 0);

    if (timestamp.tv_sec > last_timestamp.tv_sec + 1)
        m_config->reload ();

    gettimeofday (&last_timestamp, 0);

    m_send_trans.put_command (SCIM_TRANS_CMD_OK);
}

void
IBusFrontEnd::ibus_get_config_string (int /*client_id*/)
{
    if (m_config.null ()) return;

    String key;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_string.\n";

    if (m_receive_trans.get_data (key)) {
        String value;

        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";

        if (m_config->read (key, &value)) {
            m_send_trans.put_data (value);
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
        }
    }
}

void
IBusFrontEnd::ibus_set_config_string (int /*client_id*/)
{
    if (m_config_readonly || m_config.null ())
        return;

    String key;
    String value;

    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_string.\n";

    if (m_receive_trans.get_data (key) &&
        m_receive_trans.get_data (value)) {

        SCIM_DEBUG_FRONTEND (3) << "  Key   (" << key << ").\n";
        SCIM_DEBUG_FRONTEND (3) << "  Value (" << value << ").\n";

        if (m_config->write (key, value))
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_config_int (int /*client_id*/)
{
    if (m_config.null ()) return;

    String key;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_int.\n";

    if (m_receive_trans.get_data (key)) {

        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";

        int value;
        if (m_config->read (key, &value)) {
            m_send_trans.put_data ((uint32) value);
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
        }
    }
}

void
IBusFrontEnd::ibus_set_config_int (int /*client_id*/)
{
    if (m_config_readonly || m_config.null ())
        return;

    String key;
    uint32 value;

    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_int.\n";

    if (m_receive_trans.get_data (key) &&
        m_receive_trans.get_data (value)) {

        SCIM_DEBUG_FRONTEND (3) << "  Key   (" << key << ").\n";
        SCIM_DEBUG_FRONTEND (3) << "  Value (" << value << ").\n";

        if (m_config->write (key, (int) value))
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_config_bool (int /*client_id*/)
{
    if (m_config.null ()) return;

    String key;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_bool.\n";

    if (m_receive_trans.get_data (key)) {
        bool value;

        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";

        if (m_config->read (key, &value)) {
            m_send_trans.put_data ((uint32) value);
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
        }
    }
}

void
IBusFrontEnd::ibus_set_config_bool (int /*client_id*/)
{
    if (m_config_readonly || m_config.null ())
        return;

    String key;
    uint32 value;

    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_bool.\n";

    if (m_receive_trans.get_data (key) &&
        m_receive_trans.get_data (value)) {

        SCIM_DEBUG_FRONTEND (3) << "  Key   (" << key << ").\n";
        SCIM_DEBUG_FRONTEND (3) << "  Value (" << value << ").\n";

        if (m_config->write (key, (bool) value))
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_config_double (int /*client_id*/)
{
    if (m_config.null ()) return;

    String key;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_double.\n";

    if (m_receive_trans.get_data (key)) {
        double value;

        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";

        if (m_config->read (key, &value)) {
            char buf [80];
            snprintf (buf, 79, "%lE", value);
            m_send_trans.put_data (String (buf));
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
        }
    }
}

void
IBusFrontEnd::ibus_set_config_double (int /*client_id*/)
{
    if (m_config_readonly || m_config.null ())
        return;

    String key;
    String str;

    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_double.\n";

    if (m_receive_trans.get_data (key) &&
        m_receive_trans.get_data (str)) {
        double value;
        sscanf (str.c_str (), "%lE", &value);

        SCIM_DEBUG_FRONTEND (3) << "  Key   (" << key << ").\n";
        SCIM_DEBUG_FRONTEND (3) << "  Value (" << value << ").\n";

        if (m_config->write (key, value))
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_config_vector_string (int /*client_id*/)
{
    if (m_config.null ()) return;

    String key;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_vector_string.\n";

    if (m_receive_trans.get_data (key)) {
        std::vector <String> vec;

        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";

        if (m_config->read (key, &vec)) {
            m_send_trans.put_data (vec);
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
        }
    }
}

void
IBusFrontEnd::ibus_set_config_vector_string (int /*client_id*/)
{
    if (m_config_readonly || m_config.null ())
        return;

    String key;
    std::vector<String> vec;

    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_vector_string.\n";

    if (m_receive_trans.get_data (key) &&
        m_receive_trans.get_data (vec)) {

        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";

        if (m_config->write (key, vec))
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_get_config_vector_int (int /*client_id*/)
{
    if (m_config.null ()) return;

    String key;

    SCIM_DEBUG_FRONTEND (2) << " ibus_get_config_vector_int.\n";

    if (m_receive_trans.get_data (key)) {
        std::vector <int> vec;

        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";

        if (m_config->read (key, &vec)) {
            std::vector <uint32> reply;

            for (uint32 i=0; i<vec.size (); ++i)
                reply.push_back ((uint32) vec[i]);

            m_send_trans.put_data (reply);
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
        }
    }
}

void
IBusFrontEnd::ibus_set_config_vector_int (int /*client_id*/)
{
    if (m_config_readonly || m_config.null ())
        return;

    String key;
    std::vector<uint32> vec;

    SCIM_DEBUG_FRONTEND (2) << " ibus_set_config_vector_int.\n";

    if (m_receive_trans.get_data (key) &&
        m_receive_trans.get_data (vec)) {
        std::vector<int> req;

        SCIM_DEBUG_FRONTEND (3) << "  Key (" << key << ").\n";

        for (uint32 i=0; i<vec.size (); ++i)
            req.push_back ((int) vec[i]);

        if (m_config->write (key, req))
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
    }
}

void
IBusFrontEnd::ibus_load_file (int /*client_id*/)
{
    String filename;
    char *bufptr = 0;
    size_t filesize = 0;

    SCIM_DEBUG_FRONTEND (2) << " ibus_load_file.\n";

    if (m_receive_trans.get_data (filename)) {
        SCIM_DEBUG_FRONTEND (3) << "  File (" << filename << ").\n";

        if ((filesize = scim_load_file (filename, &bufptr)) > 0) {
            m_send_trans.put_data (bufptr, filesize);
            m_send_trans.put_command (SCIM_TRANS_CMD_OK);
        }

        delete [] bufptr;
    }
}

void
IBusFrontEnd::reload_config_callback (const ConfigPointer &config)
{
    SCIM_DEBUG_FRONTEND (1) << "Reload configuration.\n";

    int max_clients = -1;

    m_config_readonly = config->read (String (SCIM_CONFIG_FRONTEND_IBUS_CONFIG_READONLY), false);
    max_clients = config->read (String (SCIM_CONFIG_FRONTEND_IBUS_MAXCLIENTS), -1);

    m_socket_server.set_max_clients (max_clients);
}

int IBusFrontEnd::portal_create_ctx(sd_bus_message *m,
                                    sd_bus_error *ret_error)
{
    log_func_not_impl(-ENOSYS);
}

int IBusFrontEnd::srv_destroy(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl(-ENOSYS);
}

int IBusFrontEnd::ctx_process_key_event(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl(-ENOSYS);

//    uint32_t keyval = 0;
//    uint32_t keycode = 0;
//    uint32_t state = 0;
//
//    if (sd_bus_message_read(m, "uuu", &keyval, &keycode, &state) < 0) {
//        return -1;
//    }
//
//    bool processed = m_observer->input_context_process_key_event(this,
//                                                               keyval,
//                                                               keycode,
//                                                               state);
//    return sd_bus_reply_method_return(m, "b", processed);
}

int IBusFrontEnd::ctx_set_cursor_location(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl(-ENOSYS);

//    if (sd_bus_message_read(m,
//                            "iiii",
//                            &m_cursor_location.x,
//                            &m_cursor_location.y,
//                            &m_cursor_location.h,
//                            &m_cursor_location.y) < 0) {
//        return -1;
//    }
//
//    m_observer->input_context_cursor_location_updated(this);
//
//    return 0;
}

int IBusFrontEnd::ctx_set_cursor_location_relative(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl(-ENOSYS);

//    IBusRect rect; 
//    if (sd_bus_message_read(m, "iiii", &rect.x, &rect.y, &rect.h, &rect.y) < 0) {
//        return -1;
//    }
//
//    m_cursor_location.x += rect.x;
//    m_cursor_location.y += rect.y;
//    m_cursor_location.w += rect.w;
//    m_cursor_location.h += rect.h;
//
//    return 0;
}


int IBusFrontEnd::ctx_process_hand_writing_event(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_cancel_hand_writing(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_property_activate(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_set_engine(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_get_engine(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_set_surrounding_text(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);
}

int IBusFrontEnd::ctx_focus_in(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);

//    m_observer->input_context_focus_in(this);
//
//    return 0;
}

int IBusFrontEnd::ctx_focus_out(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);

//    m_observer->input_context_focus_out(this);
//
//    return 0;
}

int IBusFrontEnd::ctx_reset(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);

//    m_observer->input_context_reset(this);
//
//    return 0;
}

int IBusFrontEnd::ctx_set_capabilities(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);

//    if (sd_bus_message_read(m, "u", &m_capabilities) < 0) {
//        return -1;
//    }
//
//    log_debug("capabilities = %s", ibus_caps_to_str(m_capabilities).c_str());
//
//    m_observer->input_context_capability_updated(this);
//
//    return 0;
}

int IBusFrontEnd::ctx_get_client_commit_preedit (sd_bus *bus,
                                                 const char *path,
                                                 const char *interface,
                                                 const char *property,
                                                 sd_bus_message *value,
                                                 sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);

//    if (sd_bus_message_open_container(value, 'r', "b") < 0) {
//        return -1;
//    }
//
//    bool v = m_client_commit_preedit;
//    if (sd_bus_message_append(value, "b", v) < 0) {
//        return -1;
//    }
//
//    return sd_bus_message_close_container(value);
}

int IBusFrontEnd::ctx_set_client_commit_preedit (sd_bus *bus,
                                                 const char *path,
                                                 const char *interface,
                                                 const char *property,
                                                 sd_bus_message *value,
                                                 sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);

//    if (sd_bus_message_enter_container(value, 'r', "uu") < 0) {
//        return -1;
//    }
//
//    return sd_bus_message_read(value, "uu", &m_purpose, &m_hints);
}

int IBusFrontEnd::ctx_get_content_type (sd_bus *bus,
                                        const char *path,
                                        const char *interface,
                                        const char *property,
                                        sd_bus_message *value,
                                        sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);

//    if (sd_bus_message_open_container(value, 'r', "uu") < 0) {
//        return -1;
//    }
//
//    if (sd_bus_message_append(value, "uu", m_purpose, m_hints) < 0) {
//        return -1;
//    }
//
//    return sd_bus_message_close_container(value);
}

int IBusFrontEnd::ctx_set_content_type (sd_bus *bus,
                                        const char *path,
                                        const char *interface,
                                        const char *property,
                                        sd_bus_message *value,
                                        sd_bus_error *ret_error)
{
    log_func_not_impl (-ENOSYS);

//    if (sd_bus_message_enter_container(value, 'r', "uu") < 0) {
//        return -1;
//    }
//
//    if (sd_bus_message_read(value, "uu", &m_purpose, &m_hints) < 0) {
//        return -1;
//    }
//
//    return 0;
}
/*
vi:ts=4:nowrap:expandtab
*/
