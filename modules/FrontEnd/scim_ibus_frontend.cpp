/** @file scim_ibus_frontend.cpp
 * implementation of class IBusFrontEnd.
 */

#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_FRONTEND
#define Uses_SCIM_PANEL_CLIENT
#define Uses_SCIM_HOTKEY

#include <set>
#include <sstream>
#include <cassert>
#include <cstdarg>
#include <sys/time.h>
#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include "scim_private.h"
#include "scim.h"
#include "scim_ibus_ctx.h"
#include "scim_ibus_frontend.h"
#include "scim_ibus_types.h"
#include "scim_ibus_utils.h"

#define scim_module_init                          ibus_LTX_scim_module_init
#define scim_module_exit                          ibus_LTX_scim_module_exit
#define scim_frontend_module_init                 ibus_LTX_scim_frontend_module_init
#define scim_frontend_module_run                  ibus_LTX_scim_frontend_module_run

#define SCIM_KEYBOARD_ICON_FILE                   (SCIM_ICONDIR "/keyboard.png")

#define IBUS_PORTAL_SERVICE                       "org.freedesktop.portal.IBus"
#define IBUS_PORTAL_OBJECT_PATH                   "/org/freedesktop/IBus"
#define IBUS_PORTAL_INTERFACE                     "org.freedesktop.IBus.Portal"
#define IBUS_INPUTCONTEXT_OBJECT_PATH             "/org/freedesktop/IBus/InputContext_"
#define IBUS_INPUTCONTEXT_INTERFACE               "org.freedesktop.IBus.InputContext"
#define IBUS_SERVICE_INTERFACE                    "org.freedesktop.IBus.Service"
#define IBUS_INPUTCONTEXT_OBJECT_PATH_BUF_SIZE    (STRLEN (IBUS_INPUTCONTEXT_OBJECT_PATH) + 10)

#define STRLEN(s)                                 (sizeof (s) - 1)

using namespace scim;

static Pointer <IBusFrontEnd> _scim_frontend (0);

// XML Introspection Data
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='org.freedesktop.IBus.Portal'>"
  "    <method name='CreateInputContext'>"
  "      <arg type='s' name='client_name' direction='in'/>"
  "      <arg type='o' name='object_path' direction='in'/>"
  "      <arg type='o' name='path' direction='out'/>"
  "    </method>"
  "  </interface>"
  "  <interface name='org.freedesktop.IBus.InputContext'>"
  "    <method name='ProcessKeyEvent'>"
  "      <arg type='u' name='keyval' direction='in'/>"
  "      <arg type='u' name='keycode' direction='in'/>"
  "      <arg type='u' name='state' direction='in'/>"
  "      <arg type='b' name='handled' direction='out'/>"
  "    </method>"
  "    <method name='SetCursorLocation'>"
  "      <arg type='i' name='x' direction='in'/>"
  "      <arg type='i' name='y' direction='in'/>"
  "      <arg type='i' name='w' direction='in'/>"
  "      <arg type='i' name='h' direction='in'/>"
  "    </method>"
  "    <method name='SetCursorLocationRelative'>"
  "      <arg type='i' name='x' direction='in'/>"
  "      <arg type='i' name='y' direction='in'/>"
  "      <arg type='i' name='w' direction='in'/>"
  "      <arg type='i' name='h' direction='in'/>"
  "    </method>"
  "    <method name='ProcessHandWritingEvent'>"
  "      <arg type='ad' name='coordinates' direction='in'/>"
  "    </method>"
  "    <method name='CancelHandWriting'>"
  "      <arg type='u' name='n_strokes' direction='in'/>"
  "    </method>"
  "    <method name='FocusIn'/>"
  "    <method name='FocusOut'/>"
  "    <method name='Reset'/>"
  "    <method name='SetCapabilities'>"
  "      <arg type='u' name='caps' direction='in'/>"
  "    </method>"
  "    <method name='PropertyActivate'>"
  "      <arg type='s' name='name' direction='in'/>"
  "      <arg type='u' name='state' direction='in'/>"
  "    </method>"
  "    <method name='SetEngine'>"
  "      <arg type='s' name='name' direction='in'/>"
  "    </method>"
  "    <method name='GetEngine'>"
  "      <arg type='v' name='desc' direction='out'/>"
  "    </method>"
  "    <method name='SetSurroundingText'>"
  "      <arg type='v' name='text' direction='in'/>"
  "      <arg type='u' name='cursor_pos' direction='in'/>"
  "      <arg type='u' name='anchor_pos' direction='in'/>"
  "    </method>"
  "    <signal name='CommitText'>"
  "      <arg type='v' name='text'/>"
  "    </signal>"
  "    <signal name='ForwardKeyEvent'>"
  "      <arg type='u' name='keyval'/>"
  "      <arg type='u' name='keycode'/>"
  "      <arg type='u' name='state'/>"
  "    </signal>"
  "    <signal name='UpdatePreeditText'>"
  "      <arg type='v' name='text'/>"
  "      <arg type='u' name='cursor_pos'/>"
  "      <arg type='b' name='visible'/>"
  "    </signal>"
  "    <signal name='UpdatePreeditTextWithMode'>"
  "      <arg type='v' name='text'/>"
  "      <arg type='u' name='cursor_pos'/>"
  "      <arg type='b' name='visible'/>"
  "      <arg type='u' name='mode'/>"
  "    </signal>"
  "    <signal name='ShowPreeditText'/>"
  "    <signal name='HidePreeditText'/>"
  "    <signal name='UpdateAuxiliaryTextWithMode'>"
  "      <arg type='v' name='text'/>"
  "      <arg type='b' name='visible'/>"
  "    </signal>"
  "    <signal name='ShowAuxiliaryText'/>"
  "    <signal name='HideAuxiliaryText'/>"
  "    <signal name='UpdateLookupTable'>"
  "      <arg type='v' name='table'/>"
  "      <arg type='b' name='visible'/>"
  "    </signal>"
  "    <signal name='ShowLookupTable'/>"
  "    <signal name='HideLookupTable'/>"
  "    <signal name='PageUpLookupTable'/>"
  "    <signal name='PageDownLookupTable'/>"
  "    <signal name='CursorUpLookupTable'/>"
  "    <signal name='CursorDownLookupTable'/>"
  "    <signal name='RegisterProperties'>"
  "      <arg type='v' name='props'/>"
  "    </signal>"
  "    <signal name='UpdateProperty'>"
  "      <arg type='v' name='prop'/>"
  "    </signal>"
  "    <property name='ClientCommitPreedit' type='(b)' access='readwrite'/>"
  "    <property name='ContentType' type='(uu)' access='readwrite'/>"
  "  </interface>"
  "  <interface name='org.freedesktop.IBus.Service'>"
  "    <method name='Destroy'/>"
  "  </interface>"
  "</node>";

