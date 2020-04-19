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
 * $Id: scim_ibus_frontend.cpp,v 1.179.2.6 2007/06/16 06:23:38 suzhe Exp $
 *
 */
#include <systemd/sd-bus.h>
#include <sstream>
#include <vector>
#include <scim.h>
#include <scim_event.h>
#include "scim_ibus_types.h"
#include "scim_ibus_utils.h"
#include "scim_ibus_ic.h"

#define IBUS_INPUTCONTEXT_OBJECT_PATH             "/org/freedesktop/IBus/InputContext_"
#define IBUS_INPUTCONTEXT_INTERFACE               "org.freedesktop.IBus.InputContext"
#define IBUS_SERVICE_INTERFACE                    "org.freedesktop.IBus.Service"


using namespace std;

template<int (IBusInputContext::*mf)(sd_bus_message *m, sd_bus_error *ret_error)>
int
ibus_inputcontext_message_adapter(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    return sd_bus_message_adapter<IBusInputContext, mf>(m, userdata, ret_error);
}

template<int (IBusInputContext::*mf)(sd_bus *bus,
 	                                 const char *path,
 	                                 const char *interface,
 	                                 const char *property,
 	                                 sd_bus_message *value,
 	                                 sd_bus_error *ret_error)>
int
ibus_inputcontext_prop_adapter(sd_bus *bus,
 	                           const char *path,
 	                           const char *interface,
 	                           const char *property,
 	                           sd_bus_message *value,
 	                           void *userdata,
 	                           sd_bus_error *ret_error)
{
    return sd_bus_prop_adapter<IBusInputContext, mf>(bus, path, interface, property, value, userdata, ret_error);
}

template<int (IBusInputContext::*mf)(sd_event_source *s)>
int
ibus_inputcontext_event_adapter(sd_event_source *s, void *userdata)
{
    return sd_event_adapter<IBusInputContext, mf>(s, userdata);
}