GDBusNodeInfo *ibus_introspection_data = NULL;

IBusFrontEnd::IBusFrontEnd (const BackEndPointer &backend,
                            const ConfigPointer  &config)
    : FrontEndBase (backend),
      m_config (config),
      m_current_instance (-1),
      m_current_ibus_ctx (NULL),
      m_ctx_counter (0),
      m_loop (NULL),
      m_bus (NULL),
      m_portal_id (0),
      m_match_id (0),
      m_name_owner_id (0),
      m_panel_source (0)
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

    IBusIDCtxMap::iterator it = m_id_ctx_map.begin ();
    for (; it != m_id_ctx_map.end (); ++ it) {
        delete it->second;
    }

    panel_disconnect ();

    if (m_match_id > 0 && m_bus) {
        g_dbus_connection_signal_unsubscribe (m_bus, m_match_id);
    }
    
    if (m_name_owner_id > 0) {
        g_bus_unown_name (m_name_owner_id);
    }

    if (m_portal_id > 0 && m_bus) {
        g_dbus_connection_unregister_object (m_bus, m_portal_id);
    }

    if (m_bus) {
        g_object_unref (m_bus);
    }

    if (m_loop) {
        g_main_loop_unref (m_loop);
    }
}

// Global VTable for Portal Interface
static const GDBusInterfaceVTable portal_vtable = {
    IBusFrontEnd::portal_method_call,
    NULL,
    NULL,
    { 0 }
};

void
IBusFrontEnd::init (int argc, char **argv)
{
    log_func();
    GError *error = NULL;

    reload_config_callback (m_config);

    if (!m_config.null ()) {
        m_config->signal_connect_reload (slot (this, &IBusFrontEnd::reload_config_callback));
    }

    // Create Main Loop
    m_loop = g_main_loop_new (NULL, FALSE);
    if (!m_loop) {
        throw FrontEndError ("IBusFrontEnd -- Cannot create event loop.");
    }

    // Connect to Session Bus
    m_bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (!m_bus) {
        String msg = String ("IBusFrontEnd -- Cannot connect to session bus: ") + error->message;
        g_error_free (error);
        throw FrontEndError (msg);
    }

    // Parse Introspection Data
    ibus_introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, &error);
    if (!ibus_introspection_data) {
         String msg = String ("IBusFrontEnd -- Failed to parse introspection data: ") + error->message;
         g_error_free (error);
         throw FrontEndError (msg);
    }

    // Own the name
    m_name_owner_id = g_bus_own_name_on_connection (m_bus,
                                                   IBUS_PORTAL_SERVICE,
                                                   G_BUS_NAME_OWNER_FLAGS_REPLACE,
                                                   NULL, NULL, NULL, NULL);

    // Register Portal Object
    GDBusInterfaceInfo *portal_info = g_dbus_node_info_lookup_interface (ibus_introspection_data, IBUS_PORTAL_INTERFACE);
    m_portal_id = g_dbus_connection_register_object (m_bus,
                                                     IBUS_PORTAL_OBJECT_PATH,
                                                     portal_info,
                                                     &portal_vtable,
                                                     this,
                                                     NULL,
                                                     &error);
    if (m_portal_id == 0) {
        String msg = String ("IBus -- failed to add portal object: ") + error->message;
        g_error_free (error);
        throw FrontEndError (msg);
    }

    // Add match for NameOwnerChanged
    m_match_id = g_dbus_connection_signal_subscribe (m_bus,
                                                     "org.freedesktop.DBus",
                                                     "org.freedesktop.DBus",
                                                     "NameOwnerChanged",
                                                     "/org/freedesktop/DBus",
                                                     NULL,
                                                     G_DBUS_SIGNAL_FLAGS_NONE,
                                                     on_name_owner_changed,
                                                     this,
                                                     NULL);

    if (panel_connect() < 0) {
        throw FrontEndError (String ("IBus -- failed to connect to the panel daemon!"));
    }
}

void
IBusFrontEnd::run ()
{
    log_func();

    if (m_loop) {
        g_main_loop_run (m_loop);
    }
}

// ... helper methods for instance generation ...

int
IBusFrontEnd::generate_ctx_id ()
{
    return ++ m_ctx_counter;
}

void
IBusFrontEnd::reload_config_callback (const ConfigPointer &config)
{
    log_func();

    SCIM_DEBUG_FRONTEND (1) << "Reload configuration.\n";

    m_frontend_hotkey_matcher.load_hotkeys (config);
    m_imengine_hotkey_matcher.load_hotkeys (config);
}

// Portal Interface Implementation

void
IBusFrontEnd::portal_method_call (GDBusConnection       *connection,
                                  const gchar           *sender,
                                  const gchar           *object_path,
                                  const gchar           *interface_name,
                                  const gchar           *method_name,
                                  GVariant              *parameters,
                                  GDBusMethodInvocation *invocation,
                                  gpointer               user_data)
{
    IBusFrontEnd *self = (IBusFrontEnd *) user_data;
    if (g_strcmp0 (method_name, "CreateInputContext") == 0) {
        self->portal_create_ctx (invocation, parameters);
    }
}

void
IBusFrontEnd::portal_create_ctx (GDBusMethodInvocation *invocation, GVariant *parameters)
{
    log_func();

    const char *client_name;
    const char *object_path;
    g_variant_get (parameters, "(&s&o)", &client_name, &object_path);

    String locale = scim_get_current_locale ();
    String encoding = scim_get_locale_encoding (locale);
    String language = scim_get_current_language ();
    String sfid = get_default_factory (language, encoding);

    log_debug ("locale=%s", locale.c_str ());

    int siid = new_instance (sfid, encoding);

    int id = generate_ctx_id ();
    char path[IBUS_INPUTCONTEXT_OBJECT_PATH_BUF_SIZE];
    snprintf(path, sizeof(path), IBUS_INPUTCONTEXT_OBJECT_PATH "%d", id);

    log_debug ("instance name=%s, authors=%s, encoding=%s, uuid=%s",
            utf8_wcstombs (get_instance_name (siid)).c_str(),
            utf8_wcstombs (get_instance_authors (siid)).c_str(),
            get_instance_encoding (siid).c_str (),
            get_instance_uuid (siid).c_str ());

    IBusCtx *ctx = new IBusCtx (g_dbus_method_invocation_get_sender (invocation),
                                locale,
                                id,
                                siid);
    if (ctx->init (m_bus, path) < 0) {
        delete ctx;
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Failed to initialize InputContext");
        return;
    }

    m_panel_client.prepare (id);
    m_panel_client.register_input_context (id, get_instance_uuid (siid));
    m_panel_client.send ();

    m_id_ctx_map [id] = ctx;
    m_siid_ctx_map [siid] = ctx;
    m_name_ctx_map [g_dbus_method_invocation_get_sender (invocation)].insert (ctx);

    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(o)", path));
}

// Input Context Interface VTable Logic

void
IBusFrontEnd::input_context_method_call (GDBusConnection       *connection,
                                         const gchar           *sender,
                                         const gchar           *object_path,
                                         const gchar           *interface_name,
                                         const gchar           *method_name,
                                         GVariant              *parameters,
                                         GDBusMethodInvocation *invocation,
                                         gpointer               user_data)
{
    IBusCtx *ctx = (IBusCtx *) user_data;

    // Safety check? The user_data is what we passed in g_dbus_connection_register_object
    if (!ctx) return;

    // We access the frontend instance from global for dispatch, but we need member access.
    // However, IBusFrontEnd methods are members. We need access to _scim_frontend.
    if (_scim_frontend.null()) return;

    if (g_strcmp0 (interface_name, "org.freedesktop.IBus.Service") == 0) {
        if (g_strcmp0 (method_name, "Destroy") == 0) {
            _scim_frontend->destroy_ctx (ctx);
            g_dbus_method_invocation_return_value (invocation, NULL);
        }
        return;
    }

    if (g_strcmp0 (method_name, "ProcessKeyEvent") == 0) {
        _scim_frontend->ctx_process_key_event (ctx, invocation, parameters);
    } else if (g_strcmp0 (method_name, "SetCursorLocation") == 0) {
        _scim_frontend->ctx_set_cursor_location (ctx, invocation, parameters);
    } else if (g_strcmp0 (method_name, "SetCursorLocationRelative") == 0) {
        _scim_frontend->ctx_set_cursor_location_relative (ctx, invocation, parameters);
    } else if (g_strcmp0 (method_name, "FocusIn") == 0) {
        _scim_frontend->ctx_focus_in (ctx, invocation);
    } else if (g_strcmp0 (method_name, "FocusOut") == 0) {
        _scim_frontend->ctx_focus_out (ctx, invocation);
    } else if (g_strcmp0 (method_name, "Reset") == 0) {
        _scim_frontend->ctx_reset (ctx, invocation);
    } else if (g_strcmp0 (method_name, "SetCapabilities") == 0) {
        _scim_frontend->ctx_set_capabilities (ctx, invocation, parameters);
    } else if (g_strcmp0 (method_name, "PropertyActivate") == 0) {
        _scim_frontend->ctx_property_activate (ctx, invocation, parameters);
    } else if (g_strcmp0 (method_name, "SetEngine") == 0) {
        _scim_frontend->ctx_set_engine (ctx, invocation, parameters);
    } else if (g_strcmp0 (method_name, "GetEngine") == 0) {
        _scim_frontend->ctx_get_engine (ctx, invocation);
    } else if (g_strcmp0 (method_name, "SetSurroundingText") == 0) {
        _scim_frontend->ctx_set_surrounding_text (ctx, invocation, parameters);
    } else if (g_strcmp0 (method_name, "ProcessHandWritingEvent") == 0) {
        _scim_frontend->ctx_process_hand_writing_event (ctx, invocation, parameters);
    } else if (g_strcmp0 (method_name, "CancelHandWriting") == 0) {
        _scim_frontend->ctx_cancel_hand_writing (ctx, invocation, parameters);
    }
}