const sd_bus_vtable IBusInputContext::m_inputcontext_vtbl[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_NAMES(
            "ProcessKeyEvent",
            "uuu", SD_BUS_PARAM(keyval) SD_BUS_PARAM(keycode) SD_BUS_PARAM(state),
            "b", SD_BUS_PARAM(handled),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::process_key_event>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCursorLocation",
            "iiii", SD_BUS_PARAM(x) SD_BUS_PARAM(y) SD_BUS_PARAM(w) SD_BUS_PARAM(h),
            "", SD_BUS_PARAM(),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::set_cursor_location>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCursorLocationRelative",
            "iiii", SD_BUS_PARAM(x) SD_BUS_PARAM(y) SD_BUS_PARAM(w) SD_BUS_PARAM(h),
            "", SD_BUS_PARAM(),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::set_cursor_location_relative>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "ProcessHandWritingEvent",
            "ad", SD_BUS_PARAM(coordinates),
            "", SD_BUS_PARAM(),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::process_hand_writing_event>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "CancelHandWriting",
            "u", SD_BUS_PARAM(n_strokes),
            "", SD_BUS_PARAM(),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::cancel_hand_writing>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "FocusIn", "", "",
            (&ibus_inputcontext_message_adapter<&IBusInputContext::focus_in>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "FocusOut", "", "",
            (&ibus_inputcontext_message_adapter<&IBusInputContext::focus_out>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "Reset", "", "",
            (&ibus_inputcontext_message_adapter<&IBusInputContext::reset>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCapabilities",
            "u", SD_BUS_PARAM(caps),
            "", SD_BUS_PARAM(),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::set_capabilities>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "PropertyActivate",
            "su", SD_BUS_PARAM(name) SD_BUS_PARAM(state),
            "", SD_BUS_PARAM(),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::property_activate>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetEngine",
            "s", SD_BUS_PARAM(name),
            "", SD_BUS_PARAM(),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::set_engine>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "GetEngine",
            "", SD_BUS_PARAM(),
            "v", SD_BUS_PARAM(desc),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::get_engine>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetSurroundingText",
            "vuu", SD_BUS_PARAM(text) SD_BUS_PARAM(cursor_pos) SD_BUS_PARAM(anchor_pos),
            "v", SD_BUS_PARAM(desc),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::set_surrounding_text>),
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
            (&ibus_inputcontext_prop_adapter<&IBusInputContext::get_client_commit_preedit>),
            (&ibus_inputcontext_prop_adapter<&IBusInputContext::set_client_commit_preedit>),
            0,
            SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_WRITABLE_PROPERTY(
            "ContentType",
            "(uu)",
            (&ibus_inputcontext_prop_adapter<&IBusInputContext::get_content_type>),
            (&ibus_inputcontext_prop_adapter<&IBusInputContext::set_content_type>),
            0,
            SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_VTABLE_END,
};

const sd_bus_vtable IBusInputContext::m_service_vtbl[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD(
            "Destroy",
            "",
            "",
            (&ibus_inputcontext_message_adapter<&IBusInputContext::destroy>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

/**
 * X11ICManager::create_ic
 */
IBusInputContext::IBusInputContext(IBusInputContextObserver *observer, int id, int siid)
    : m_observer(observer),
      m_id(id),
      m_siid(siid),
      m_shared_siid(false),
      m_on(false),
      m_onspot_preedit_started(false),
      m_onspot_preedit_length(0),
      m_onspot_caret(0),
      m_client_commit_preedit(false),
      m_purpose(0),
      m_hints(0),
      m_capabilities(0),
      m_cursor_location({0, 0, 0, 0}),
      m_bus(NULL),
      m_inputcontext_slot(NULL),
      m_service_slot(NULL)
{
    log_func();
}

IBusInputContext::~IBusInputContext()
{
    log_func();

    deinit();
}

int IBusInputContext::init(sd_bus *bus)
{
    log_func();

    ostringstream ss;
    ss << IBUS_INPUTCONTEXT_OBJECT_PATH << m_id;
    String path = ss.str();

    log_debug("object path: %s", path.c_str());

    if (sd_bus_add_object_vtable(bus,
                                 &m_service_slot,
                                 path.c_str(),
                                 IBUS_SERVICE_INTERFACE,
                                 m_service_vtbl,
                                 this) < 0) {
        return -1;
    }

    if (sd_bus_add_object_vtable(bus,
                                 &m_inputcontext_slot,
                                 path.c_str(),
                                 IBUS_INPUTCONTEXT_INTERFACE,
                                 m_inputcontext_vtbl,
                                 this) < 0) {
        return -1;
    }

    m_object_path = path;
    m_bus = bus;

    return 0;
}

/**
 * @v <@(sa{sv}sv) ("IBusText", {}, "\37239\38899\36664\20837\27861", <@(sa{sv}av) ("IBusAttrList", {}, [])>)>
 */
int IBusInputContext::notify_commit_text(const char *text)
{
    log_func_incomplete();

    // TODO finish text to signal
    sd_bus_message *sig = NULL;
    if (sd_bus_message_new_signal(m_bus,
                                  &sig,
                                  m_object_path.c_str(),
                                  IBUS_INPUTCONTEXT_INTERFACE,
                                  "CommitText") < -1) {
        return -1;
    }

    return -1;
}

int IBusInputContext::notify_forward_key_event(uint32_t keyval, uint32_t keycode, uint32_t state)
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "ForwardKeyEvent",
                              "uuu",
                              keyval,
                              keycode,
                              state);
}

int IBusInputContext::notify_update_preedit_text(const char *text, uint32_t cursor_pos, bool visible, uint32_t mode)
{
    log_func_not_impl();

    return -1;
}

int IBusInputContext::notify_show_preedit_text()
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "ShowPreeditText",
                              "");
}

int IBusInputContext::notify_hide_preedit_text()
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "HidePreeditText",
                              "");
}

int IBusInputContext::notify_update_aux_text(const char *text, bool visible)
{
    log_func_not_impl();

    return -1;
}

int IBusInputContext::notify_show_aux_text()
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "ShowAuxiliaryText",
                              "");
}

int IBusInputContext::notify_hide_aux_text()
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "HideAuxiliaryText",
                              "");
}

int IBusInputContext::notify_update_lookup_table(/* v* */ void *table, bool visible)
{
    log_func_not_impl();

    return -1;
}

int IBusInputContext::notify_show_lookup_table()
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "ShowLookupTable",
                              "");
}

int IBusInputContext::notify_hide_lookup_table()
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "HideLookupTable",
                              "");
}

int IBusInputContext::notify_page_up_lookup_table()
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "PageUpLookupTable",
                              "");
}

int IBusInputContext::notify_page_down_lookup_table()
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "PageDownLookupTable",
                              "");
}

int IBusInputContext::notify_cursor_up_lookup_table()
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "CursorUpLookupTable",
                              "");
}

int IBusInputContext::notify_cursor_down_lookup_table()
{
    log_func();

    return sd_bus_emit_signal(m_bus,
                              m_object_path.c_str(),
                              IBUS_INPUTCONTEXT_INTERFACE,
                              "CursorDownLookupTable",
                              "");
}

int IBusInputContext::notify_register_properties(/* v */ void *props)
{
    log_func_not_impl();

    return -1;
}

int IBusInputContext::notify_update_property(/* v */ void *prop)
{
    log_func_not_impl();

    return -1;
}