GVariant *
IBusFrontEnd::input_context_get_property (GDBusConnection       *connection,
                                          const gchar           *sender,
                                          const gchar           *object_path,
                                          const gchar           *interface_name,
                                          const gchar           *property_name,
                                          GError               **error,
                                          gpointer               user_data)
{
    IBusCtx *ctx = (IBusCtx *) user_data;
    if (_scim_frontend.null()) return NULL;

    if (g_strcmp0 (property_name, "ClientCommitPreedit") == 0) {
        return _scim_frontend->ctx_get_client_commit_preedit (ctx);
    } else if (g_strcmp0 (property_name, "ContentType") == 0) {
        return _scim_frontend->ctx_get_content_type (ctx);
    }

    return NULL;
}

gboolean
IBusFrontEnd::input_context_set_property (GDBusConnection       *connection,
                                          const gchar           *sender,
                                          const gchar           *object_path,
                                          const gchar           *interface_name,
                                          const gchar           *property_name,
                                          GVariant              *value,
                                          GError               **error,
                                          gpointer               user_data)
{
    IBusCtx *ctx = (IBusCtx *) user_data;
    if (_scim_frontend.null()) return FALSE;

    if (g_strcmp0 (property_name, "ClientCommitPreedit") == 0) {
        return _scim_frontend->ctx_set_client_commit_preedit (ctx, value);
    } else if (g_strcmp0 (property_name, "ContentType") == 0) {
        return _scim_frontend->ctx_set_content_type (ctx, value);
    }

    return FALSE;
}

// Internal Method Handlers

void
IBusFrontEnd::ctx_process_key_event(IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters)
{
    log_func ();

    uint32_t keyval = 0;
    uint32_t keycode = 0;
    uint32_t state = 0;

    g_variant_get (parameters, "(uuu)", &keyval, &keycode, &state);

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
    m_panel_client.send ();

    m_current_instance = -1;

    String eventstr;
    log_debug ("%s %s",
               (scim_key_to_string (eventstr, event), eventstr).c_str (),
               consumed ? "consumed" : "ignored");

    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(b)", consumed));
}

void
IBusFrontEnd::ctx_set_cursor_location(IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters)
{
    log_func ();

    if (ctx->cursor_location_from_variant (parameters) < 0) {
        // Handle error?
    }

    m_panel_client.prepare (ctx->id ());
    panel_req_update_spot_location (ctx);
    m_panel_client.send ();

    g_dbus_method_invocation_return_value (invocation, NULL);
}

void
IBusFrontEnd::ctx_set_cursor_location_relative(IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters)
{
    log_func ();

    if (ctx->cursor_location_relative_from_variant (parameters) < 0) {
        // Handle error
    }

    m_panel_client.prepare (ctx->id ());
    panel_req_update_spot_location (ctx);
    m_panel_client.send ();

    g_dbus_method_invocation_return_value (invocation, NULL);
}

void
IBusFrontEnd::ctx_process_hand_writing_event(IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters)
{
    log_func_ignored ();
    g_dbus_method_invocation_return_value (invocation, NULL);
}

void
IBusFrontEnd::ctx_cancel_hand_writing(IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters)
{
    log_func_ignored ();
    g_dbus_method_invocation_return_value (invocation, NULL);
}

void
IBusFrontEnd::ctx_property_activate(IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters)
{
    log_func_ignored ();
    g_dbus_method_invocation_return_value (invocation, NULL);
}

void
IBusFrontEnd::ctx_set_engine(IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters)
{
    log_func_ignored ();
    g_dbus_method_invocation_return_value (invocation, NULL);
}

void
IBusFrontEnd::ctx_get_engine(IBusCtx *ctx, GDBusMethodInvocation *invocation)
{
    log_func_ignored ();
    // TODO: Return proper dummy value?
    g_dbus_method_invocation_return_value (invocation, g_variant_new ("(v)", g_variant_new_string("")));
}

void
IBusFrontEnd::ctx_set_surrounding_text(IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters)
{
    log_func_ignored ();
    g_dbus_method_invocation_return_value (invocation, NULL);
}

void
IBusFrontEnd::ctx_focus_in(IBusCtx *ctx, GDBusMethodInvocation *invocation)
{
    log_func ();

    uint32 siid = ctx->siid();

    m_current_ibus_ctx = ctx;
    m_current_instance = siid;

    focus_in (siid); 

    m_panel_client.prepare (ctx->id ());
    panel_req_focus_in (ctx);
    if (ctx->is_on ()) {
        start_ctx (ctx);
    }
    m_panel_client.send ();

    m_current_instance = -1;

    g_dbus_method_invocation_return_value (invocation, NULL);
}

void
IBusFrontEnd::ctx_focus_out(IBusCtx *ctx, GDBusMethodInvocation *invocation)
{
    log_func ();

    uint32 siid = ctx->siid ();

    m_current_instance = siid;

    m_panel_client.prepare (ctx->id ());
    m_panel_client.focus_out (ctx-> id());
    m_panel_client.send ();

    focus_out (siid); 

    m_current_ibus_ctx = NULL;
    m_current_instance = -1;

    g_dbus_method_invocation_return_value (invocation, NULL);
}

void
IBusFrontEnd::ctx_reset(IBusCtx *ctx, GDBusMethodInvocation *invocation)
{
    log_func ();

    uint32 siid = ctx->siid ();

    m_current_instance = siid;

    reset (siid); 

    ctx->preedit_reset ();

    m_current_instance = -1;

    g_dbus_method_invocation_return_value (invocation, NULL);
}

void
IBusFrontEnd::ctx_set_capabilities(IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters)
{
    log_func ();

    SCIM_DEBUG_FRONTEND (3) << "  SI (" << ctx->siid () << ").\n";

    if (ctx->caps_from_variant (parameters) < 0) {
        // error
    }

    int siid = ctx->siid ();

    m_current_instance = siid;
    update_client_capabilities (siid, ctx->scim_caps ()); 
    m_current_instance = -1;

    log_debug("IBus caps=%s => SCIM caps=%s",
              ibus_caps_to_str (ctx->caps()).c_str(),
              scim_caps_to_str (ctx->scim_caps()).c_str());

    g_dbus_method_invocation_return_value (invocation, NULL);
}

GVariant*
IBusFrontEnd::ctx_get_client_commit_preedit (IBusCtx *ctx)
{
    log_func ();
    return ctx->client_commit_preedit_to_variant ();
}

gboolean
IBusFrontEnd::ctx_set_client_commit_preedit (IBusCtx *ctx, GVariant *value)
{
    log_func ();
    return ctx->client_commit_preedit_from_variant (value) == 0;
}

GVariant*
IBusFrontEnd::ctx_get_content_type (IBusCtx *ctx)
{
    log_func ();
    return ctx->content_type_to_variant ();
}

gboolean
IBusFrontEnd::ctx_set_content_type (IBusCtx *ctx, GVariant *value)
{
    log_func ();
    return ctx->content_type_from_variant (value) == 0;
}

// Signal Handler for NameOwnerChanged
void
IBusFrontEnd::on_name_owner_changed (GDBusConnection *connection,
                                     const gchar     *sender_name,
                                     const gchar     *object_path,
                                     const gchar     *interface_name,
                                     const gchar     *signal_name,
                                     GVariant        *parameters,
                                     gpointer         user_data)
{
    IBusFrontEnd *self = (IBusFrontEnd *) user_data;
    const gchar *name, *old_owner, *new_owner;
    g_variant_get (parameters, "(&s&s&s)", &name, &old_owner, &new_owner);

    if (g_strcmp0 (new_owner, "") == 0) {
        // Name lost
        self->client_destroy (name);
    }
}


void
IBusFrontEnd::client_destroy (const char *name)
{
    log_func();

    IBusNameCtxMap::iterator ctx_set = m_name_ctx_map.find (name);
    if (ctx_set == m_name_ctx_map.end ()) {
        return;
    }

    IBusCtxSet::iterator ctx_it = ctx_set->second.begin ();
    // Need to collect contexts first to avoid iterator invalidation during erase?
    // destroy_ctx removes from maps.
    // Yes, destroy_ctx erases from m_name_ctx_map[name]?
    // No, destroy_ctx logic in scim_ibus_frontend.cpp:
    // "m_name_ctx_map[sd_bus_message_get_sender (m)].erase (ctx);" was in srv_destroy but not in destroy_ctx logic in old code?
    // Let's check old code destroy_ctx:
    /*
    void IBusFrontEnd::destroy_ctx (IBusCtx *ctx) {
        ...
        m_id_ctx_map.erase (ctx->id ());
        m_siid_ctx_map.erase (ctx->siid ());
        delete ctx;
    }
    */
    // It did NOT erase from m_name_ctx_map! That seems like a bug or handled by caller?
    // srv_destroy did erase from m_name_ctx_map.
    // client_destroy in old code did:
    /*
    IBusNameCtxMap::iterator ctx_set = m_name_ctx_map.find (name);
    for (; ctx_it != ctx_set->second.end (); ctx_it ++) {
        destroy_ctx (*ctx_it);
    }
    m_name_ctx_map.erase (ctx_set);
    */
    // This implies destroy_ctx does NOT touch m_name_ctx_map.

    std::vector<IBusCtx*> ctxs_to_delete;
    for (ctx_it = ctx_set->second.begin (); ctx_it != ctx_set->second.end (); ++ctx_it) {
        ctxs_to_delete.push_back(*ctx_it);
    }

    for (size_t i = 0; i < ctxs_to_delete.size(); i++) {
        destroy_ctx(ctxs_to_delete[i]);
    }

    m_name_ctx_map.erase (ctx_set);
}

// Serialization Helpers

static GVariant* serialize_text (const WideString &wstr, const AttributeList &attrs)
{
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("a(uuuu)"));

    AttributeList::const_iterator it = attrs.begin ();
    for (; it != attrs.end(); it++) {
        IBusAttribute attr;
        if (scim_attr_to_ibus_attr (*it, attr)) {
            g_variant_builder_add (&builder, "(uuuu)", attr.type, attr.value, attr.start_index, attr.end_index);
        }
    }

    String utf8 = utf8_wcstombs (wstr);

    return g_variant_new ("(sa{sv}sv)",
                          "IBusText",
                          NULL, // annotations
                          utf8.c_str(),
                          g_variant_builder_end (&builder) // attributes -> variant?
                          // Wait, signature is (sa{sv}sv). Last v is the attribute list.
                          // But wait, attribute list structure?
                          // Old code:
                          // serialize_attribute_list opens container 'v' containing "(sa{sv}av)"
                          // Inside: "IBusAttrList", annotations, array of attributes (uuuu)
    );
}