void IBusInputContext::deinit()
{
    log_func();

    if(m_inputcontext_slot) {
        sd_bus_slot_unref(m_inputcontext_slot);
        m_inputcontext_slot = NULL;
    }

    if(m_service_slot) {
        sd_bus_slot_unref(m_service_slot);
        m_service_slot = NULL;
    }
}

int IBusInputContext::destroy(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func();

    deinit();

    m_observer->input_context_destroy(this);

    return 0;
}

int IBusInputContext::process_key_event(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func();

    uint32_t keyval = 0;
    uint32_t keycode = 0;
    uint32_t state = 0;

    if (sd_bus_message_read(m, "uuu", &keyval, &keycode, &state) < 0) {
        return -1;
    }

    bool processed = m_observer->input_context_process_key_event(this,
                                                               keyval,
                                                               keycode,
                                                               state);
    return sd_bus_reply_method_return(m, "b", processed);
}

int IBusInputContext::set_cursor_location(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func();

    if (sd_bus_message_read(m,
                            "iiii",
                            &m_cursor_location.x,
                            &m_cursor_location.y,
                            &m_cursor_location.h,
                            &m_cursor_location.y) < 0) {
        return -1;
    }

    m_observer->input_context_cursor_location_updated(this);

    return 0;
}

int IBusInputContext::set_cursor_location_relative(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func();

    IBusRect rect; 
    if (sd_bus_message_read(m, "iiii", &rect.x, &rect.y, &rect.h, &rect.y) < 0) {
        return -1;
    }

    m_cursor_location.x += rect.x;
    m_cursor_location.y += rect.y;
    m_cursor_location.w += rect.w;
    m_cursor_location.h += rect.h;

    return 0;
}


int IBusInputContext::process_hand_writing_event(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl();

    return -1;
}

int IBusInputContext::cancel_hand_writing(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl();

    return -1;
}

int IBusInputContext::property_activate(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl();

    return -1;
}

int IBusInputContext::set_engine(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl();

    return -1;
}

int IBusInputContext::get_engine(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl();

    return -1;
}

int IBusInputContext::set_surrounding_text(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func_not_impl();

    return -1;
}

int IBusInputContext::focus_in(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func();

    m_observer->input_context_focus_in(this);

    return 0;
}

int IBusInputContext::focus_out(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func();

    m_observer->input_context_focus_out(this);

    return 0;
}

int IBusInputContext::reset(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func();

    m_observer->input_context_reset(this);

    return 0;
}

int IBusInputContext::set_capabilities(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func();

    if (sd_bus_message_read(m, "u", &m_capabilities) < 0) {
        return -1;
    }

    log_debug("capabilities = %s", ibus_caps_to_str(m_capabilities).c_str());

    m_observer->input_context_capability_updated(this);

    return 0;
}

int IBusInputContext::get_client_commit_preedit(sd_bus *bus,
                                                const char *path,
                                                const char *interface,
                                                const char *property,
                                                sd_bus_message *value,
                                                sd_bus_error *ret_error)
{
    log_func();

    if (sd_bus_message_open_container(value, 'r', "b") < 0) {
        return -1;
    }

    bool v = m_client_commit_preedit;
    if (sd_bus_message_append(value, "b", v) < 0) {
        return -1;
    }

    return sd_bus_message_close_container(value);
}

int IBusInputContext::set_client_commit_preedit(sd_bus *bus,
                                                const char *path,
                                                const char *interface,
                                                const char *property,
                                                sd_bus_message *value,
                                                sd_bus_error *ret_error)
{
    log_func();

    if (sd_bus_message_enter_container(value, 'r', "uu") < 0) {
        return -1;
    }

    return sd_bus_message_read(value, "uu", &m_purpose, &m_hints);
}

int IBusInputContext::get_content_type(sd_bus *bus,
                                       const char *path,
                                       const char *interface,
                                       const char *property,
                                       sd_bus_message *value,
                                       sd_bus_error *ret_error)
{
    log_func();

    if (sd_bus_message_open_container(value, 'r', "uu") < 0) {
        return -1;
    }

    if (sd_bus_message_append(value, "uu", m_purpose, m_hints) < 0) {
        return -1;
    }

    return sd_bus_message_close_container(value);
}

int IBusInputContext::set_content_type(sd_bus *bus,
                                       const char *path,
                                       const char *interface,
                                       const char *property,
                                       sd_bus_message *value,
                                       sd_bus_error *ret_error)
{
    log_func();

    if (sd_bus_message_enter_container(value, 'r', "uu") < 0) {
        return -1;
    }

    if (sd_bus_message_read(value, "uu", &m_purpose, &m_hints) < 0) {
        return -1;
    }

    return 0;
}