// Wait, complex serialization of IBus objects (IBusText, IBusAttribute) into GVariant 'v' slots.
// IBus uses a convention where an Object is (sa{sv}v) or similar?
// Old code:
/*
serialize_text:
  open variant "(sa{sv}sv)"
    open struct "sa{sv}sv"
      "IBusText"
      annotations (array dict)
      utf8_string
      serialize_attribute_list -> returns 'v' containing "IBusAttrList"
*/

static GVariant* serialize_attribute_list (const AttributeList &attrs)
{
    GVariantBuilder array_builder;
    g_variant_builder_init (&array_builder, G_VARIANT_TYPE ("av")); // Array of variants (attributes)

    AttributeList::const_iterator it = attrs.begin ();
    for (; it != attrs.end(); it++) {
        IBusAttribute attr;
        if (scim_attr_to_ibus_attr (*it, attr)) {
            // IBusAttribute is (sa{sv}uuuu) wrapped in 'v'
            GVariant *attr_val = g_variant_new ("(sa{sv}uuuu)",
                                                "IBusAttribute",
                                                NULL, // annotations
                                                attr.type, attr.value, attr.start_index, attr.end_index);
            g_variant_builder_add (&array_builder, "v", attr_val);
        }
    }

    return g_variant_new ("(sa{sv}av)", "IBusAttrList", NULL, &array_builder);
}

static GVariant* serialize_ibus_text (const WideString &wstr, const AttributeList &attrs)
{
    String utf8 = utf8_wcstombs (wstr);
    GVariant *attr_list_variant = serialize_attribute_list (attrs);

    return g_variant_new ("(sa{sv}sv)",
                          "IBusText",
                          NULL,
                          utf8.c_str(),
                          attr_list_variant);
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
    snprintf(path, sizeof(path), IBUS_INPUTCONTEXT_OBJECT_PATH "%d", ctx->id ());

    GVariant *parameters = NULL;
    va_list args;
    va_start (args, signal);

    if (strcmp ("ForwardKeyEvent", signal) == 0) {
        KeyEvent &event = *va_arg (args, KeyEvent *);
        parameters = g_variant_new ("(uuu)",
                                    event.code,
                                    0,
                                    scim_keymask_to_ibus_keystate (event.mask) | IBUS_FORWARD_MASK);
    } else if (strcmp ("UpdatePreeditTextWithMode", signal) == 0) {
        const WideString &str = *va_arg (args, const WideString *);
        const AttributeList &attrs = *va_arg (args, const AttributeList *);
        uint32_t caret_pos = va_arg (args, uint32_t);
        bool visible = (bool) !!va_arg (args, int);
        uint32_t mode = va_arg (args, uint32_t);

        parameters = g_variant_new ("(vubu)",
                                    serialize_ibus_text (str, attrs),
                                    caret_pos,
                                    visible,
                                    mode);
    } else if (strcmp ("CommitText", signal) == 0) {
        const WideString &wstr = *va_arg (args, const WideString *);
        parameters = g_variant_new ("(v)",
                                    serialize_ibus_text (wstr, AttributeList()));
    } else if (strcmp ("ShowPreeditText", signal) == 0 ||
               strcmp ("HidePreeditText", signal) == 0 ||
               strcmp ("ShowAuxiliaryText", signal) == 0 ||
               strcmp ("HideAuxiliaryText", signal) == 0) {
        parameters = NULL; // No args
    } else {
        // Ignored or unknown
        va_end (args);
        return;
    }
    va_end (args);

    GError *error = NULL;
    g_dbus_connection_emit_signal (m_bus,
                                   ctx->owner ().c_str (),
                                   path,
                                   IBUS_INPUTCONTEXT_INTERFACE,
                                   signal,
                                   parameters,
                                   &error);

    if (error) {
        log_warn ("Failed to emit signal %s: %s", signal, error->message);
        g_error_free (error);
    }
}

// Helper methods from old file
IBusCtx *
IBusFrontEnd::find_ctx_by_siid (int siid) const
{
    IBusIDCtxMap::const_iterator it = m_siid_ctx_map.find (siid);
    if (it == m_siid_ctx_map.end ()) {
        return NULL;
    }

    return it->second;
}

IBusCtx *
IBusFrontEnd::find_ctx (int id) const
{
    IBusIDCtxMap::const_iterator it = m_id_ctx_map.find (id);
    if (it == m_id_ctx_map.end ()) {
        return NULL;
    }

    return it->second;
}

IBusCtx *
IBusFrontEnd::find_ctx (const char *path) const
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
        return NULL;
    }

    return _scim_frontend->find_ctx (id);
}

inline bool
IBusFrontEnd::validate_ctx (IBusCtx *ctx) const {
    return ctx != NULL &&
           m_id_ctx_map.find (ctx->id ()) != m_id_ctx_map.end ();
}

void
IBusFrontEnd::start_ctx (IBusCtx *ctx)
{
    log_func ();

    if (validate_ctx (ctx)) {

        ctx->on ();

        panel_req_update_spot_location (ctx);
        panel_req_update_factory_info (ctx);

        m_panel_client.turn_on (ctx->id ());
        m_panel_client.hide_lookup_table (ctx->id ());
        signal_ctx (ctx->siid (), "HidePreeditText");
        signal_ctx (ctx->siid (), "HideAuxiliaryText");

        if (ctx->shared_siid ()) reset (ctx->siid ());
        focus_in (ctx->siid ());
    }
}

void
IBusFrontEnd::stop_ctx (IBusCtx *ctx)
{
    log_func ();

    if (validate_ctx (ctx)) {
        focus_out (ctx->siid ());
        if (ctx->shared_siid ()) reset (ctx->siid ());

        signal_ctx (ctx->siid (), "HidePreeditText");
        signal_ctx (ctx->siid (), "HideAuxiliaryText");
        m_panel_client.hide_lookup_table (ctx->id ());
        m_panel_client.turn_off (ctx->id ());

        panel_req_update_factory_info (ctx);

        ctx->off ();
    }
}

void
IBusFrontEnd::destroy_ctx (IBusCtx *ctx)
{
    log_func ();

    if (m_id_ctx_map.find (ctx->id ()) != m_id_ctx_map.end ())  {
        m_id_ctx_map.erase (ctx->id ());
    }

    if (m_current_instance == ctx->siid ()) {
        m_current_instance = -1;
    }

    if (ctx == m_current_ibus_ctx) {
        m_current_ibus_ctx = NULL;
    }

    m_panel_client.prepare (ctx->id ());
    m_panel_client.remove_input_context (ctx->id ());
    m_panel_client.send ();

    m_id_ctx_map.erase (ctx->id ());
    m_siid_ctx_map.erase (ctx->siid ());

    // We also need to unregister object from bus
    // Since we don't have the object ID easily here (unless we stored it in IBusCtx),
    // we updated IBusCtx to handle unregistration in its destructor.

    delete ctx;
}


// Panel Connection Helpers using GIOChannel

int
IBusFrontEnd::panel_connect ()
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
    GIOChannel *channel = g_io_channel_unix_new (fd);
    m_panel_source = g_io_add_watch (channel, G_IO_IN, panel_io_func, this);
    g_io_channel_unref (channel); // g_io_add_watch takes a reference

    return 0;
}

void
IBusFrontEnd::panel_disconnect ()
{
    log_func();

    if (m_panel_source > 0) {
        g_source_remove (m_panel_source);
        m_panel_source = 0;
    }

    if (m_panel_client.is_connected()) {
        m_panel_client.close_connection();
    }
}

gboolean
IBusFrontEnd::panel_io_func (GIOChannel *source, GIOCondition condition, gpointer data)
{
    IBusFrontEnd *self = (IBusFrontEnd *) data;
    return self->panel_handle_io (NULL, 0, 0) == 0;
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

// Frontend virtual method implementations (mostly unchanged, just calling internal methods)

void IBusFrontEnd::show_preedit_string (int siid) { signal_ctx (siid, "ShowPreeditText"); }
void IBusFrontEnd::show_aux_string (int siid) { signal_ctx (siid, "ShowAuxiliaryText"); }
void IBusFrontEnd::show_lookup_table (int siid) {
    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.show_lookup_table (ctx->id ());
        m_panel_client.send ();
    }
}
void IBusFrontEnd::hide_preedit_string (int siid) { signal_ctx (siid, "HidePreeditText"); }
void IBusFrontEnd::hide_aux_string (int siid) { signal_ctx (siid, "HideAuxiliaryText"); }
void IBusFrontEnd::hide_lookup_table (int siid) {
    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.hide_lookup_table (ctx->id ());
        m_panel_client.send ();
    }
}
void IBusFrontEnd::update_preedit_caret (int siid, int caret) {
    m_current_ibus_ctx->preedit_caret (caret);
    signal_ctx (siid, "UpdatePreeditTextWithMode", &m_current_ibus_ctx->preedit_text (), &m_current_ibus_ctx->preedit_attrs (), m_current_ibus_ctx->preedit_caret (), true, IBUS_ENGINE_PREEDIT_CLEAR);
}
void IBusFrontEnd::update_preedit_string (int siid, const WideString & str, const AttributeList & attrs) {
    m_current_ibus_ctx->preedit_text (str);
    m_current_ibus_ctx->preedit_attrs (attrs);
    signal_ctx (siid, "UpdatePreeditTextWithMode", &str, &attrs, m_current_ibus_ctx->preedit_caret (), true, IBUS_ENGINE_PREEDIT_CLEAR);
}
void IBusFrontEnd::update_aux_string (int siid, const WideString & str, const AttributeList & attrs) {
    signal_ctx (siid, "UpdateAuxiliaryText", &str, &attrs);
}
void IBusFrontEnd::commit_string (int siid, const WideString & str) {
    signal_ctx (siid, "CommitText", &str);
    m_current_ibus_ctx->preedit_reset();
}
void IBusFrontEnd::forward_key_event (int siid, const KeyEvent & key) { signal_ctx (siid, "ForwardKeyEvent", &key); }
void IBusFrontEnd::update_lookup_table (int siid, const LookupTable & table) {
    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.update_lookup_table (ctx->id (), table);
        m_panel_client.send ();
    }
}
void IBusFrontEnd::register_properties (int siid, const PropertyList &properties) {
    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.register_properties (ctx->id (), properties);
        m_panel_client.send ();
    }
}
void IBusFrontEnd::update_property (int siid, const Property &property) {
    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.update_property (ctx->id (), property);
        m_panel_client.send ();
    }
}
void IBusFrontEnd::beep (int siid) {}
void IBusFrontEnd::start_helper (int siid, const String &helper_uuid) {
    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (m_current_instance == siid && validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.start_helper (ctx->id (), helper_uuid);
        m_panel_client.send();
    }
}
void IBusFrontEnd::stop_helper (int siid, const String &helper_uuid) {
    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (m_current_instance == siid && validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.stop_helper (ctx->id (), helper_uuid);
        m_panel_client.send();
    }
}
void IBusFrontEnd::send_helper_event (int siid, const String &helper_uuid, const Transaction &trans) {
    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (m_current_instance == siid && validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.send_helper_event (ctx->id (), helper_uuid, trans);
        m_panel_client.send();
    }
}
bool IBusFrontEnd::get_surrounding_text (int siid, WideString &text, int &cursor, int maxlen_before, int maxlen_after) { return false; }
bool IBusFrontEnd::delete_surrounding_text (int siid, int offset, int len) { return false; }


// Panel Slots Implementation

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
    g_main_loop_quit (m_loop);
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
                    signal_ctx (ctx->siid (), "ForwardKeyEvent", &key);
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
    }
}
void
IBusFrontEnd::panel_slot_forward_key_event (int context, const KeyEvent &key)
{
    log_func ();
    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        forward_key_event (ctx->id (), key);
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
    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        if (uuid.length () == 0 && ctx->is_on ()) {
            stop_ctx (ctx);
        }else if(uuid.length () == 0 && (ctx->is_on ()  == false)){
    		panel_req_update_factory_info (ctx);
        	m_panel_client.turn_off (ctx->id ());        	
        }else if (uuid.length ()) {
            String encoding = scim_get_locale_encoding (ctx->locale ());
            String language = scim_get_locale_language (ctx->locale ());
            if (validate_factory (uuid, encoding)) {
                stop_ctx (ctx);
                replace_instance (ctx->siid (), uuid);
                m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
                set_default_factory (language, uuid);
                start_ctx (ctx);
            }
        }
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::panel_req_update_screen (const IBusCtx *ctx)
{
    log_func_not_impl ();
    m_panel_client.update_screen (ctx->id (), 0);
}

void
IBusFrontEnd::panel_req_show_help (const IBusCtx *ctx)
{
    log_func ();
    String help;
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
IBusFrontEnd::panel_req_focus_out (const IBusCtx *ctx)
{
    // Not used in old code
}

void
IBusFrontEnd::panel_req_update_factory_info (const IBusCtx *ctx)
{
    log_func ();
    if (!is_current_ctx (ctx)) {
        return;
    }
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
}

void
IBusFrontEnd::panel_req_update_spot_location (const IBusCtx *ctx)
{
    log_func ();
    m_panel_client.update_spot_location (ctx->id (),
                                         ctx->cursor_location ().x,
                                         ctx->cursor_location ().y +
                                             ctx->cursor_location ().h);
}

bool
IBusFrontEnd::filter_hotkeys (IBusCtx *ctx, const KeyEvent &scimkey)
{
    bool ok = false;
    if (!is_current_ctx (ctx)) return false;

    m_frontend_hotkey_matcher.push_key_event (scimkey);
    m_imengine_hotkey_matcher.push_key_event (scimkey);
    FrontEndHotkeyAction hotkey_action = m_frontend_hotkey_matcher.get_match_result ();

    if (hotkey_action == SCIM_FRONTEND_HOTKEY_TRIGGER) {
        if (!ctx->is_on ()) start_ctx (ctx); else stop_ctx (ctx); ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_ON) {
        if (!ctx->is_on ()) start_ctx (ctx); ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_OFF) {
        if (ctx->is_on ()) stop_ctx (ctx); ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY) {
        // ... (implementation same as old) ...
        String encoding = scim_get_locale_encoding (ctx->locale ());
        String language = scim_get_locale_language (ctx->locale ());
        String sfid = get_next_factory ("", encoding, get_instance_uuid (ctx->siid ()));
        if (validate_factory (sfid, encoding)) {
            stop_ctx (ctx);
            replace_instance (ctx->siid (), sfid);
            m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
            set_default_factory (language, sfid);
            start_ctx (ctx);
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY) {
        String encoding = scim_get_locale_encoding (ctx->locale ());
        String language = scim_get_locale_language (ctx->locale ());
        String sfid = get_previous_factory ("", encoding, get_instance_uuid (ctx->siid ()));
        if (validate_factory (sfid, encoding)) {
            stop_ctx (ctx);
            replace_instance (ctx->siid (), sfid);
            m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
            set_default_factory (language, sfid);
            start_ctx (ctx);
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) {
        panel_req_show_factory_menu (ctx);
        ok = true;
    } else if (m_imengine_hotkey_matcher.is_matched ()) {
        String encoding = scim_get_locale_encoding (ctx->locale ());
        String language = scim_get_locale_language (ctx->locale ());
        String sfid = m_imengine_hotkey_matcher.get_match_result ();
        if (validate_factory (sfid, encoding)) {
            stop_ctx (ctx);
            replace_instance (ctx->siid (), sfid);
            m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
            set_default_factory (language, sfid);
            start_ctx (ctx);
        }
        ok = true;
    }
    return ok;
}

// Module Entry Points
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
            _scim_frontend->init (argc, argv);
        }
    }

    void scim_frontend_module_run (void)
    {
        if (!_scim_frontend.null ()) {
            SCIM_DEBUG_FRONTEND(1) << "Starting Socket FrontEnd module...\n";
            _scim_frontend->run ();
        }
    }
}
